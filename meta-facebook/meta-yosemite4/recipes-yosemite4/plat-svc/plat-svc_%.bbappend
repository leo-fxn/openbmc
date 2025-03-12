FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += " \
    file://0001-meta-facebook-yosemite4-optimize-i2cv2-frequency.patch \
    file://0002-meta-facebook-yosemite4-set-i3c-hub-register.patch \
    "

UNPACKDIR = "${S}"

do_patch() {
    cd ${UNPACKDIR} && patch -p6 < "${UNPACKDIR}/0001-meta-facebook-yosemite4-optimize-i2cv2-frequency.patch"
    cd ${UNPACKDIR} && patch -p6 < "${UNPACKDIR}/0002-meta-facebook-yosemite4-set-i3c-hub-register.patch"
}
