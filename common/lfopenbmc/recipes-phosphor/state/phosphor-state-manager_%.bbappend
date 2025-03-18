FILESEXTRAPATHS:prepend := "${THISDIR}/files:"


EXTRA_OEMESON:append = " \
                         -Donly-allow-boot-when-bmc-ready=false \
                       "

SRC_URI += " \
    file://0001-Also-allow-power-policy-when-watchdog-flag-is-raised.patch \
    file://0002-Add-bmc-reboot-event-log.patch \
"
