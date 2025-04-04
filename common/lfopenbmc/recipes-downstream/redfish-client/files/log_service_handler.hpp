#pragma once

#include "redfish-binding/LogEntryCollection_LogEntryCollection.hpp"
#include "redfish-binding/LogEntry_EventSeverity.hpp"
#include "redfish-binding/LogEntry_LogEntry.hpp"
#include "source.hpp"

#include <sdbusplus/message.hpp>

#include <memory>
#include <string>
#include <unordered_map>

namespace redfish_client_daemon
{

class LogServiceHandler
{
  public:
    LogServiceHandler() = delete;

    explicit LogServiceHandler(const std::string& url) : url(url) {};

    void runOnce(std::shared_ptr<IRedfishSource> redfishSource);

    void applyLogEntryCollection(
        redfish_binding::LogEntryCollection::LogEntryCollection& collection);

  private:
    std::string url;
    std::unordered_map<std::string, std::string> seenEntries;

    static int getLogEntrySeverity(
        redfish_binding::LogEntry::EventSeverity severity);

    template <typename T>
    static T makeCPER(const auto& source, const auto& cper)
    {
        return T("SOURCE", sdbusplus::message::object_path(source), "CPER",
                 cper.toJson().dump());
    }
    void applyLogEntry(redfish_binding::LogEntry::LogEntry& entry);
};

} // namespace redfish_client_daemon
