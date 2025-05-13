FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

EXTRA_OEMESON:append = " -Dmaximum-transfer-size=150 "
EXTRA_OEMESON:append = " -Dsensor-polling-time=2000 "
EXTRA_OEMESON:append = " -Ddiscovery-fru-data-from-terminus=disabled "
