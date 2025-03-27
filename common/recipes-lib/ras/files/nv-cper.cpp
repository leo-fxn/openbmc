#include "nv-cper.hpp"
#include "cper.hpp"
#include <syslog.h>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

int parseFwError1(const uint8_t* data, const size_t dataSize, std::string& entry)
{
  std::ostringstream oss;
  nv_cper_fwerror1_t* event = (nv_cper_fwerror1_t*) data;
  int channel_disable1, channel_disable2, worst_channel, bad_pages;

  oss << "Error Type: CPU_DRAM-CHANNELS-DISABLE1, " \
      << "Error Severity: Fatal, MB1 Error Code: 0x"  \
      << std::uppercase << std::hex << std::setw(8) << std::setfill('0') \
      << event->mb1_error_code << ", ";
      
  oss << "Channel Disabled1: ";
  if (event->scratch_dram_channel_disable & 0x01) {
    channel_disable1 = ((event->scratch_dram_channel_disable & 0x3E) >> 1);
    oss << channel_disable1 << ", Disable Reason1: ";
    if (event->channel_retired_mask_bad_pages & (1 << channel_disable1)) {
      oss << "BAD_PAGE_THR_EXC, ";
    } else if (event->channel_retired_mask_training_fail & (1 << channel_disable1)) {
      oss << "TRAINING_FAIL, ";
    } else if (event->channel_fail_mask_frequency & (1 << channel_disable1)) {
      oss << "BOOT_FREQ_TRAINING_FAIL, ";
    } else {
      oss << "UNKNOW, ";
    }
  } else {
    oss << "NA, Disable Reason1: NA, ";
  }

  oss << "Channel Disabled2: ";
  if (event->scratch_dram_channel_disable & 0x80) {
    channel_disable2 = ((event->scratch_dram_channel_disable & 0x1F00) >> 8);
    oss << channel_disable2 << ", Disable Reason2: ";
    if (event->channel_retired_mask_bad_pages & (1 << channel_disable2)) {
      oss << "BAD_PAGE_THR_EXC, ";
    } else if (event->channel_retired_mask_training_fail & (1 << channel_disable2)) {
      oss << "TRAINING_FAIL, ";
    } else if (event->channel_fail_mask_frequency & (1 << channel_disable2)) {
      oss << "BOOT_FREQ_TRAINING_FAIL, ";
    } else {
      oss << "UNKNOW, ";
    }
  } else {
    oss << "NA, Disable Reason2: NA, ";
  }
  
  worst_channel = (event->channel_1st_bad_pages & 0x1F);
  bad_pages = ((event->channel_1st_bad_pages & 0xFFFFFFE0) >> 5);
  oss << "Worst Channel: " << worst_channel << ", Bad Pages: " << bad_pages;

  entry = oss.str();
  return cper::CPER_HANDLE_SUCCESS;
}

int parseFwError2(const uint8_t* data, const size_t dataSize, std::string& entry)
{
  std::ostringstream oss;

  oss << "Error Type: CPU_DRAM-CHANNELS-DISABLE2, " \
      << "Error Severity: Fatal"; 
  return cper::CPER_HANDLE_SUCCESS;
}

int parseNvSsifCperFile(const uint8_t* data, const size_t dataSize, std::string& entry)
{
  try
	{
    std::unordered_map<std::string, std::function<int(const uint8_t*, const size_t, std::string&)>> switchMap;
    std::string ip_signature(reinterpret_cast<const char*>(data), 16);
    size_t end_pos = ip_signature.find_last_not_of('\0');
    if (end_pos != std::string::npos) {
        ip_signature.resize(end_pos + 1);
    }

    switchMap["FWERROR1"] = parseFwError1;
    switchMap["FWERROR2"] = parseFwError2;

    auto it = switchMap.find(ip_signature);
    if (it != switchMap.end()) {
        it->second(data, dataSize, entry);
    } else {
      entry = "unknow signature";
    }
	}
	catch(const std::exception& e)
	{
		syslog(LOG_ERR, "Failed to parse CPER : %s", e.what());
    return cper::CPER_HANDLE_FAIL;
	}

  return cper::CPER_HANDLE_SUCCESS;
}

int createNvSsifCperDumpEntry(const uint8_t fru, 
                              const uint8_t* data, const size_t dataSize) {

  if (!fs::exists(cper::CPER_DUMP_PATH))
  {
    if (!fs::create_directories(cper::CPER_DUMP_PATH))
    {
      syslog(LOG_ERR, "Failed to create %s directory", cper::CPER_DUMP_PATH);
      return cper::CPER_HANDLE_FAIL;
    }
  }

  try
  {
    cper::limitDumpEntries();
  }
  catch(const std::exception& e)
  {
    syslog(LOG_ERR, "Failed to limit CPER dump entries. %s.", e.what());
  }

  // e.g. slotX_ssif_cper_202503110123_1741656213000
  std::ostringstream oss;
  uint64_t timestamp =
  std::chrono::duration_cast<std::chrono::milliseconds>(
  std::chrono::system_clock::now().time_since_epoch()).count();

  auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::tm* time_info = std::localtime(&now);

  oss << cper::CPER_DUMP_PATH
      << "slot" << static_cast<int>(fru)
      << "_ssif_cper_" << std::put_time(time_info, "%Y%m%d%H%M%S")
      << "_" << timestamp;
  auto faultLogFile = fs::path(oss.str());

  std::ofstream ofs;
  ofs.exceptions(std::ofstream::failbit | std::ofstream::badbit);
  try
  {
    ofs.open(faultLogFile);
    ofs.write(reinterpret_cast<const char*>(data), dataSize);
    ofs.close();
  }
  catch (const std::ios_base::failure& e)
  {
    syslog(LOG_ERR, "Failed to save CPER to %s, %s.",
    faultLogFile.c_str(), e.what());
    return cper::CPER_HANDLE_FAIL;
  }

  try
  {
    std::string entry;
    auto filename = faultLogFile.filename().string();

    if (parseNvSsifCperFile(data+3, dataSize-3, entry) != cper::CPER_HANDLE_SUCCESS)
    {
      throw std::runtime_error("Failed to parse CPER file: " + filename);
    }

    syslog(LOG_CRIT, "SEL Entry: FRU: %u, Record: Facebook Unified SEL (0xFB), GeneralInfo: ProcessorErr(0x2F), Location: Socket 00, %s, Log: %s",
      fru, entry.c_str(), filename.c_str());
  }
  catch(const std::exception& e)
  {
    syslog(LOG_ERR, "Failed to create CPER dump entry, %s.", e.what());
    return cper::CPER_HANDLE_FAIL;
  }

  return cper::CPER_HANDLE_SUCCESS;
}