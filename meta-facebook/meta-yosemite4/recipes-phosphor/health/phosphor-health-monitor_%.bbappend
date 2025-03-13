FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += " \
  file://mem-warning.service \
  file://mem-warning \
"

MEM_WARN_TGT = "mem-warning.service"

SYSTEMD_SERVICE:${PN} += "\
  mem-warning.service \
"

do_install:append:() {
  install -d ${D}${datadir}/phosphor-health-monitor
  install -m 0644 ${UNPACKDIR}/mem-warning.service ${D}${systemd_system_unitdir}/mem-warning.service
  install -d ${D}${libexecdir}/${PN}
  install -m 0755 ${UNPACKDIR}/mem-warning ${D}${libexecdir}/${PN}/
}
