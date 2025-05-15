#!/bin/bash -e
REQUEST_FILE=/run/openbmc/pldm_monitor_requested
PLDM_CANARY_SENSOR_LIST=/var/pldm_canary_sensor_list

function remediation_log {
	echo "restarting pldmd"
}

function remediation_action {
	systemctl restart pldmd
}

function test_if_chassis_powered_on {
	busctl get-property xyz.openbmc_project.State.Chassis$1 /xyz/openbmc_project/state/chassis$1 xyz.openbmc_project.State.Chassis CurrentPowerState | grep -q PowerState.On
	return $?
}

function remediate_and_exit {
	# Remove the request file, so that systemd can re-create it with a new timestamp
	# Only remediate if all of the hosts are up, as we can't trust the numbers otherwise
	CHASSIS_UP=0
	for i in $(seq 8); do
		test_if_chassis_powered_on "$i"
		if [ $? -eq 0 ]; then
			CHASSIS_UP=$((CHASSIS_UP + 1))
		fi
	done

	# if all chassis are powered, then we can assume that the BICs are also powered
	# and as a result assume that something is happening with pldmd
	if [ $CHASSIS_UP -eq 8 ]; then
		rm -f "$REQUEST_FILE"
		remediation_log
		remediation_action
	fi
	exit 0
}

function check_for_value {
	object=$1
	# nan or 0 are indicative of failure
	busctl get-property xyz.openbmc_project.VirtualSensor "$object" xyz.openbmc_project.Sensor.Value Value | awk '{print $2}' | egrep -q -i -v -e "nan" -e '^0$'
	return $?
}

if [ ! -f $REQUEST_FILE ]; then
	exit 0
fi

# Give pldm at least 5 minutes to start up. So we don't do
# anything if the age of the request file is less than 5m
AGE=$(($(date +%s) - $(stat -c %Y "$REQUEST_FILE")))
if [ "$AGE" -lt 300 ]; then
	exit 0
fi

# Populate with the virtual sensors that we use as indicators that pldmd is misbehaving
# /xyz/openbmc_project/sensors/temperature/PMB_TEMP_C
canary_paths=()

if [ -f "$PLDM_CANARY_SENSOR_LIST" ]; then
	readarray -t canary_paths < <(cat $PLDM_CANARY_SENSOR_LIST)
fi

for object in "${canary_paths[@]}"; do
	check_for_value "$object" || remediate_and_exit
done

rm -f $REQUEST_FILE
exit 0
