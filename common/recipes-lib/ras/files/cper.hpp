/*
 * Copyright 2024-present Facebook. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#pragma once

#include <map>
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace cper
{

constexpr auto CPER_DUMP_PATH = "/mnt/data/faultlog/cper/";
constexpr auto CPER_CONVERT = "/usr/bin/cper-convert";
constexpr auto CPER_DUMP_MAX_LIMIT = 1024;

constexpr auto INVALID_ID = 0xFF;
constexpr auto INVALID_ERROR_DESC = "None";

using SectionHandler = int(*)(const nlohmann::ordered_json&, const nlohmann::ordered_json&, std::string&);

struct SectionHandlerEntry
{
    std::string type;
    std::string typeShortName;
    SectionHandler handler;
};

class SectionHandlerRegistry {
  public:
    explicit SectionHandlerRegistry(
        std::initializer_list<SectionHandlerEntry> initList)
    {
        handlers.insert(handlers.end(), initList.begin(), initList.end());
    }

    void addHandlerEntry(const std::string& type,
                         const std::string& typeShortName,
                         SectionHandler handler)
    {
        auto it = std::find_if(handlers.begin(), handlers.end(),
                               [&type](const SectionHandlerEntry& entry) {
                                   return entry.type == type;
                               });

        if (it != handlers.end())
        {
            // replace a handler if the type already exists
            it->typeShortName = typeShortName;
            it->handler = handler;
        }
        else
        {
            handlers.push_back({type, typeShortName, handler});
        }
    }

    SectionHandlerEntry* findHandlerEntry(const std::string& type)
    {
        auto it = std::find_if(handlers.begin(), handlers.end(),
                               [&type](const SectionHandlerEntry& entry) {
                                   return entry.type == type;
                               });

        return (it != handlers.end()) ? &(*it) : nullptr;
    }

  private:
    std::vector<SectionHandlerEntry> handlers;
};

enum CperHandleCodes
{
  CPER_HANDLE_SUCCESS = 0,
  CPER_HANDLE_FAIL
};

enum UnifiedErrorType
{
  UNIFIED_PCIE_ERR  = 0x20,
  UNIFIED_MEM_ERR   = 0x21,
  UNIFIED_PROC_ERR  = 0x2F,
};

std::string formatHex(uint32_t value, int width = 2);
int registerOemSectionHandlers(SectionHandlerRegistry& registry);
int parseCperFile(const std::string& file, std::vector<std::string>& entries);
int createCperDumpEntry(const uint8_t& payloadId, 
                        const uint8_t* data, const size_t& dataSize);

} // namespace cper