inherit pkgconfig meson
inherit ptest-meson
inherit python3native
inherit systemd

SUMMARY = "redfish-client"
HOMEPAGE = "https://github.com/facebook/openbmc"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=86d3f3a95c324c9479bd8986968f4327"
DEPENDS += " \
    ${PYTHON_PN}-mako-native \
    boost \
    curl \
    nlohmann-json \
    phosphor-dbus-interfaces \
    phosphor-logging \
    sdbusplus \
    ${@bb.utils.contains('PTEST_ENABLED', '1', 'gtest', '', d)} \
"

S = "${WORKDIR}/sources"
UNPACKDIR = "${S}"

SYSTEMD_SERVICE:${PN} += "xyz.openbmc_project.RedfishClient.service"
SRC_URI = " \
    file://LICENSE \
    file://meson.build \
    file://meson.options \
    file://configurations \
    file://redfish-binding \
    file://test \
    file://daemon.cpp \
    file://daemon.hpp \
    file://daemon_main.cpp \
    file://http_client.cpp \
    file://http_client.hpp \
    file://sensor.cpp \
    file://sensor.hpp \
    file://source.cpp \
    file://source.hpp \
    file://xyz.openbmc_project.RedfishClient.service \
    https://www.dmtf.org/sites/default/files/standards/documents/DSP8010_2024.4.zip;sha256sum=f364f36046897ffb5957f7b19378efd4365648df09b888da679b8d1a6c6af19d \
"

EXTRA_OEMESON = " \
    -Dtests=${@bb.utils.contains('PTEST_ENABLED', '1', 'enabled', 'disabled', d)} \
"

do_install:append() {
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${S}/xyz.openbmc_project.RedfishClient.service ${D}${systemd_system_unitdir}
}
