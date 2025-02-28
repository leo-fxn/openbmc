FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += " \
    file://objmgr_memprotection.conf \
    "

do_install:append() {
    install -d ${D}/etc/systemd/system/xyz.openbmc_project.ObjectMapper.service.d/
    install -m 0644 ${UNPACKDIR}/objmgr_memprotection.conf ${D}/etc/systemd/system/xyz.openbmc_project.ObjectMapper.service.d/
}
