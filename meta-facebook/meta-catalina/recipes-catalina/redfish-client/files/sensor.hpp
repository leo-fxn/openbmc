#pragma once

#include <optional>
#include <string>

namespace redfish_client_daemon
{

class Sensor
{
  public:
    static std::optional<Sensor> parseSensor(const std::string& sensorJson);

    double getReading()
    {
        return reading;
    }

    const std::string& getSensorUnitText()
    {
        return sensorUnit;
    }

    std::optional<double> getMinValue()
    {
        return minValue;
    }

    std::optional<double> getMaxValue()
    {
        return maxValue;
    }

    // Expose a "constructor" for testing purposes.
    static Sensor createTestSensor(double reading, const char* sensorUnit,
                                   std::optional<double> minValue,
                                   std::optional<double> maxValue)
    {
        Sensor rv;
        rv.reading = reading;
        rv.sensorUnit = sensorUnit;
        rv.minValue = minValue;
        rv.maxValue = maxValue;
        return rv;
    }

  private:
    double reading;
    std::string sensorUnit;
    std::optional<double> minValue;
    std::optional<double> maxValue;
};

} // namespace redfish_client_daemon
