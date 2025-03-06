FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += " \
  file://mem-dump.service \
  file://mem-dump \
"

MEM_WARN_TGT = "mem-dump.service"

SYSTEMD_SERVICE:${PN} += "\
  mem-dump.service \
"

do_install:append:() {
  install -d ${D}${datadir}/phosphor-health-monitor
  install -m 0644 ${UNPACKDIR}/mem-dump.service ${D}${systemd_system_unitdir}/mem-dump.service
  install -d ${D}${libexecdir}/${PN}
  install -m 0755 ${UNPACKDIR}/mem-dump ${D}${libexecdir}/${PN}/
}
