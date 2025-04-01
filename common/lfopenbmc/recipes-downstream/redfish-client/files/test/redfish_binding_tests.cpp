#include "redfish-binding/Sensor_Sensor.hpp"

#include <gtest/gtest.h>

TEST(RedfishBindingTest, ParseSensorTest)
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
    auto sensor = redfish_binding::Sensor::parseSensor(sensorJson);
    EXPECT_EQ(sensor.getReading().value(), 1.5);
    EXPECT_EQ(sensor.getReadingUnits().value(), "Cel");
    EXPECT_EQ(sensor.getName().value(), "Test_Sensor");
    EXPECT_EQ(sensor.getStatus().value().getState().value(),
              redfish_binding::Resource::State::Enabled);
}
