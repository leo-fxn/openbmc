#pragma once

#include <nlohmann/json.hpp>

#include <iostream>
#include <string>
#include <variant>

namespace redfishlib
{

class IProperty
{
  public:
    IProperty() = default;

    virtual bool setValue(const nlohmann::json& json) = 0;

    virtual std::string name() const = 0;

    virtual ~IProperty() = default;

    IProperty(const IProperty&) = default;

    IProperty(IProperty&&) = default;

    IProperty& operator=(const IProperty&) = default;

    IProperty& operator=(IProperty&&) = default;
};

template <typename T>
class Property : public IProperty
{
  public:
    explicit Property(const std::string& name) : name_(name) {}

    Property() = delete;

    T& value()
    {
        return *value_;
    }

    bool hasValue() const
    {
        return value_ != nullptr;
    }

    bool setValue(const nlohmann::json& json) override
    {
        if (json.is_null())
        {
            value_.reset();
            return true;
        }
        try
        {
            value_ = std::make_unique<T>(json.template get<T>());
            return true;
        }
        catch (const std::exception& ex)
        {
            // change it to log
            std::cout << name() << ":" << json.dump() << " - " << ex.what()
                      << std::endl;
            return false;
        }
    }

    std::string name() const override
    {
        return name_;
    }

  private:
    std::unique_ptr<T> value_;
    std::string name_;
};

template <typename... Args>
struct variant_cast_proxy
{
    std::variant<Args...> v;

    template <typename... ToArgs>
    operator std::variant<ToArgs...>() const
    {
        return std::visit(
            [](auto&& arg) -> std::variant<ToArgs...> { return arg; }, v);
    }
};

template <typename... Args>
auto variant_cast(const std::variant<Args...>& v) -> variant_cast_proxy<Args...>
{
    return {v};
}

class JsonDeserializable
{
  public:
    JsonDeserializable() = default;

    virtual void fromJson(const nlohmann::json& json) = 0;

    virtual ~JsonDeserializable() = default;

    JsonDeserializable(const JsonDeserializable&) = default;

    JsonDeserializable(JsonDeserializable&&) = default;

    JsonDeserializable& operator=(const JsonDeserializable&) = default;

    JsonDeserializable& operator=(JsonDeserializable&&) = default;
};

template <typename T, std::enable_if_t<std::is_base_of_v<JsonDeserializable, T>,
                                       bool> = true>
void from_json(const nlohmann::json& json, T& deserializable)
{
    deserializable.fromJson(json);
}

class ResourceBase : public JsonDeserializable
{
  public:
    ResourceBase() = default;

    void fromJson(const nlohmann::json& json) override
    {
        for (const auto& [key, value] : json.items())
        {
            if (auto it = properties_.find(key);
                it == properties_.end() || !it->second->setValue(value))
            {
                leftover_[key] = value;
            }
        }
    }

    nlohmann::json& leftover()
    {
        return leftover_;
    }

  protected:
    void registerProperty(IProperty* property)
    {
        properties_.emplace(property->name(), property);
    }

  private:
    std::unordered_map<std::string, IProperty*> properties_{};
    nlohmann::json leftover_{};
};

class Error : public ResourceBase
{
  public:
    Error()
    {
        registerProperty(&code_);
        registerProperty(&message_);
    }

    Property<std::string>& code()
    {
        return code_;
    }

    Property<std::string>& message()
    {
        return message_;
    }

  private:
    Property<std::string> code_{"code"};
    Property<std::string> message_{"message"};
};

class ResourceBaseWithError : public ResourceBase
{
  public:
    ResourceBaseWithError()
    {
        registerProperty(&error_);
    }

    Property<Error>& error()
    {
        return error_;
    }

  private:
    Property<Error> error_{"error"};
};

} // namespace redfishlib

namespace nlohmann
{
template <typename T, typename... Ts>
struct adl_serializer<std::variant<T, Ts...>>
{
    static void from_json(const nlohmann::json& json,
                          std::variant<T, Ts...>& value)
    {
        try
        {
            value = json.template get<T>();
        }
        catch (const std::exception&)
        {
            if constexpr (sizeof...(Ts) == 0)
            {
                throw;
            }
            else
            {
                value = redfishlib::variant_cast(
                    json.template get<std::variant<Ts...>>());
            }
        }
    }
};
} // namespace nlohmann
