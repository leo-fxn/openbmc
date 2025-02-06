#!/bin/sh
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

. /usr/local/bin/openbmc-utils.sh
. /usr/local/fbpackages/utils/ast-functions

HMC_FRU_BIN="/tmp/fruid_hmc.bin"
HMC_EEPROM_ADDR="0x4e"
CX7_FRU_BIN="/tmp/fruid_cx7.bin"
CX7_EEPROM_ADDR="0x4d"
 
probe_eeprom_driver () {
  addr_wo_prefix=${1#0x}
  MAX_RETRY=$2
  driver="24c64"

  if [ "$1" == "$HMC_EEPROM_ADDR" ]; then
   driver="24c02"
  fi

  for (( i=1; i<=$MAX_RETRY; i++ )); do
    if [ ! -L "/sys/bus/i2c/drivers/at24/9-00$addr_wo_prefix" ]; then
      i2c_device_delete 9 $1 2>/dev/null
      i2c_device_add 9 $1 "$driver" 2>/dev/null
    else
      return
    fi
    sleep 0.2
  done
}

copy_gpu_eeprom () {
  addr=$1
  bin=$2
  MAX_RETRY=$3

  for (( i=1; i<=$MAX_RETRY; i++ )); do
    if [ ! -e "$bin" ] || [ $(wc -c <"$bin") -eq 0 ]; then
      /bin/dd if=/sys/class/i2c-dev/i2c-9/device/9-00${addr}/eeprom of=$bin bs=512 count=1
    fi
    sleep 0.2
  done
}

setup_hmc_eeprom () {
  addr=("$HMC_EEPROM_ADDR" "$CX7_EEPROM_ADDR")
  bins=("$HMC_FRU_BIN" "$CX7_FRU_BIN")

  MAX_RETRY=10

  for loop in "${!addr[@]}"; do
    probe_eeprom_driver "${addr[$loop]}" $MAX_RETRY
    copy_gpu_eeprom "${addr[$loop]#0x}" "${bins[$loop]}" $MAX_RETRY
  done
}

setup_hmc_eeprom
