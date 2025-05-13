#

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "file://cover.conf \
            file://f0tg_hpdb_hsc.conf \
            file://f0tb_hpdb_hsc.conf \
            file://f0tc_hpdb_hsc.conf \
           "

do_install:append() {
    install -m 644 ${UNPACKDIR}/f0tg_hpdb_hsc.conf ${D}${sysconfdir}/sensors_cfg/f0tg_hpdb_hsc.conf
    install -m 644 ${UNPACKDIR}/f0tb_hpdb_hsc.conf ${D}${sysconfdir}/sensors_cfg/f0tb_hpdb_hsc.conf
    install -m 644 ${UNPACKDIR}/f0tc_hpdb_hsc.conf ${D}${sysconfdir}/sensors_cfg/f0tc_hpdb_hsc.conf
}
