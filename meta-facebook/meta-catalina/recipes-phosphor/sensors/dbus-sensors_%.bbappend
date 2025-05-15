FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

PACKAGECONFIG[satellitesensor] = "-Dsatellite=enabled, -Dsatellite=disabled"

SYSTEMD_SERVICE:${PN} += "${@bb.utils.contains('PACKAGECONFIG', 'satellitesensor', \
                                               'xyz.openbmc_project.satellitesensor.service', \
                                               '', d)}"

PACKAGECONFIG:append = " \
    satellitesensor \
"

SRC_URI += " \
    file://0101-dbus-sensors-setup-SatelliteSensor-meson-build.patch \
    file://0102-dbus-sensors-Add-SatelliteSensor-support.patch \
    file://0103-dbus-sensors-Fix-SatelliteSensor-build-errors.patch \
    file://0104-dbus-sensors-fix-abnormal-sensor-readings-from-HMC.patch \
    file://0105-SatelliteSensor-improve-I2C-transactions-by-repeat-S.patch \
    "

SYSTEMD_OVERRIDE:${PN}:append = "\
    wait-host0-state-ready.conf:xyz.openbmc_project.satellitesensor.service.d/wait-host0-state-ready.conf \
"
