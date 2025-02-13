FILESEXTRAPATHS:prepend := "${THISDIR}/linux-aspeed:"

SRC_URI += " \
    file://0001-meta-facebook-ventura-Add-Linux-device-tree-related-.patch \
    file://0002-ARM-dts-aspeed-ventura-modify-dts-for-PVT-stage.patch \
"
