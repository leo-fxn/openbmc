FILESEXTRAPATHS:prepend := "${THISDIR}/obmc-console:"

inherit obmc-phosphor-systemd

SRC_URI += " \
    file://install-multi-user.conf \
"
do_install:append() {
    install -d ${D}${systemd_system_unitdir}/obmc-console@.service.d/
    install -m 0644 ${UNPACKDIR}/install-multi-user.conf ${D}${systemd_system_unitdir}/obmc-console@.service.d/install-multi-user.conf
}
