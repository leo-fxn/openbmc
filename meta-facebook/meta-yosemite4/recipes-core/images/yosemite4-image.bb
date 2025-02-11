require common/images/fb-openbmc-image.inc

# Install temporary firmware update utilities for POC phase.
IMAGE_INSTALL:append = " bios-fw-update"
IMAGE_INSTALL:append = " cpld-fw-handler"
IMAGE_INSTALL:append = " pldm-fw-update"
IMAGE_INSTALL:append = " pldm-package-re-wrapper"
IMAGE_INSTALL:append = " cxl-fw-update"
IMAGE_INSTALL:append = " addc"
IMAGE_INSTALL:append = " yaap"
IMAGE_INSTALL:append = " compute-software-id"
IMAGE_INSTALL:append = " fw-versions"
IMAGE_INSTALL:append = " pmic-error-injection"
# Added as temp memory bloat remediation T213223725
IMAGE_INSTALL:append = " sshd-remediate"
# T214301589: Added as a temp fix for power sensor collection on T2 machines
IMAGE_INSTALL:append = " sensor-cache"
# T214925969: Temp measure to allow fans to be set to a particular PCT on boot
IMAGE_INSTALL:append = " fan-manual-mode"
