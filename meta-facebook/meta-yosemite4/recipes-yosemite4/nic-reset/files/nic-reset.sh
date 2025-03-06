#!/bin/bash
set -x

NIC=$1
echo "Resetting NIC $NIC"

# Get the set_gpio function necessary for resetting the nic
source /usr/libexec/yosemite4-common-functions

function restart_nic_services {
    pin_target_action="$1" # RISING or FALLING
    services=$(jq -c -r ".[] | select(.Name == \"PRSNT_NIC${NIC}_N\") | .Targets.${pin_target_action} | join(\" \")" /usr/share/phosphor-gpio-monitor/phosphor-multi-gpio-monitor.json)
    for s in $services;
    do
        timeout 15 systemctl restart "$s"
    done;
}

# Disable the nic
set_gpio "EN_NIC${NIC}_POWER_BMC_R"  0

# Run the same commands we do when a nic is removed
restart_nic_services RISING
sleep 1

# Re-enable the nic
set_gpio "EN_NIC${NIC}_POWER_BMC_R"  1
sleep 5

# Run the same commands we do when a nic is first inserted
restart_nic_services FALLING
