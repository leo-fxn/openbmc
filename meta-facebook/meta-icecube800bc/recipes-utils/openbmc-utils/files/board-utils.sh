#!/bin/bash
#
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This program file is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; version 2 of the License.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program in a file named COPYING; if not, write to the
# Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor,
# Boston, MA 02110-1301 USA

#shellcheck disable=SC1091
# Do not change this line to openbmc-utils.sh, or it will generate a source loop.
. /usr/local/bin/i2c-utils.sh

MCBCPLD_SYSFS_DIR=$(i2c_device_sysfs_abspath 12-0060)
SCMCPLD_SYSFS_DIR=$(i2c_device_sysfs_abspath 1-0035)

BOARD_ID="${SCMCPLD_SYSFS_DIR}/board_id"
VERSION_ID="${SCMCPLD_SYSFS_DIR}/version_id"
COME_POWER_EN="${SCMCPLD_SYSFS_DIR}/pwr_come_en"
COME_POWER_OFF="${SCMCPLD_SYSFS_DIR}/pwr_force_off"
COME_SYSTEM_WARM_RESET="${SCMCPLD_SYSFS_DIR}/cb_sys_reset"

CHASSIS_POWER_CYCLE="${MCBCPLD_SYSFS_DIR}/power_cycle_go"

wedge_board_type() {
    echo 'icecube800bc'
}

wedge_board_rev() {
    board_id=$(head -n 1 < "$BOARD_ID" 2> /dev/null)
    version_id=$(head -n 1 < "$VERSION_ID" 2> /dev/null)

    case "$((board_id))" in
        8)
            echo "Board type: Icecube800bc Networking System Board "
            ;;
        *)
            echo "Board type: unknown value [$board_id]"
            ;;
    esac

    case "$((version_id))" in
        0)
            echo "Revision: Pre-EVT & EVT1"
            ;;
        1)
            echo "Revision: EVT-2A"
            ;;
        2)
            echo "Revision: EVT-2B"
            ;;
        3)
            echo "Revision: DVT-1A"
            ;;
        4)
            echo "Revision: DVT-1B"
            ;;
        5)
            echo "Revision: PPVT"
            ;;
        6)
            echo "Revision: PVT"
            ;;
        7)
            echo "Revision: MP"
            ;;
        *)
            echo "Revision: unknown value [$version_id]"
            ;;
    esac

    return 0
}

userver_power_is_on() {
    userver_pwr_status_n=$(head -n 1 "$COME_POWER_OFF" 2> /dev/null)

    if [ $((userver_pwr_status_n)) -eq $((0x1)) ] ; then
        return 0
    fi

    return 1
}

userver_power_on() {
    if ! sysfs_write "$COME_POWER_OFF" 1; then
        return 1
    fi
    if ! sysfs_write "$COME_POWER_EN" 1; then
        return 1
    fi
}

userver_power_off() {
    if ! sysfs_write "$COME_POWER_OFF" 0; then
        return 1
    fi
}

userver_reset() {
    if ! sysfs_write "$COME_SYSTEM_WARM_RESET" 0; then
        return 1
    fi
    sleep 1
    if ! sysfs_write "$COME_SYSTEM_WARM_RESET" 1; then
        return 1
    fi

    return 0
}

chassis_power_cycle() {
    if ! sysfs_write "$CHASSIS_POWER_CYCLE" 1; then
        return 1
    fi

    return 0
}

bmc_mac_addr() {
    # Fetch mac addr supporting v5+ format.
    bmc_mac=$(weutil | sed -nE 's/BMC MAC Base: (.*)/\1/p')
    if [ -z "$bmc_mac" ]; then
        echo "BMC MAC Address Not Found !" 1>&2
        logger -p user.crit "BMC MAC Address Not Found !"
        return 1
    else
        echo "$bmc_mac"
    fi
}

userver_mac_addr() {
    # Fetch mac addr supporting v5+ format.
    cpu_mac=$(weutil | sed -nE 's/X86 CPU MAC Base: (.*)/\1/p')
    if [ -z "$cpu_mac" ]; then
        echo "x86 CPU MAC Address Not Found !" 1>&2
        logger -p user.crit "x86 CPU MAC Address Not Found !"
        return 1
    else
        echo "$cpu_mac"
    fi
}
