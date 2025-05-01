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

enable_ACA () {
  # Check ACA
  response=$(curl -k http://192.168.31.1/redfish/v1/Chassis/UBB | grep -i "ShowACAErrData")
  if echo "$response" | grep -i "Enabled"; then
    exit 0
  else
    # Enable ACA
    curl -k http://192.168.31.1/redfish/v1/Chassis/UBB -X PATCH -d '{"Oem" : { "ShowACAErrData": "Enabled"}}'
  fi
}

enable_err_inj() {
  for i in 0 1 2 3 4 5 6 7 ;
  do
    response=$(curl -ks http://192.168.31.1/redfish/v1/Chassis/OAM_${i}  | grep "EINJState")
    if echo "$response" | grep -i "enable"; then
      continue
    else
      curl -k -X POST http://192.168.31.1/redfish/v1/Chassis/OAM_${i}/Actions/Oem/AMD/Chassis.ErrInjection -d '{"ErrInjection" : "Enable"}';
    fi
  done
}

enable_err_inj

# Since ACA feature is crucial for debugging GPU issue
# Keep enabling it in the background until successful
while [ 1 ]; 
do
  enable_ACA
  sleep 60
done
