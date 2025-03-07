FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = " \
    file://0001-hwmontempsensor-add-option-for-PowerStateChanged.patch \
"

EXTRA_OEMESON:append = " -Ddisable-host-power-monitoring=true"
