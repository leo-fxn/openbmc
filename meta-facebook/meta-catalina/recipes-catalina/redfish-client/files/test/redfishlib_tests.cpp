#include "Sensor_Sensor.hpp"

#include <gtest/gtest.h>

TEST(RedfishLibTest, ParseSensorTest)
{
    std::string sensorJson = R"(
  {
    "Reading": 1.5,
    "ReadingUnits": "Cel",
    "Name": "Test_Sensor",
    "Status": {
        "State": "Enabled"
    }
  }
)";
    auto sensor = redfishlib::Sensor::parseSensor(sensorJson);
    EXPECT_EQ(sensor.getReading().value(), 1.5);
    EXPECT_EQ(sensor.getReadingUnits().value(), "Cel");
    EXPECT_EQ(sensor.getName().value(), "Test_Sensor");
    EXPECT_EQ(sensor.getStatus().value().getState().value().value(),
              redfishlib::Resource::State::Enum::Enabled);
}
