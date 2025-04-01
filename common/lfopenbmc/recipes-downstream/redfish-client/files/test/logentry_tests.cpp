#include "daemon.hpp"

#include <nlohmann/json.hpp>
#include <phosphor-logging/commit.hpp>
#include <sdbusplus/server.hpp>
#include <xyz/openbmc_project/Logging/Create/client.hpp>
#include <xyz/openbmc_project/Logging/Create/common.hpp>
#include <xyz/openbmc_project/Logging/Create/server.hpp>
#include <xyz/openbmc_project/Logging/event.hpp>

#include <condition_variable>
#include <mutex>
#include <thread>

#include <gtest/gtest.h>

using namespace redfish_client_daemon;

using LoggingLevel =
    sdbusplus::common::xyz::openbmc_project::logging::Entry::Level;

using CreateSyncIntf = sdbusplus::server::object_t<
    sdbusplus::server::xyz::openbmc_project::logging::Create>;

class FakeLogManager
{
  public:
    struct CallLog
    {
        CallLog(bool performDebugDump, const std::string& message,
                LoggingLevel severity,
                const std::map<std::string, std::string>& additionalData) :
            message(message), severity(severity), additionalData(additionalData)
        {
            // For some tests, it actually makes sense to dump the request to
            // see how the exception object was formatted/constructed.
            if (performDebugDump)
            {
                printf("Received a create request:\n");
                printf("  message = \"%s\"\n", message.c_str());
                printf("  severity = %d\n", (int)severity);
                for (const auto& [k, v] : additionalData)
                {
                    printf("  additionalData[\"%s\"] == \"%s\"\n", k.c_str(),
                           v.c_str());
                }
                fflush(stdout);
            }
        }

        std::string message;
        LoggingLevel severity;
        std::map<std::string, std::string> additionalData;
    };
    std::shared_ptr<std::vector<CallLog>> callLogs;

    FakeLogManager() : callLogs(std::make_shared<std::vector<CallLog>>()) {}

    bool performDebugDump = false;

    void start()
    {
        ASSERT_FALSE(serverRunning);
        ASSERT_TRUE(serverThread == nullptr);

        std::mutex lock;
        std::condition_variable condVar;
        bool serverStarted = false;

        serverRunning = true;
        serverThread = std::make_unique<std::thread>([this, &lock, &condVar,
                                                      &serverStarted]() {
            auto bus = sdbusplus::bus::new_default();

            // Instance path: "/xyz/openbmc_project/logging"
            sdbusplus::server::manager_t mgr{bus, Create::instance_path};

            // Default service: "xyz.openbmc_project.Logging"
            bus.request_name(Create::default_service);

            // Instance path: "/xyz/openbmc_project/logging"
            Create impl{bus, Create::instance_path, callLogs, performDebugDump};

            // Notify the client thread.
            {
                {
                    std::lock_guard<std::mutex> guard(lock);
                    serverStarted = true;
                }
                condVar.notify_all();
            }

            // Handle dbus processing while the server is running.
            sdbusplus::SdBusDuration timeout = std::chrono::milliseconds(100);
            while (serverRunning)
            {
                bus.process_discard();
                bus.wait(timeout);
            }
        });

        // Wait for the server to start.
        {
            std::unique_lock<std::mutex> ulock(lock);
            condVar.wait(ulock, [this, &lock, &condVar, &serverStarted] {
                return serverStarted;
            });
        }
    }

    void stop()
    {
        ASSERT_TRUE(serverThread != nullptr);
        ASSERT_TRUE(serverRunning);

        serverRunning = false;
        serverThread->join();
        serverThread = nullptr;
    }

  private:
    bool serverRunning = false;

    struct Create : CreateSyncIntf
    {
        int64_t nextId = 1;
        std::shared_ptr<std::vector<CallLog>> callLogs;
        bool& performDebugDump;

        Create(sdbusplus::bus_t& bus, const char* path,
               std::shared_ptr<std::vector<CallLog>> callLogs,
               bool& performDebugDump) :
            CreateSyncIntf(bus, path), callLogs(callLogs),
            performDebugDump(performDebugDump)
        {}

        virtual ~Create() {}

        sdbusplus::message::object_path create(
            std::string message, LoggingLevel severity,
            std::map<std::string, std::string> additionalData) override
        {
            callLogs->emplace_back(performDebugDump, message, severity,
                                   additionalData);
            // Instance path: "/xyz/openbmc_project/logging"
            std::string pathStr = Create::instance_path;
            pathStr += "/entry/";
            pathStr += std::to_string(nextId++);
            sdbusplus::message::object_path path(pathStr);
            return path;
        }

        void createWithFFDCFiles(
            std::string message, LoggingLevel severity,
            std::map<std::string, std::string> additionalData,
            std::vector<std::tuple<FFDCFormat, uint8_t, uint8_t,
                                   sdbusplus::message::unix_fd>>
                ffdc) override
        {
            // This test does not exercise the FFDC code path.
            ASSERT_TRUE(false);
        }
    };

    std::unique_ptr<std::thread> serverThread;
};

class LogEntryTests : public ::testing::Test
{
  protected:
    LogEntryTests() {}

    ~LogEntryTests() noexcept override {}

    void SetUp() override
    {
        redfish_client_daemon::installSignalHandlers();
        logManager.start();
    }

    void TearDown() override
    {
        ctx.spawn(
            [](sdbusplus::async::context& ctx) -> sdbusplus::async::task<> {
                ctx.request_stop();
                co_return;
            }(ctx));
        ctx.run();
        ASSERT_TRUE(testBodyExecuted);
        logManager.stop();
    }

    bool testBodyExecuted = false;
    FakeLogManager logManager;
    sdbusplus::async::context ctx;
};

static constexpr const char* kEventlogEntryCollectionJson = R"(
{
    "@odata.id": "/redfish/v1/Systems/SomeBaseBoard/LogServices/EventLog/Entries",
    "@odata.type": "#LogEntryCollection.LogEntryCollection",
    "Members@odata.count": 2,
    "Members": [
        {
            "@odata.id": "/redfish/v1/Managers/System0/LogServices/EventLog/Entries/101",
            "@odata.type": "#LogEntry.v1_15_0.LogEntry",
            "Created": "2025-01-01T12:00:00+00:00",
            "EntryType": "Event",
            "Id": "101",
            "Message": "CPU sensor crossed a warning high threshold going high. Reading=100.123456 Threshold=80.000000.",
            "MessageArgs": [
                "CPU",
                "100.123456",
                "80.000000"
            ],
            "MessageId": "OpenBMC.0.4.SensorThresholdWarningHighGoingHigh",
            "Name": "Manager Event Log Entry",
            "Resolution": "None",
            "Resolved": false,
            "Severity": "Warning"
        },
        {
            "@odata.id": "/redfish/v1/Systems/System0/LogServices/EventLog/Entries/102",
            "@odata.type": "#LogEntry.v1_15_0.LogEntry",
            "CPER": {
                "NotificationType": "3d61a466-ab40-409a-a698-f362d464b38f",
                "SectionType": "6d5244f2-2712-11ec-bea7-cb3fdb95c786"
            },
            "Created": "2025-01-01T12:00:00+00:00",
            "DiagnosticDataType": "CPERSection",
            "EntryType": "Event",
            "Id": "102",
            "Links": {
                "OriginOfCondition": {
                    "@odata.id": "/redfish/v1/Systems/System0/Processors/CPU_1"
                }
            },
            "Message": "A platform error occurred.",
            "MessageArgs": [],
            "MessageId": "Platform.1.0.PlatformError",
            "Name": "System Event Log Entry",
            "Resolution": "Check additional diagnostic data if available.",
            "Resolved": false,
            "Severity": "Critical"
        }
    ]
}
)";

TEST_F(LogEntryTests, ParseEventsCorrectly)
{
    logManager.performDebugDump = true;
    ctx.spawn([this]() -> sdbusplus::async::task<> {
        auto eventsDbusObject = createEventsDbusObjectForTest();

        auto coll = redfish_binding::LogEntryCollection::parseLogEntryCollection(
            kEventlogEntryCollectionJson);

        using namespace std::string_literals;

        // Apply collection once.
        eventsDbusObject->applyLogEntryCollection(coll);
        EXPECT_EQ(2, logManager.callLogs->size());

        // Validate record at index 0 (Unexpected Exception).
        {
            auto& currentCallLog = (*logManager.callLogs)[0];
            EXPECT_EQ(LoggingLevel::Warning, currentCallLog.severity);
            EXPECT_EQ("com.meta.RedfishClient.UnexpectedException"s,
                      currentCallLog.message);

            auto js = nlohmann::json::parse(
                currentCallLog.additionalData["REDFISH_EVENT"]);

            EXPECT_EQ("OpenBMC.0.4.SensorThresholdWarningHighGoingHigh"s,
                      js.at("MessageId").get<std::string>());
        }

        // Validate record at index 1 (CPER event).
        {
            auto& currentCallLog = (*logManager.callLogs)[1];
            EXPECT_EQ(LoggingLevel::Critical, currentCallLog.severity);
            EXPECT_EQ("xyz.openbmc_project.State.CPER.GenericCPERFault"s,
                      currentCallLog.message);

            EXPECT_EQ("/redfish/v1/Systems/System0/Processors/CPU_1"s,
                      currentCallLog.additionalData["SOURCE"]);

            auto js =
                nlohmann::json::parse(currentCallLog.additionalData["CPER"]);

            EXPECT_EQ("3d61a466-ab40-409a-a698-f362d464b38f"s,
                      js.at("NotificationType").get<std::string>());
        }

        // Apply again, there should be no changes (object is idempotent).
        EXPECT_EQ(2, logManager.callLogs->size());

        testBodyExecuted = true;
        co_return;
    }());
}
