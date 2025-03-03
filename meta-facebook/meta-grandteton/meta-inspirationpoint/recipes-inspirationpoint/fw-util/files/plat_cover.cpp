#include <cstdio>
#include <cstring>
#include <fstream>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <openbmc/pal.h>
#include <openbmc/kv.hpp>
#include <openbmc/obmc-i2c.h>
#include <syslog.h>
#include "vr_fw.h"
#include "bios.h"

class palBiosComponent : public BiosComponent {
  public:
    palBiosComponent(const std::string &fru, const std::string &comp, const std::string &mtd,
                     const std::string &devpath, const std::string &dev, const std::string &shadow,
                     bool level, const std::string &verp) :
      BiosComponent(fru, comp, mtd, devpath, dev, shadow, level, verp) {}
    int update_finish(void) override;
    int reboot(uint8_t fruid) override;
    int setMeRecovery(uint8_t retry) override;
};

int palBiosComponent::update_finish(void) {
  sys().output << "To complete the upgrade, please perform 'power-util sled-cycle'" << std::endl;
  return 0;
}

int palBiosComponent::reboot(uint8_t /*fruid*/) {
  return 0;
}

//No ME in AMD platfrom
int palBiosComponent::setMeRecovery(uint8_t /*retry*/) {
  return 0;
}


class bios_cover_config {
  private:
    std::optional<palBiosComponent> bios;

    bios_cover_config() {
      try {
        std::string product = kv::get("cpu_product", kv::region::temp);
        if (product == "Turin") {
          bios.emplace("mb", "bios", "pnor", "/sys/bus/platform/drivers/aspeed-smc",
                       "1e631000.spi", "FM_BMC_MUX_CS_SPI_SEL_0", true, "(F0TC)(.*)");
        } else if (product == "Genoa") {
          bios.emplace("mb", "bios", "pnor", "/sys/bus/platform/drivers/aspeed-smc",
                       "1e631000.spi", "FM_BMC_MUX_CS_SPI_SEL_0", true, "(F0TG)(.*)");
        } else {
          syslog(LOG_CRIT, "Unknown CPU product: %s", product.c_str());
        }
      } catch (const std::exception& err) {
          syslog(LOG_CRIT, "Exception in bios_cover_config, err: %s", err.what());
      }
    }
  public:
    static bios_cover_config& getInstance() {
      static bios_cover_config instance;
      return instance;
    }
};
bios_cover_config& config = bios_cover_config::getInstance();

VrComponent vr_cpu0_vcore0("mb", "cpu0_vcore0", "VR_CPU0_VCORE0/SOC");
VrComponent vr_cpu0_vcore1("mb", "cpu0_vcore1", "VR_CPU0_VCORE1/PVDDIO");
VrComponent vr_cpu0_pvdd11("mb", "cpu0_pvdd11", "VR_CPU0_PVDD11");

VrComponent vr_cpu1_vcore0("mb", "cpu1_vcore0", "VR_CPU1_VCORE0/SOC");
VrComponent vr_cpu1_vcore1("mb", "cpu1_vcore1", "VR_CPU1_VCORE1/PVDDIO");
VrComponent vr_cpu1_pvdd11("mb", "cpu1_pvdd11", "VR_CPU1_PVDD11");
