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

FILESEXTRAPATHS:prepend := "${THISDIR}/patches_6.12:"

SRC_URI:append = "file://1001-ARM-dts-aspeed-wedge400-Set-i2c-0-slave-timeout.patch \
                  file://1002-ARM-dts-aspeed-wedge400-Enable-aspeed-spi-controller.patch \
                  file://1003-ARM-dts-aspeed-wedge400-Enable-jtag-controller.patch \
                  file://1004-hwmon-pmbus-pxe1610-support-pxe1211-chip.patch \
                  file://1005-hwmon-pmbus-add-ir35215-driver.patch \
                  file://1006-hwmon-powr1220-Update-max-channel-to-14.patch \
                  file://1007-hwmon-Add-net_asci-driver.patch \
"
