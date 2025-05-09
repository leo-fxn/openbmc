FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += " \
    file://limitcore.conf \
    file://1000-avoid-new-dbus.patch \
"

do_install:append() {
    install -d ${D}${systemd_system_unitdir}/phosphor-pid-control.service.d/
    install -m 0644 ${UNPACKDIR}/limitcore.conf ${D}${systemd_system_unitdir}/phosphor-pid-control.service.d/
}
