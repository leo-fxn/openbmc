FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

EXTRA_OEMESON:append = " \
    -Ddisable-host-power-monitoring=true \
    -Ddisable-in2-alarm-event=true \
    -Dfan_sensor_retry_attempts=5 \
"
