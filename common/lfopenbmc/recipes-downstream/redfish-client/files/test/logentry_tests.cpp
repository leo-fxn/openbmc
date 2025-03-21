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

    std::unique_ptr<sdbusplus::exception::generated_event_base>
        makeTestRedfishEvent()
    {
        using LoggingLevel =
            sdbusplus::common::xyz::openbmc_project::logging::Entry::Level;
        int severity = (int)LoggingLevel::Informational;
        std::map<std::string, std::string> additionalData;
        additionalData["SomeKey"] =
            std::string("Test exception value for SomeKey.");
        return makeRedfishEvent(severity, additionalData);
    }

    bool testBodyExecuted = false;
    FakeLogManager logManager;
    sdbusplus::async::context ctx;
};

TEST_F(LogEntryTests, BasicCommit)
{
    using LoggingCleared =
        sdbusplus::event::xyz::openbmc_project::Logging::Cleared;

    logManager.performDebugDump = true;
    ctx.spawn([this]() -> sdbusplus::async::task<> {
        auto testEvent1 = LoggingCleared("NUMBER_OF_LOGS", 3);
        auto path = lg2::commit(std::move(testEvent1));
        EXPECT_EQ(path.str,
                  std::string("/xyz/openbmc_project/logging/entry/1"));
        {
            EXPECT_EQ(1, logManager.callLogs->size());
            auto& lastCallLog = logManager.callLogs->back();

            EXPECT_EQ(LoggingLevel::Informational, lastCallLog.severity);
            EXPECT_EQ(std::string("xyz.openbmc_project.Logging.Cleared"),
                      lastCallLog.message);
            EXPECT_EQ(std::string("3"),
                      lastCallLog.additionalData["NUMBER_OF_LOGS"]);
        }

        path = lg2::commit(LoggingCleared("NUMBER_OF_LOGS", 3));
        EXPECT_EQ(path.str,
                  std::string("/xyz/openbmc_project/logging/entry/2"));
        {
            EXPECT_EQ(2, logManager.callLogs->size());
            auto& lastCallLog = logManager.callLogs->back();

            EXPECT_EQ(LoggingLevel::Informational, lastCallLog.severity);
            EXPECT_EQ(std::string("xyz.openbmc_project.Logging.Cleared"),
                      lastCallLog.message);
            EXPECT_EQ(std::string("3"),
                      lastCallLog.additionalData["NUMBER_OF_LOGS"]);
        }
        testBodyExecuted = true;
        co_return;
    }());
}

TEST_F(LogEntryTests, CommitRedfishEvent)
{
    logManager.performDebugDump = true;
    ctx.spawn([this]() -> sdbusplus::async::task<> {
        auto testEvent = makeTestRedfishEvent();
        EXPECT_STREQ("xyz.openbmc_project.Logging.ManualException",
                     testEvent->name());
        auto path = lg2::commit(std::move(*testEvent));
        EXPECT_EQ(path.str,
                  std::string("/xyz/openbmc_project/logging/entry/1"));
        {
            EXPECT_EQ(1, logManager.callLogs->size());
            auto& lastCallLog = logManager.callLogs->back();

            EXPECT_EQ(LoggingLevel::Informational, lastCallLog.severity);
            EXPECT_EQ(
                std::string("xyz.openbmc_project.Logging.ManualException"),
                lastCallLog.message);
            EXPECT_EQ(std::string("Test exception value for SomeKey."),
                      lastCallLog.additionalData["SomeKey"]);
        }

        testBodyExecuted = true;
        co_return;
    }());
}

TEST_F(LogEntryTests, CommitLoop)
{
    // No need for verbose debug output in "loop" test.
    logManager.performDebugDump = false;
    for (int i = 0; i < 100; i++)
    {
        ctx.spawn([this](int index) -> sdbusplus::async::task<> {
            auto testEvent = makeTestRedfishEvent();
            (void)lg2::commit(std::move(*testEvent));
            EXPECT_EQ(index + 1, logManager.callLogs->size());
            co_return;
        }(i));
    }
    ctx.spawn([this]() -> sdbusplus::async::task<> {
        testBodyExecuted = true;
        co_return;
    }());
}

static constexpr const char* kEventlogEntryCollectionJson = R"(
    {
        "@odata.id": "/redfish/v1/Systems/SomeBaseBoard/LogServices/EventLog/Entries",
        "@odata.type": "#LogEntryCollection.LogEntryCollection",
        "Members@odata.count": 2,
        "Members": [
            {
                "@odata.id": "/redfish/v1/Systems/SomeBaseBoard/LogServices/EventLog/Entries/102",
                "@odata.type": "#LogEntry.v1_15_0.LogEntry",
                "Created": "2025-01-09T06:32:02+00:00",
                "Modified": "2025-01-14T20:55:46+00:00",
                "EntryType": "Event",
                "Id": "102",
                "MessageId": "Some message id 1.",
                "Message": "Some message 1.",
                "MessageArgs": [
                    "message args 0",
                    "message args 1"
                ],
                "Name": "System Event Log Entry",
                "Resolution": "Some resolution.",
                "Resolved": false,
                "Severity": "Warning"
            },
            {
                "@odata.id": "/redfish/v1/Systems/SomeBaseBoard/LogServices/EventLog/Entries/103",
                "@odata.type": "#LogEntry.v1_15_0.LogEntry",
                "Created": "2025-01-14T21:12:42+00:00",
                "EntryType": "Event",
                "Id": "103",
                "MessageId": "Some message id 2.",
                "Message": "Some message 2.",
                "Resolved": true,
                "Resolution": "Some resolution 2.",
                "Severity": "Critical",
                "CPER": {
                    "NotificationType": "some notification type",
                    "SectionType": "some section type",
                    "Oem": {
                    "some_key": "some_value"
                    }
                }
            }
        ]
    }
)";

TEST_F(LogEntryTests, EventsDbusObject)
{
    logManager.performDebugDump = true;
    ctx.spawn([this]() -> sdbusplus::async::task<> {
        auto eventsDbusObject = createEventsDbusObjectForTest();

        auto coll = redfishlib::LogEntryCollection::parseLogEntryCollection(
            kEventlogEntryCollectionJson);

        // Apply collection once.
        eventsDbusObject->applyLogEntryCollection(coll);
        EXPECT_EQ(2, logManager.callLogs->size());

        // Validate record at index 0.
        {
            auto& currentCallLog = (*logManager.callLogs)[0];
            EXPECT_EQ(LoggingLevel::Warning, currentCallLog.severity);
            EXPECT_EQ(
                std::string("xyz.openbmc_project.Logging.ManualException"),
                currentCallLog.message);
            EXPECT_EQ(std::string("Some message id 1."),
                      currentCallLog.additionalData["MessageID"]);
            EXPECT_EQ(std::string("Some message 1."),
                      currentCallLog.additionalData["Message"]);
            EXPECT_EQ(std::string("message args 0"),
                      currentCallLog.additionalData["MessageArgs_0"]);
            EXPECT_EQ(std::string("message args 1"),
                      currentCallLog.additionalData["MessageArgs_1"]);
        }

        // Validate record at index 1.
        {
            auto& currentCallLog = (*logManager.callLogs)[1];
            EXPECT_EQ(LoggingLevel::Critical, currentCallLog.severity);
            EXPECT_EQ(
                std::string("xyz.openbmc_project.Logging.ManualException"),
                currentCallLog.message);
            EXPECT_EQ(std::string("Some message 2."),
                      currentCallLog.additionalData["Message"]);
            EXPECT_EQ(std::string("Some message id 2."),
                      currentCallLog.additionalData["MessageID"]);
            EXPECT_EQ(currentCallLog.additionalData.find("MessageArgs_0"),
                      currentCallLog.additionalData.end());
        }

        // Apply again, there should be no changes (object is idempotent).
        EXPECT_EQ(2, logManager.callLogs->size());

        testBodyExecuted = true;
        co_return;
    }());
}
