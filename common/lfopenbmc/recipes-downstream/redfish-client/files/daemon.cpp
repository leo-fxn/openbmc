#include "daemon.hpp"

#include <boost/stacktrace.hpp>
#include <nlohmann/json.hpp>
#include <phosphor-logging/commit.hpp>
#include <phosphor-logging/lg2.hpp>
#include <xyz/openbmc_project/Logging/Create/client.hpp>
#include <xyz/openbmc_project/State/CPER/event.hpp>

#include <csignal>

PHOSPHOR_LOG2_USING;

// We want to run the daemon in dry-run mode for stress testing, i.e. in a mode
// where we scrape the data using our curl code and parsing logic, but don't
// actually write it. This helps us "bisect" the code into separate
// functionalities that can be stress-tested independently.
static const bool LG2_COMMIT_ALLOWED = getenv("DISABLE_LG2_COMMIT") == nullptr;

namespace redfish_client_daemon
{

using PathIntf = ValueIntf::namespace_path;

class ServerObjectIntf :
    public sdbusplus::aserver::xyz::openbmc_project::sensor::Value<
        ServerObjectIntf>
{
  public:
    explicit ServerObjectIntf(sdbusplus::async::context& ctx, auto path) :
        sdbusplus::aserver::xyz::openbmc_project::sensor::Value<
            ServerObjectIntf>(ctx, path)
    {}
};

// Source:
// https://github.com/openbmc/phosphor-dbus-interfaces/blob/master/yaml/xyz/openbmc_project/Sensor/Value.interface.yaml

std::optional<ValueIntf::Unit> toMaybeIntfUnits(const std::string& unitsStr)
{
    // The strings used in this function and the mapping to items in the
    // ValueIntf::Unit enumeration were obtained experimentally and should be
    // extended as needed.

    if (unitsStr == "%")
    {
        return ValueIntf::Unit::Percent;
    }

    if (unitsStr == "Cel")
    {
        return ValueIntf::Unit::DegreesC;
    }

    if (unitsStr == "J")
    {
        return ValueIntf::Unit::Joules;
    }

    if (unitsStr == "Pa")
    {
        return ValueIntf::Unit::Pascals;
    }

    if (unitsStr == "V")
    {
        return ValueIntf::Unit::Volts;
    }

    if (unitsStr == "W")
    {
        return ValueIntf::Unit::Watts;
    }

    return std::nullopt;
}

// Source:
// https://github.com/openbmc/phosphor-dbus-interfaces/blob/master/yaml/xyz/openbmc_project/Sensor/Value.interface.yaml

const char* getActualMetricNamespace(const char* logicalNameParam)
{
    // This function is an identify function for the time being, but
    // having a lookup table infrastructure makes it possible for us to
    // change the values supported in the interface without worrying about
    // the values actually emitted by the redfish server.

    std::string logicalName(logicalNameParam);
    if (logicalName == "airflow")
    {
        return PathIntf::airflow;
    }
    if (logicalName == "altitude")
    {
        return PathIntf::altitude;
    }
    if (logicalName == "current")
    {
        return PathIntf::current;
    }
    if (logicalName == "energy")
    {
        return PathIntf::energy;
    }
    if (logicalName == "fan_tach")
    {
        return PathIntf::fan_tach;
    }
    if (logicalName == "humidity")
    {
        return PathIntf::humidity;
    }
    if (logicalName == "liquidflow")
    {
        return PathIntf::liquidflow;
    }
    if (logicalName == "power")
    {
        return PathIntf::power;
    }
    if (logicalName == "pressure")
    {
        return PathIntf::pressure;
    }
    if (logicalName == "temperature")
    {
        return PathIntf::temperature;
    }
    if (logicalName == "utilization")
    {
        return PathIntf::utilization;
    }
    if (logicalName == "voltage")
    {
        return PathIntf::voltage;
    }
    throw std::invalid_argument(logicalName.c_str());
}

DaemonConfig DaemonConfig::fromJson(const std::string& configJson)
{
    using json = nlohmann::json;
    DaemonConfig rv;
    json parsed = json::parse(configJson);
    rv.serviceName = parsed["service_name"].get<std::string>();
    rv.hostPort = parsed["host_port"].get<std::string>();
    auto& parsedSensorConfigs = parsed["sensor_configs"];
    for (json::iterator it = parsedSensorConfigs.begin();
         it != parsedSensorConfigs.end(); ++it)
    {
        SensorConfigValue item;
        std::string expandedUrl = "http://";
        expandedUrl += rv.hostPort;
        expandedUrl += it.value()["url"].get<std::string>();
        item.url = expandedUrl;
        item.symbolicNamespace =
            it.value()["symbolic_namespace"].get<std::string>();

        rv.sensorConfigs[it.key()] = item;
    }
    rv.intervalMilliseconds = parsed["interval_milliseconds"].get<size_t>();
    rv.retries = parsed["retries"].get<int>();
    rv.waitMilliseconds = parsed["wait_milliseconds"].get<size_t>();

    static constexpr auto kEventLogConfigKey = "event_log_configs";
    if (parsed.contains(kEventLogConfigKey))
    {
        auto parsedEventLogConfigs = parsed[kEventLogConfigKey];
        EventLogConfigs configs;
        configs.intervalMilliseconds =
            parsedEventLogConfigs["eventlog_interval_milliseconds"]
                .get<size_t>();
        auto& parsedPaths = parsedEventLogConfigs["paths"];
        for (json::iterator it = parsedPaths.begin(); it != parsedPaths.end();
             ++it)
        {
            std::string expandedUrl = "http://";
            expandedUrl += rv.hostPort;
            expandedUrl += it.value().get<std::string>();
            configs.urls.push_back(expandedUrl);
        }

        if (configs.urls.size() > 0)
        {
            rv.eventLogConfigs = configs;
        }
    }

    return rv;
}

const char* getSensorRootPath()
{
    return PathIntf::value;
}

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

struct EventsDbusObject : IEventsDbusObject
{
    EventsDbusObject() = delete;
    EventsDbusObject(const EventsDbusObject&) = delete;
    EventsDbusObject(EventsDbusObject&&) = delete;

    EventsDbusObject(const std::string& url) : url(url) {}

    void runOnce(std::shared_ptr<IRedfishSource> redfishSource)
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
            auto coll = redfishlib::LogEntryCollection::parseLogEntryCollection(
                logEntryJson);
            applyLogEntryCollection(coll);
        }
        catch (const std::exception& exn)
        {
            info("Exception while parsing url response {EXC}", "EXC",
                 exn.what());
            debug("Exception while querying url response: {URL}", "URL",
                  url.c_str());
        };
    }

    void applyLogEntryCollection(
        redfishlib::LogEntryCollection::LogEntryCollection& collection) override
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

  private:
    static int getLogEntrySeverity(redfishlib::LogEntry::EventSeverity severity)
    {
        switch (severity)
        {
            case redfishlib::LogEntry::EventSeverity::OK:
                return LOG_INFO;

            case redfishlib::LogEntry::EventSeverity::Warning:
                return LOG_WARNING;

            case redfishlib::LogEntry::EventSeverity::Critical:
                return LOG_CRIT;

            default:
                return LOG_ERR;
        }
    }
    template <typename T>
    static T makeCPER(const auto& source, const auto& cper)
    {
        return T("SOURCE", sdbusplus::message::object_path(source), "CPER",
                 cper.toJson().dump());
    }

    void applyLogEntry(redfishlib::LogEntry::LogEntry& entry)
    {
        namespace CPER = sdbusplus::error::xyz::openbmc_project::state::CPER;

        if (!entry.getId().hasValue())
        {
            return;
        }
        auto& entryId = entry.getId().value();
        if (seenEntries.find(entryId) != seenEntries.end())
        {
            return;
        }
        seenEntries.insert(entryId);
        try
        {
            auto& maybeRedfishSeverity = entry.getSeverity();
            auto& maybeCPER = entry.getCPER();
            auto& maybeLinks = entry.getLinks();

            auto severity =
                maybeRedfishSeverity.hasValue()
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
                    auto& id = origin.value().get_odata_id();
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

    std::string url;
    std::unordered_set<std::string> seenEntries;
};

std::shared_ptr<IEventsDbusObject> createEventsDbusObjectForTest()
{
    std::string url = "fake url";
    return std::make_shared<EventsDbusObject>(url);
}

struct SensorDbusObject
{
    SensorDbusObject() = delete;
    SensorDbusObject(const SensorDbusObject&) = delete;
    SensorDbusObject(SensorDbusObject&&) = delete;

    SensorDbusObject(sdbusplus::async::context& ctx, const char* metricPath,
                     const SensorConfigValue& sensorConfigValue) :
        ctx(ctx), metricPath(metricPath), sensorConfigValue(sensorConfigValue)
    {}

    auto update(Sensor&& sensor) -> sdbusplus::async::task<>
    {
        // Uncomment this to debug. If we leave this uncommented in the code
        // then phosphor logging always prints these in TTY mode and that makes
        // manual testing a bit difficult.
        //
        // Note:
        // https://github.com/openbmc/phosphor-logging/blob/master/docs/structured-logging.md
        // "The lg2 APIs detect if the application is running on a TTY and
        // additionally log to the TTY."
        //
        // debug("Updating metric {NAME}", "NAME", metricPath.c_str());
        std::lock_guard<std::mutex> guard(this->lock);
        bool addedThisTime = false;

        if (object == nullptr)
        {
            // Don't signal the bus while the object is getting created.
            constexpr bool emitSignal = false;
            object =
                std::make_unique<ServerObjectIntf>(ctx, metricPath.c_str());

            auto maybeUnit = toMaybeIntfUnits(sensor.getSensorUnitText());
            if (maybeUnit.has_value())
            {
                object->unit<emitSignal>(maybeUnit.value());
            }
            else
            {
                // Initialize it to something, otherwise we see errors on some
                // OS versions.
                object->unit<emitSignal>(ValueIntf::Unit::Amperes);
            }

            object->value<emitSignal>(std::numeric_limits<double>::quiet_NaN());
            assert(std::isnan(lastNotifiedValue));
            addedThisTime = true;
        }

        if (!minValueNotified)
        {
            auto minValue = sensor.getMinValue();
            if (minValue.has_value())
            {
                object->min_value<false>(minValue.value());
                minValueNotified = true;
            }
        }
        if (!maxValueNotified)
        {
            auto maxValue = sensor.getMaxValue();
            if (maxValue.has_value())
            {
                object->max_value<false>(maxValue.value());
                maxValueNotified = true;
            }
        }

        double current = sensor.getReading();
        const bool emitSignal = !shouldSkipSignal(current);
        if (emitSignal)
        {
            lastNotifiedValue = current;
        }
        if (emitSignal && !addedThisTime)
        {
            object->value<true>(current);
        }
        else
        {
            object->value<false>(current);
        }
        if (addedThisTime)
        {
            object->emit_added();
        }
        co_return;
    }

    auto shouldSkipSignal(double current) -> bool
    {
        if (std::isnan(lastNotifiedValue) && std::isnan(current))
        {
            return true;
        }
        if (std::isnan(lastNotifiedValue) && !(std::isnan(current)))
        {
            return false;
        }
        if ((!std::isnan(lastNotifiedValue)) && std::isnan(current))
        {
            return false;
        }
        assert(!std::isnan(lastNotifiedValue));
        assert(!std::isnan(current));
        if (std::abs(lastNotifiedValue) < 1e-6)
        {
            // avoid division by zero
            return true;
        }
        auto changed =
            std::abs((current - lastNotifiedValue) / lastNotifiedValue * 100.0);
        if (changed >= 1.0)
        {
            return false;
        }
        return true;
    }

    sdbusplus::async::context& ctx;
    std::mutex lock;
    std::string metricPath;
    std::unique_ptr<ServerObjectIntf> object;
    double lastNotifiedValue = std::numeric_limits<double>::quiet_NaN();
    bool minValueNotified = false;
    bool maxValueNotified = false;
    SensorConfigValue sensorConfigValue;
};

class DbusServer
{
  public:
    DbusServer() = delete;

    explicit DbusServer(sdbusplus::async::context& ctx,
                        const DaemonConfig& daemonConfig,
                        std::shared_ptr<IRedfishSource> redfishSource) :
        ctx(ctx), daemonConfig(daemonConfig), redfishSource(redfishSource)
    {
        info("Creating Dbus Server with sensor config size {SIZE}", "SIZE",
             daemonConfig.sensorConfigs.size());

        info("Creating Sensor dbus objects");
        for (const auto& [sensorConfigKey, sensorConfig] :
             daemonConfig.sensorConfigs)
        {
            auto metricNamespace = std::string(getActualMetricNamespace(
                sensorConfig.symbolicNamespace.c_str()));

            std::string fullMetricPath =
                std::string(getSensorRootPath()) + std::string("/") +
                metricNamespace + sensorConfigKey;

            metrics[sensorConfigKey] = std::make_shared<SensorDbusObject>(
                ctx, fullMetricPath.c_str(), sensorConfig);
        }

        if (daemonConfig.eventLogConfigs.has_value())
        {
            const auto& eventLogConfigs = daemonConfig.eventLogConfigs.value();
            info("eventLogConfigs intervalMilliseconds = {INTERVAL}",
                 "INTERVAL", eventLogConfigs.intervalMilliseconds);
            for (const auto& url : eventLogConfigs.urls)
            {
                info("eventLogConfigs url = {URL}", "URL", url.c_str());
                eventObjects.push_back(std::make_shared<EventsDbusObject>(url));
            }
        }

        ctx.spawn(runEventPollingLoop());
        sensorThread = std::thread([this] { runSensorLoop(); });
    }

    ~DbusServer()
    {
        ctx.request_stop();
        if (sensorThread.joinable())
        {
            sensorThread.join();
        }
    }

  private:
    std::optional<Sensor> readWithRetries(
        const SensorConfigValue& sensorConfigValue)
    {
        for (int i = 0; i < daemonConfig.retries; ++i)
        {
            std::string sensorJson;

            try
            {
                sensorJson = redfishSource->getBody(sensorConfigValue.url);
            }
            catch (const std::exception& exn)
            {
                info("Exception while querying url {EXC}", "EXC", exn.what());
                debug("Exception while querying url: {URL}", "URL",
                      sensorConfigValue.url.c_str());
            };

            try
            {
                return Sensor::parseSensor(sensorJson);
            }
            catch (const std::exception& exn)
            {
                info("Exception while parsing sensor json: {EXC}", "EXC",
                     exn.what());
                debug("Exception while parsing sensor json: {JSON}", "JSON",
                      sensorJson.c_str());
            };

            std::this_thread::sleep_for(
                std::chrono::milliseconds(daemonConfig.waitMilliseconds));
        }
        return std::nullopt;
    }

    auto runEventPollingLoop() -> sdbusplus::async::task<>
    {
        if (eventObjects.size() == 0)
        {
            co_return;
        }

        info("Running DBus Server event polling loop");

        try
        {
            while (!ctx.stop_requested())
            {
                for (auto& eventObj : eventObjects)
                {
                    eventObj->runOnce(redfishSource);
                }

                co_await sdbusplus::async::sleep_for(
                    ctx, std::chrono::milliseconds(
                             daemonConfig.eventLogConfigs.value()
                                 .intervalMilliseconds));
            }
        }
        catch (const std::logic_error& exn)
        {
            debug("Unhandled logic error: {NAME}", "WHAT", exn.what());
        };

        co_return;
    }

    void runSensorLoop()
    {
        info("Running DBus Server sensor loop");
        try
        {
            while (!ctx.stop_requested())
            {
                for (const auto& [metricKey, metric] : metrics)
                {
                    auto maybeSensor =
                        readWithRetries(metric->sensorConfigValue);
                    if (!maybeSensor.has_value())
                    {
                        continue;
                    }
                    // Sensor object is only a few bytes. Lets explicitly copy
                    // the values to a movable object so that ownership &
                    // destruction happen once the task is processed.
                    Sensor sensor = maybeSensor.value();
                    ctx.spawn(metric->update(std::move(sensor)));
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(
                    daemonConfig.intervalMilliseconds));
            }
        }
        catch (const std::logic_error& exn)
        {
            debug("Unhandled logic error: {NAME}", "WHAT", exn.what());
        };
    }

    sdbusplus::async::context& ctx;
    std::unordered_map<std::string, std::shared_ptr<SensorDbusObject>> metrics;
    std::vector<std::shared_ptr<EventsDbusObject>> eventObjects;
    DaemonConfig daemonConfig;
    std::shared_ptr<IRedfishSource> redfishSource;
    std::thread sensorThread;
};

void installSignalHandlers()
{
    auto printStackTraceOnCrashHandler = [](int signal) {
        boost::stacktrace::stacktrace st;
        std::string stacktrace_str = boost::stacktrace::to_string(st);
        fprintf(stderr, "Uncaught exception:\n%s\n", stacktrace_str.c_str());
        _exit(signal);
    };
    std::signal(SIGSEGV, printStackTraceOnCrashHandler);
    std::signal(SIGABRT, printStackTraceOnCrashHandler);
}

void runDbusServerTillInterrupted(const DaemonConfig& daemonConfig,
                                  sdbusplus::async::context& ctx,
                                  std::shared_ptr<IRedfishSource> redfishSource)
{
    sdbusplus::server::manager_t manager{ctx, getSensorRootPath()};

    info("Creating DbusServer");

    ctx.spawn([](sdbusplus::async::context& ctx,
                 const DaemonConfig& daemonConfig) -> sdbusplus::async::task<> {
        ctx.request_name(daemonConfig.serviceName.c_str());
        co_return;
    }(ctx, daemonConfig));

    DbusServer server(ctx, daemonConfig, redfishSource);

    ctx.run();
}

struct SensorDbusObjectForTest : public ISensorDbusObject
{
    SensorDbusObjectForTest() = delete;
    SensorDbusObjectForTest(const SensorDbusObjectForTest&) = delete;
    SensorDbusObjectForTest(SensorDbusObjectForTest&&) = delete;

    SensorDbusObjectForTest(sdbusplus::async::context& ctx,
                            const char* metricPath,
                            const SensorConfigValue& sensorConfigValue) :
        ctx(ctx), innerObject(std::make_shared<SensorDbusObject>(
                      ctx, metricPath, sensorConfigValue))
    {}

    sdbusplus::async::task<> update(const Sensor& sensor) override
    {
        Sensor sensorCopy = sensor;
        co_await innerObject->update(std::move(sensorCopy));
        co_return;
    }

    sdbusplus::async::context& ctx;
    std::shared_ptr<SensorDbusObject> innerObject;
};

std::shared_ptr<ISensorDbusObject> createSensorDbusObjectForTest(
    sdbusplus::async::context& ctx, const char* metricPath)
{
    SensorConfigValue fakeSensorConfigValue;
    return std::make_shared<SensorDbusObjectForTest>(ctx, metricPath,
                                                     fakeSensorConfigValue);
}

} // namespace redfish_client_daemon
