FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = " \
    file://0100-Revert-Explicitly-set-verify_none.patch \
    file://0101-mutual-tls-meta-Support-svc-and-host-entity-types.patch \
    file://0102-Set-request-timeout-to-40s-on-bmcweb-http-http_conne.patch \
    file://0103-Set-connection-count-to-1000.patch \
"
EXTRA_OEMESON:append = " -Dredfish-updateservice-use-dbus=enabled"
