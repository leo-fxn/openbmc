FILESEXTRAPATHS:prepend := "${THISDIR}/linux-aspeed:"
SRC_URI += " \
    file://1000-i2c-aspeed-Acknowledge-Tx-ack-late-when-in-SLAVE_REA.patch \
    file://1001-ipmi-ssif_bmc-add-GPIO-based-alert-mechanism.patch \
    file://1002-bindings-ipmi-ssif-bmc-Add-property-to-adjust-respon.patch \
    file://1003-ipmi-ssif_bmc-Add-support-for-adjustable-response-ti.patch \
    file://1004-ARM-dts-aspeed-catalina-Add-IO-Mezz-board-thermal-se.patch \
    file://1005-ARM-dts-aspeed-catalina-Add-Front-IO-board-remote-th.patch \
    file://1006-ARM-dts-aspeed-catalina-Add-MP5990-power-sensor-node.patch \
    file://1007-ARM-dts-aspeed-catalina-Add-fan-controller-support.patch \
    file://1008-ARM-dts-aspeed-catalina-Add-second-source-fan-contro.patch \
    file://1009-ARM-dts-aspeed-catalina-Add-second-source-HSC-node-s.patch \
    file://1010-ARM-dts-aspeed-catalina-Remove-INA238-and-INA230-nod.patch \
    file://1011-ARM-dts-aspeed-catalina-Enable-multi-master-on-addit.patch \
    file://1012-ARM-dts-aspeed-catalina-Update-CBC-FRU-EEPROM-I2C-bu.patch \
    file://1013-ARM-dts-aspeed-catalina-Enable-MCTP-support-for-NIC-.patch \
    file://1014-ARM-dts-aspeed-catalina-add-extra-ncsi-properties.patch \
    file://1015-ARM-dts-aspeed-catalina-add-ipmb-dev-node.patch \
    file://1016-ARM-dts-aspeed-catalina-add-max31790-nodes.patch \
    file://1017-ARM-dts-aspeed-catalina-add-raa228004-nodes.patch \
    file://1018-ARM-dts-aspeed-catalina-enable-ssif-alert-pin.patch \
    file://1019-ARM-dts-aspeed-catalina-enable-uart-dma-mode.patch \
    file://1020-ARM-dts-aspeed-catalina-update-pdb-board-cpld-ioexp-.patch \
    file://1021-ARM-dts-aspeed-catalina-add-hdd-board-cpld-ioexp.patch \
    file://1022-ARM-dts-aspeed-catalina-Add-Delta-brick-nodes.patch \
    file://1023-ARM-dts-aspeed-catalina-Increase-SSIF-response-timeo.patch \
    file://catalina-uart.cfg \
"
