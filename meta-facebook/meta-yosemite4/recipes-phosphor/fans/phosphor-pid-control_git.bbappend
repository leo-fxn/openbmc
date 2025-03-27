FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += " \
    file://limitcore.conf \
    file://1000-try-catch-removal.patch \
"

do_install:append() {
    install -d ${D}${systemd_system_unitdir}/phosphor-pid-control.service.d/
    install -m 0644 ${UNPACKDIR}/limitcore.conf ${D}${systemd_system_unitdir}/phosphor-pid-control.service.d/
}
