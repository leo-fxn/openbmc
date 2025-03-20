#!/bin/bash
# Function to display VSZ for a specific PID
display_vsz() {
    local pid=$1
    # Check if /proc/<pid>/stat exists
    if [ -f "/proc/$pid/stat" ]; then
        # Extract the VSZ value (23rd field in /proc/<pid>/stat)
        vsz_bytes=$(awk '{print $23}' "/proc/$pid/stat")
        
        # Convert VSZ from bytes to kilobytes (1 KB = 1024 bytes)
        vsz_kb=$((vsz_bytes / 1024))
        if [ $vsz_kb -lt 1024 ]; then
          return
        fi
        # Get the command name from the comm file
        comm_file="/proc/$pid/comm"
        if [ -r "$comm_file" ]; then
          comm=$(cat "$comm_file")
        else
          comm="Unknown"
        fi
        
        # Print PID and VSZ in KB
        echo "${vsz_kb} KB PID: $pid ($comm)"
    fi
}
inject_logs() {
	while(true) ; do
		busctl call xyz.openbmc_project.Logging \
		  /xyz/openbmc_project/logging \
		  xyz.openbmc_project.Logging.Create \
		  Create \
		  ssa{ss} \
		  "xyz.openbmc_project.Sensor.Threshold.Error.Fan_tachWarningHigh" \
		  "xyz.openbmc_project.Logging.Entry.Level.Warning" \
		  5 \
		  "CALLOUT_INVENTORY_PATH" "/xyz/openbmc_project/inventory/system/board/Yosemite_4_Fan_Board_1" \
		  "SENSOR_NAME" "/xyz/openbmc_project/sensors/fan_tach/FANBOARD1_FAN2_TACH_OUTLET_SPEED_RPM" \
		  "SENSOR_VALUE" "13366.000000" \
		  "THRESHOLD_VALUE" "13300.000000" \
		  "_PID" "894"
	done
}
FAN_PCT=100
echo "FAN_PCT is $FAN_PCT"
/usr/bin/mfg-tool fan-mode -m
/usr/bin/mfg-tool fan-speed | /usr/bin/jq 'keys' | /usr/bin/grep FANBOARD | /usr/bin/tr -d \", | /usr/bin/awk '{print $1}' | xargs -n1 /usr/bin/mfg-tool fan-speed -t "$FAN_PCT" -p
for x in `seq 1 3` ; do
	inject_logs &> /dev/zero &
done
while(true) ; do
    hostname
    date
    # No PID provided, iterate through all PIDs in /proc
    for pid in $(ls /proc | grep -E '^[0-9]+$'); do
	display_vsz "$pid"
    done | sort -n | tail -n 5
done
wait
