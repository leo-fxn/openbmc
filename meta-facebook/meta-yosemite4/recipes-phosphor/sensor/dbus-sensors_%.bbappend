FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = " \
    file://0001-hwmontempsensor-add-option-for-PowerStateChanged.patch \
    file://0002-meta-facebook-yosemite4-Disable-in2_alarm-event.patch \
"

EXTRA_OEMESON:append = " -Ddisable-host-power-monitoring=true"
EXTRA_OEMESON:append = " -Ddisable-in2-alarm-event=true"
