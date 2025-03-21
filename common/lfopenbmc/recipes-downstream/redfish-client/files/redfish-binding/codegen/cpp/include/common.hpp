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

    virtual nlohmann::json toJson() const = 0;

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

    nlohmann::json toJson() const override
    {
        nlohmann::json json({});
        if (hasValue())
        {
            json[name_] = *value_;
        }
        return json;
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

class ResourceBase
{
  public:
    ResourceBase() = default;

    ResourceBase(const ResourceBase&) = default;

    ResourceBase(ResourceBase&&) = default;

    ResourceBase& operator=(const ResourceBase&) = default;

    ResourceBase& operator=(ResourceBase&&) = default;

    virtual ~ResourceBase() = default;

    void fromJson(const nlohmann::json& json)
    {
        for (const auto& [key, value] : json.items())
        {
            if (auto property = findProperty(key);
                property == nullptr || !property->setValue(value))
            {
                leftover_[key] = value;
            }
        }
    }

    nlohmann::json toJson() const
    {
        nlohmann::json json(leftover_);
        forEachProperty([&](const IProperty* property) {
            json.update(property->toJson());
        });
        return json;
    }

    nlohmann::json& leftover()
    {
        return leftover_;
    }

  protected:
    virtual IProperty* findProperty(const std::string&)
    {
        return nullptr;
    }

    virtual void
        forEachProperty(const std::function<void(const IProperty*)>&) const
    {}

  private:
    nlohmann::json leftover_ = nlohmann::json({});
};

template <typename T,
          std::enable_if_t<std::is_base_of_v<ResourceBase, T>, bool> = true>
void from_json(const nlohmann::json& json, T& resource)
{
    resource.fromJson(json);
}

template <typename T,
          std::enable_if_t<std::is_base_of_v<ResourceBase, T>, bool> = true>
void to_json(nlohmann::json& json, const T& resource)
{
    json = resource.toJson();
}

class Error : public ResourceBase
{
  public:
    Property<std::string>& code()
    {
        return code_;
    }

    Property<std::string>& message()
    {
        return message_;
    }

  protected:
    IProperty* findProperty(const std::string& name) override
    {
        if (name == code_.name())
        {
            return &code_;
        }
        if (name == message_.name())
        {
            return &message_;
        }
        return ResourceBase::findProperty(name);
    }

    void forEachProperty(
        const std::function<void(const IProperty*)>& fn) const override
    {
        fn(&code_);
        fn(&message_);
        ResourceBase::forEachProperty(fn);
    }

  private:
    Property<std::string> code_{"code"};
    Property<std::string> message_{"message"};
};

class ResourceBaseWithError : public ResourceBase
{
  public:
    Property<Error>& error()
    {
        return error_;
    }

  protected:
    IProperty* findProperty(const std::string& name) override
    {
        if (name == error_.name())
        {
            return &error_;
        }
        return ResourceBase::findProperty(name);
    }

    void forEachProperty(
        const std::function<void(const IProperty*)>& fn) const override
    {
        fn(&error_);
        ResourceBase::forEachProperty(fn);
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

    static void to_json(nlohmann::json& json,
                        const std::variant<T, Ts...>& value)
    {
        std::visit([&](auto&& arg) { json = arg; }, value);
    }
};
} // namespace nlohmann
