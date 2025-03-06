#include "sensor.hpp"

#include "Sensor_Sensor.hpp"

namespace redfish_client_daemon
{

namespace
{

std::optional<double> toMaybeDouble(redfishlib::Property<double>& property)
{
    if (property.hasValue())
    {
        return property.value();
    }
    return std::nullopt;
}

} // namespace

std::optional<Sensor> Sensor::parseSensor(const std::string& sensorJson)
{
    auto parsed = redfishlib::Sensor::parseSensor(sensorJson);

    auto maybeReading = toMaybeDouble(parsed.getReading());
    if (!maybeReading.has_value())
    {
        return std::nullopt;
    }

    if (!parsed.getReadingUnits().hasValue())
    {
        return std::nullopt;
    }

    Sensor sensor;
    sensor.reading = maybeReading.value();
    sensor.sensorUnit = parsed.getReadingUnits().value();
    sensor.minValue = toMaybeDouble(parsed.getReadingRangeMin());
    sensor.maxValue = toMaybeDouble(parsed.getReadingRangeMax());
    return sensor;
}

} // namespace redfish_client_daemon
