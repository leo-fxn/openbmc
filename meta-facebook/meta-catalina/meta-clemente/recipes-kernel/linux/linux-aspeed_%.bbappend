FILESEXTRAPATHS:prepend := "${THISDIR}/linux-aspeed:"

SRC_URI += " \
    file://0000-ARM-dts-aspeed-clemente-add-initial-BMC.patch \
    file://0000-dt-bindings-arm-aspeed-add-Meta-Clemente-board.patch \
    file://0001-ARM-dts-aspeed-clemente-add-various-device-nodes-and.patch \
    file://0002-ARM-dts-aspeed-clemente-update-DTS-for-DVT-board.patch \
    file://clemente-sensor.cfg \
"

