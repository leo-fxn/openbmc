FILESEXTRAPATHS:prepend := "${THISDIR}/obmc-console:"

inherit obmc-phosphor-systemd

SRC_URI += " \
    file://enable-console.rules \
"
do_install:append() {
    install -d ${D}/etc/udev/rules.d/
    install -m 0644 ${UNPACKDIR}/enable-console.rules ${D}/etc/udev/rules.d/enable-console.rules
}
