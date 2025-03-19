FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI += " \
    file://phosphor-reset-host-reboot-attempts_override.conf \
"

do_install:append() {
    override_dir=${D}${systemd_system_unitdir}/phosphor-reset-host-reboot-attempts@.service.d
    install -d ${override_dir}
    install -m 0644 ${UNPACKDIR}/phosphor-reset-host-reboot-attempts_override.conf \
            ${override_dir}/phosphor-reset-host-reboot-attempts_override.conf
}

FILES:${PN} += "${systemd_system_unitdir}/phosphor-reset-host-reboot-attempts@.service.d/phosphor-reset-host-reboot-attempts_override.conf"
