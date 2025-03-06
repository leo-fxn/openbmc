
inherit pkgconfig meson systemd

SUMMARY = "redfish-client"
HOMEPAGE = "https://github.com/facebook/openbmc"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=86d3f3a95c324c9479bd8986968f4327"
DEPENDS = " \
	${PYTHON_PN}-mako-native \
	boost \
	curl \
	nlohmann-json \
	phosphor-dbus-interfaces \
	phosphor-logging \
	sdbusplus \
"

S = "${WORKDIR}/sources"
UNPACKDIR = "${S}"

SYSTEMD_SERVICE:${PN} += "xyz.openbmc_project.RedfishClient.service"
SRC_URI = " \
	file://http_client.cpp \
	file://http_client.hpp \
	file://meson.build \
	file://meson.options \
	file://sensor.cpp \
	file://sensor.hpp \
	file://daemon.cpp \
	file://daemon.hpp \
	file://daemon_main.cpp \
	file://source.cpp \
	file://source.hpp \
	file://LICENSE \
	file://subprojects \
	file://test \
	file://redfish-binding \
	https://www.dmtf.org/sites/default/files/standards/documents/DSP8010_2024.3.zip;sha256sum=f2e2973965edbfac4e1fe351463939ce8e4ef878879d21dd5c19fc59ea7e139a \
	file://configurations \
	file://xyz.openbmc_project.RedfishClient.service \
"

do_install:append() {
	install -d ${D}${systemd_system_unitdir}
	install -m 0644 ${S}/xyz.openbmc_project.RedfishClient.service ${D}${systemd_system_unitdir}
}
