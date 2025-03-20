FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

EXTRA_OEMESON:append = " -Ddisable-host-power-monitoring=true"
EXTRA_OEMESON:append = " -Ddisable-in2-alarm-event=true"
