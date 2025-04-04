#include "log_service_handler.hpp"

#include <phosphor-logging/commit.hpp>
#include <phosphor-logging/lg2.hpp>
#include <xyz/openbmc_project/State/CPER/event.hpp>

#include <exception>

PHOSPHOR_LOG2_USING;

namespace redfish_client_daemon
{

class UnhandledException : public sdbusplus::exception::generated_event_base
{
  public:
    explicit UnhandledException(int eventSeverity,
                                const nlohmann::json& eventData) :
        eventSeverity(eventSeverity), eventData(eventData)
    {}

    const char* name() const noexcept override
    {
        return kExceptionName;
    }

    const char* description() const noexcept override
    {
        return "Unhandled Exception from Satellite MC";
    }

    int get_errno() const noexcept override
    {
        return EIO;
    }

    nlohmann::json to_json() const override
    {
        nlohmann::json j = {};
        j["REDFISH_EVENT"] = eventData.dump();

        return nlohmann::json{{kExceptionName, std::move(j)}};
    }

    int severity() const noexcept override
    {
        return eventSeverity;
    }

  private:
    int eventSeverity;
    nlohmann::json eventData;
    static constexpr const char* kExceptionName =
        "com.meta.RedfishClient.UnexpectedException";
};

void LogServiceHandler::runOnce(std::shared_ptr<IRedfishSource> redfishSource)
{
    std::string logEntryJson;

    try
    {
        logEntryJson = redfishSource->getBody(url);
    }
    catch (const std::exception& exn)
    {
        info("Exception while querying url {EXC}", "EXC", exn.what());
        debug("Exception while querying url: {URL}", "URL", url.c_str());
        return;
    };

    try
    {
        auto coll =
            redfish_binding::LogEntryCollection::parseLogEntryCollection(
                logEntryJson);
        applyLogEntryCollection(coll);
    }
    catch (const std::exception& exn)
    {
        info("Exception while parsing url response {EXC}", "EXC", exn.what());
        debug("Exception while querying url response: {URL}", "URL",
              url.c_str());
    };
}

void LogServiceHandler::applyLogEntryCollection(
    redfish_binding::LogEntryCollection::LogEntryCollection& collection)
{
    auto& maybeMembers = collection.getMembers();
    if (maybeMembers.hasValue())
    {
        for (auto& member : maybeMembers.value())
        {
            applyLogEntry(member);
        }
    }
}

int LogServiceHandler::getLogEntrySeverity(
    redfish_binding::LogEntry::EventSeverity severity)
{
    switch (severity)
    {
        case redfish_binding::LogEntry::EventSeverity::OK:
            return LOG_INFO;

        case redfish_binding::LogEntry::EventSeverity::Warning:
            return LOG_WARNING;

        case redfish_binding::LogEntry::EventSeverity::Critical:
            return LOG_CRIT;

        default:
            return LOG_ERR;
    }
}

void LogServiceHandler::applyLogEntry(
    redfish_binding::LogEntry::LogEntry& entry)
{
    namespace CPER = sdbusplus::error::xyz::openbmc_project::state::CPER;

    if (!entry.getId().hasValue())
    {
        return;
    }
    auto& entryId = entry.getId().value();
    if (!entry.getCreated().hasValue())
    {
        return;
    }
    const auto& timestamp = entry.getCreated().value();
    auto it = seenEntries.find(entryId);
    if (it != seenEntries.end() && it->second == timestamp)
    {
        return;
    }
    seenEntries[entryId] = timestamp;
    try
    {
        auto& maybeRedfishSeverity = entry.getSeverity();
        auto& maybeCPER = entry.getCPER();
        auto& maybeLinks = entry.getLinks();

        auto severity = maybeRedfishSeverity.hasValue()
                            ? getLogEntrySeverity(maybeRedfishSeverity.value())
                            : LOG_CRIT;

        // If the event has a CPER section, it must be a CPER.  Translate
        // it into the GenericCPER* exceptions.
        if (maybeCPER.hasValue())
        {
            auto source = [&]() -> std::string {
                auto unknownSource = "Unknown Source";

                if (!maybeLinks.hasValue())
                {
                    return unknownSource;
                }
                auto& origin = maybeLinks.value().getOriginOfCondition();
                if (!origin.hasValue())
                {
                    return unknownSource;
                }
                auto& id = origin.value().getOdataId();
                if (!id.hasValue())
                {
                    return unknownSource;
                }
                return id.value();
            }();

            if (severity == LOG_CRIT)
            {
                lg2::commit(makeCPER<CPER::GenericCPERFault>(
                    source, maybeCPER.value()));
            }
            else
            {
                lg2::commit(makeCPER<CPER::GenericCPERWarning>(
                    source, maybeCPER.value()));
            }
            return;
        }

        lg2::commit(UnhandledException(severity, entry.toJson()));
    }
    catch (const std::exception& e)
    {
        error(
            "Could not commit event log entry for entry id {ENTRY_ID}: {ERROR}",
            "ENTRY_ID", entryId.c_str(), "ERROR", e);
        return;
    }
}

} // namespace redfish_client_daemon
