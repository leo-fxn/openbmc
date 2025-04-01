#include "redfish-binding/LogEntryCollection_LogEntryCollection.hpp"

#include <gtest/gtest.h>

TEST(EventLogParserTests, ZeroEntriesTest)
{
    static std::string kEventlogEntryCollectionJson = R"(
{
  "@odata.id": "/redfish/v1/Systems/SomeBaseBoard/LogServices/EventLog/Entries",
  "@odata.type": "#LogEntryCollection.LogEntryCollection",
  "Description": "Collection of System Event Log Entries",
  "Members@odata.count": 0,
  "Members": [
  ],
  "Name": "System Event Log Entries"
}
)";
    auto coll = redfish_binding::LogEntryCollection::parseLogEntryCollection(
        kEventlogEntryCollectionJson);
    EXPECT_STREQ(
        "/redfish/v1/Systems/SomeBaseBoard/LogServices/EventLog/Entries",
        coll.getOdataId().value().c_str());
    EXPECT_STREQ("#LogEntryCollection.LogEntryCollection",
                 coll.getOdataType().value().c_str());
    EXPECT_STREQ("Collection of System Event Log Entries",
                 coll.getDescription().value().c_str());
    EXPECT_STREQ("System Event Log Entries", coll.getName().value().c_str());
    EXPECT_EQ(0, coll.getMembersOdataCount().value());

    EXPECT_FALSE(coll.getOem().hasValue());
    EXPECT_FALSE(coll.getOdataContext().hasValue());
    EXPECT_FALSE(coll.getOdataEtag().hasValue());

    auto& maybeMembers = coll.getMembers();
    EXPECT_TRUE(maybeMembers.hasValue());
    EXPECT_EQ(0, maybeMembers.value().size());
}

TEST(EventLogParserTests, WithEntriesTest)
{
    static std::string kEventlogEntryCollectionJson = R"(
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
    static constexpr int kExpectedEventCount = 2;
    auto coll = redfish_binding::LogEntryCollection::parseLogEntryCollection(
        kEventlogEntryCollectionJson);
    auto& maybeMembers = coll.getMembers();
    EXPECT_TRUE(maybeMembers.hasValue());
    EXPECT_EQ(kExpectedEventCount, coll.getMembersOdataCount().value());
    EXPECT_EQ(kExpectedEventCount, maybeMembers.value().size());

    // Validate entry at index 0.
    {
        auto& member = maybeMembers.value()[0];
        EXPECT_STREQ(
            "/redfish/v1/Systems/SomeBaseBoard/LogServices/EventLog/Entries/102",
            member.getOdataId().value().c_str());
        EXPECT_STREQ("#LogEntry.v1_15_0.LogEntry",
                     member.getOdataType().value().c_str());
        EXPECT_STREQ("2025-01-09T06:32:02+00:00",
                     member.getCreated().value().c_str());
        EXPECT_STREQ("2025-01-14T20:55:46+00:00",
                     member.getModified().value().c_str());
        EXPECT_EQ(redfish_binding::LogEntry::LogEntryType::Event,
                  member.getEntryType().value());
        EXPECT_STREQ("102", member.getId().value().c_str());
        EXPECT_STREQ("System Event Log Entry",
                     member.getName().value().c_str());
        EXPECT_STREQ("Some resolution.",
                     member.getResolution().value().c_str());
        EXPECT_FALSE(member.getResolved().value());
        EXPECT_EQ(redfish_binding::LogEntry::EventSeverity::Warning,
                  member.getSeverity().value());
        EXPECT_STREQ("Some message id 1.",
                     member.getMessageId().value().c_str());
        EXPECT_STREQ("Some message 1.", member.getMessage().value().c_str());
        EXPECT_EQ(2, member.getMessageArgs().value().size());
        EXPECT_STREQ("message args 0",
                     member.getMessageArgs().value()[0].c_str());
        EXPECT_STREQ("message args 1",
                     member.getMessageArgs().value()[1].c_str());
        EXPECT_FALSE(member.getOem().hasValue());
        EXPECT_FALSE(member.getCPER().hasValue());
    }

    // Validate entry at index 1.
    {
        auto& member = maybeMembers.value()[1];
        EXPECT_STREQ(
            "/redfish/v1/Systems/SomeBaseBoard/LogServices/EventLog/Entries/103",
            member.getOdataId().value().c_str());
        EXPECT_STREQ("#LogEntry.v1_15_0.LogEntry",
                     member.getOdataType().value().c_str());
        EXPECT_STREQ("2025-01-14T21:12:42+00:00",
                     member.getCreated().value().c_str());
        EXPECT_EQ(redfish_binding::LogEntry::LogEntryType::Event,
                  member.getEntryType().value());
        EXPECT_STREQ("103", member.getId().value().c_str());
        EXPECT_STREQ("Some message id 2.",
                     member.getMessageId().value().c_str());
        EXPECT_STREQ("Some message 2.", member.getMessage().value().c_str());
        EXPECT_FALSE(member.getMessageArgs().hasValue());
        EXPECT_STREQ("Some resolution 2.",
                     member.getResolution().value().c_str());
        EXPECT_TRUE(member.getResolved().value());
        EXPECT_EQ(redfish_binding::LogEntry::EventSeverity::Critical,
                  member.getSeverity().value());
        EXPECT_FALSE(member.getOem().hasValue());
        EXPECT_TRUE(member.getCPER().hasValue());
        auto& cperData = member.getCPER().value();
        EXPECT_STREQ("some notification type",
                     cperData.getNotificationType().value().c_str());
        EXPECT_STREQ("some section type",
                     cperData.getSectionType().value().c_str());
        EXPECT_TRUE(cperData.getOem().hasValue());
        nlohmann::json& oemJson = cperData.getOem().value().leftover();
        auto oemValue = oemJson["some_key"].get<std::string>();
        EXPECT_STREQ("some_value", oemValue.c_str());
    }
}
