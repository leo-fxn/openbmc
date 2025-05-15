FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += " \
    file://monitorrequested.conf \
"

FILES:${PN} += "${systemd_system_unitdir}/*"

do_install:append () {
    override_dir=${D}${systemd_system_unitdir}/pldmd.service.d
    install -d ${D}${systemd_system_unitdir}/pldmd.service.d

    install -m 0644 ${UNPACKDIR}/monitorrequested.conf ${override_dir}/monitorrequested.conf
}
