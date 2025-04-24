#!/bin/bash
#
# Copyright (c) Meta Platforms, Inc. and affiliates. (http://www.meta.com)
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
#

# shellcheck source=/dev/null
hex_to_string() {
    local hex=$1

    # Remove the '0x' prefix if present
    hex=${hex#0x}

    # Pad the hex number to ensure even number of digits
    hex=$(printf "%08x" "0x$hex")

    local result=""
    for ((i=0; i<${#hex}; i+=2)); do
        result+="${hex:$i:2} "
    done
    echo "${result% }"
}

scm_register_read() {
    # Set the address
    i2cset -y 2 0x21 "0x$4"
    i2cset -y 2 0x22 "0x$3"
    i2cset -y 2 0x23 "0x$2"
    i2cset -y 2 0x24 "0x$1"
    i2cset -y 2 0x20 0x01

    # Get the value
    b0=$( i2cget -y 2 0x25 0x01)
    b1=$( i2cget -y 2 0x26 0x01)
    b2=$( i2cget -y 2 0x27 0x01)
    b3=$( i2cget -y 2 0x28 0x01)
    echo "$b3 $b2 $b1 $b0"
}

execute_command() {
    local reg_data=$1
    local reg_addr="${reg_data##* }"
    local reg_desc="${reg_data% *}"

    local reg_addr_array=()
    read -r -a reg_addr_array <<< "$(hex_to_string "$reg_addr")"
    local output
    output=$(scm_register_read "${reg_addr_array[@]}")

    echo "$reg_desc ; value = $output"
    sleep 1
}

register_data_dump() {
    for tuple in "${tuples[@]}"; do
            execute_command "$tuple"
    done
}

declare -a tuples=(
    "new X86 STAT_303C 0x303c"
    "new X86 STAT_306C 0x306c"
    "X86 STAT REG_304c 0x304c"
    "X86 STAT 3038 0x3038"
    "SCMFPGA VER REG_0020 0x0020"
    "BMC Power State_06090 0x6090"
    "3.3V STBY & 5V  PWR_STAT_5090 0x5090"
    "CPU Power State_7090 0x7090"
    "Standby Power Good_50b4 0x50b4"
    "BMC Power Good_0x60b4 0x60b4"
    "CPU Power Good_0x70b4 0x70b4"
)

echo -e "\n##### SCM FPGA i2c dump #####"
register_data_dump
