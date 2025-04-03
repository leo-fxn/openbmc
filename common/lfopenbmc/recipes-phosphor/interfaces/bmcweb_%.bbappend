FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI:append = " \
    file://0100-Revert-Explicitly-set-verify_none.patch \
    file://0101-mutual-tls-meta-Support-svc-and-host-entity-types.patch \
    file://0102-Set-request-timeout-to-40s-on-bmcweb-http-http_conne.patch \
    file://0103-Set-connection-count-to-1000.patch \
"

EXTRA_OEMESON:append = " -Dredfish-updateservice-use-dbus=enabled"

# Enable PDI-generated Redfish Message Registries
EXTRA_OEMESON:append = " -Dredfish-allow-dbus-messages-mapping=enabled"
# Patches for PDI-generated Redfish Message Registries
SRC_URI:append = " \
    file://0200-Fix-crash-on-requesting-all-events-if-dbus-logging-e.patch \
    file://0201-registries-make-registration-dynamic.patch \
    file://0202-registries-generate-registries_selector.hpp.patch \
    file://0203-Add-generation-for-Vendor-registries-and-a-DBus-to-R.patch \
    file://0204-registries-generate-from-phosphor-dbus-interfaces.patch \
    file://0205-Add-required-MemberId-to-the-EventRecord-definition.patch \
    file://0206-Map-DBus-event-to-RF-MessageID-and-parse-args.patch \
    file://0207-Show-mapped-and-raw-Dbus-messages-in-Log-services.patch \
"

# Dependency variables for PDI-generated Redfish Message Registries
DEPENDS += " \
    ${PYTHON_PN}-requests-native \
    phosphor-dbus-interfaces \
"
inherit python3native
