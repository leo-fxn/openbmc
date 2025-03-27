FILESEXTRAPATHS:prepend := "${THISDIR}/files:"
FILES:${PN} += " ${systemd_system_unitdir}/bmcweb.service.d"
SRC_URI += " \
    file://bmcweb_memprotection.conf \
    "

do_install:append() {
    install -d ${D}${systemd_system_unitdir}/bmcweb.service.d/
    install -m 0644 ${UNPACKDIR}/bmcweb_memprotection.conf ${D}${systemd_system_unitdir}/bmcweb.service.d/
}
