#include "redfish-binding/common.hpp"

#include <gtest/gtest.h>

namespace redfish_binding
{

namespace
{

struct DummyObject
{
    std::string field1;
    bool field2;
    int field3;

    friend bool operator==(const DummyObject& lhs, const DummyObject& rhs)
    {
        return lhs.field1 == rhs.field1 && lhs.field2 == rhs.field2 &&
               lhs.field3 == rhs.field3;
    }
};

void from_json(const nlohmann::json& json, DummyObject& object)
{
    json.at("field1").get_to(object.field1);
    json.at("field2").get_to(object.field2);
    json.at("field3").get_to(object.field3);
}

void to_json(nlohmann::json& json, const DummyObject& object)
{
    json = nlohmann::json{
        {"field1", object.field1},
        {"field2", object.field2},
        {"field3", object.field3},
    };
}

class DummyResource : public ResourceBaseWithError
{
  public:
    Property<int>& p1()
    {
        return p1_;
    }

    Property<std::string>& p2()
    {
        return p2_;
    }

    Property<std::variant<double, bool>>& p3()
    {
        return p3_;
    }

  protected:
    IProperty* findProperty(const std::string& key) override
    {
        if (key == p1_.key())
        {
            return &p1_;
        }
        if (key == p2_.key())
        {
            return &p2_;
        }
        if (key == p3_.key())
        {
            return &p3_;
        }
        return ResourceBaseWithError::findProperty(key);
    }

    void forEachProperty(
        const std::function<void(const IProperty*)>& fn) const override
    {
        fn(&p1_);
        fn(&p2_);
        fn(&p3_);
        ResourceBaseWithError::forEachProperty(fn);
    }

  private:
    Property<int> p1_{"p1"};
    Property<std::string> p2_{"p2"};
    Property<std::variant<double, bool>> p3_{"p3"};
};

enum class DummyEnum
{
    A,
    B,
    C,
};
NLOHMANN_JSON_SERIALIZE_ENUM(DummyEnum, {
                                            {DummyEnum::A, "a"},
                                            {DummyEnum::B, "b"},
                                            {DummyEnum::C, "c"},
                                        })

} // namespace

TEST(PropertyTest, PrimitiveTypeTest)
{
    Property<int> intProperty{"int"};
    EXPECT_EQ(intProperty.hasValue(), false);
    intProperty.setValue(nlohmann::json(25));
    EXPECT_EQ(intProperty.hasValue(), true);
    EXPECT_EQ(intProperty.value(), 25);
    EXPECT_EQ(intProperty.toJson(), nlohmann::json({{"int", 25}}));
    intProperty.setValue(nlohmann::json(nullptr));
    EXPECT_EQ(intProperty.hasValue(), false);
    EXPECT_EQ(intProperty.toJson(), nlohmann::json({}));

    Property<double> doubleProperty{"double"};
    EXPECT_EQ(doubleProperty.hasValue(), false);
    doubleProperty.setValue(nlohmann::json(2.01));
    EXPECT_EQ(doubleProperty.hasValue(), true);
    EXPECT_EQ(doubleProperty.value(), 2.01);
    EXPECT_EQ(doubleProperty.toJson(), nlohmann::json({{"double", 2.01}}));

    Property<bool> boolProperty{"bool"};
    EXPECT_EQ(boolProperty.hasValue(), false);
    boolProperty.setValue(nlohmann::json(false));
    EXPECT_EQ(boolProperty.hasValue(), true);
    EXPECT_EQ(boolProperty.value(), false);
    EXPECT_EQ(boolProperty.toJson(), nlohmann::json({{"bool", false}}));

    Property<std::string> stringProperty{"string"};
    EXPECT_EQ(stringProperty.hasValue(), false);
    stringProperty.setValue(nlohmann::json("test"));
    EXPECT_EQ(stringProperty.hasValue(), true);
    EXPECT_EQ(stringProperty.value(), "test");
    EXPECT_EQ(stringProperty.toJson(), nlohmann::json({{"string", "test"}}));
}

TEST(PropertyTest, ObjectTypeTest)
{
    DummyObject object{
        .field1 = "test",
        .field2 = true,
        .field3 = 10,
    };
    Property<DummyObject> objectProperty{"object"};
    EXPECT_EQ(objectProperty.hasValue(), false);
    objectProperty.setValue(nlohmann::json(object));
    EXPECT_EQ(objectProperty.hasValue(), true);
    EXPECT_EQ(objectProperty.value(), object);
    EXPECT_EQ(objectProperty.toJson(), nlohmann::json({{"object", object}}));
    objectProperty.setValue(nlohmann::json(nullptr));
    EXPECT_EQ(objectProperty.hasValue(), false);
    EXPECT_EQ(objectProperty.toJson(), nlohmann::json({}));
}

TEST(PropertyTest, ContainerTypeTest)
{
    std::vector<int> intVector{10, 1, 3};
    Property<std::vector<int>> intVectorProperty{"intVector"};
    EXPECT_EQ(intVectorProperty.hasValue(), false);
    intVectorProperty.setValue(nlohmann::json(intVector));
    EXPECT_EQ(intVectorProperty.hasValue(), true);
    EXPECT_EQ(intVectorProperty.value(), intVector);
    EXPECT_EQ(intVectorProperty.toJson(),
              nlohmann::json({{"intVector", intVector}}));

    std::vector<DummyObject> objectVector{
        {
            .field1 = "test1",
            .field2 = true,
            .field3 = 101,
        },
        {
            .field1 = "test2",
            .field2 = true,
            .field3 = 103,
        },
        {
            .field1 = "test3",
            .field2 = false,
            .field3 = 55,
        },
    };
    Property<std::vector<DummyObject>> objectVectorProperty{"objectVector"};
    EXPECT_EQ(objectVectorProperty.hasValue(), false);
    objectVectorProperty.setValue(nlohmann::json(objectVector));
    EXPECT_EQ(objectVectorProperty.hasValue(), true);
    EXPECT_EQ(objectVectorProperty.value(), objectVector);
    EXPECT_EQ(objectVectorProperty.toJson(),
              nlohmann::json({{"objectVector", objectVector}}));
}

TEST(PropertyTest, VariantTypeTest)
{
    DummyObject object{
        .field1 = "test",
        .field2 = false,
        .field3 = 1022,
    };
    Property<std::variant<int, std::string, DummyObject>> variantProperty{
        "variant"};
    EXPECT_EQ(variantProperty.hasValue(), false);
    EXPECT_EQ(variantProperty.toJson(), nlohmann::json({}));

    variantProperty.setValue(nlohmann::json(5));
    EXPECT_EQ(variantProperty.hasValue(), true);
    EXPECT_EQ(std::get<int>(variantProperty.value()), 5);
    EXPECT_EQ(variantProperty.toJson(), nlohmann::json({{"variant", 5}}));

    variantProperty.setValue(nlohmann::json("test"));
    EXPECT_EQ(variantProperty.hasValue(), true);
    EXPECT_EQ(std::get<std::string>(variantProperty.value()), "test");
    EXPECT_EQ(variantProperty.toJson(), nlohmann::json({{"variant", "test"}}));

    variantProperty.setValue(nlohmann::json(object));
    EXPECT_EQ(variantProperty.hasValue(), true);
    EXPECT_EQ(std::get<DummyObject>(variantProperty.value()), object);
    EXPECT_EQ(variantProperty.toJson(), nlohmann::json({{"variant", object}}));

    variantProperty.setValue(nlohmann::json(nullptr));
    EXPECT_EQ(variantProperty.hasValue(), false);
    EXPECT_EQ(variantProperty.toJson(), nlohmann::json({}));
}

TEST(PropertyTest, EnumTypeTest)
{
    Property<DummyEnum> enumProperty{"enum"};
    EXPECT_EQ(enumProperty.hasValue(), false);
    enumProperty.setValue(nlohmann::json("a"));
    EXPECT_EQ(enumProperty.hasValue(), true);
    EXPECT_EQ(enumProperty.value(), DummyEnum::A);
    EXPECT_EQ(enumProperty.toJson(), nlohmann::json({{"enum", "a"}}));
    enumProperty.setValue(nlohmann::json("b"));
    EXPECT_EQ(enumProperty.hasValue(), true);
    EXPECT_EQ(enumProperty.value(), DummyEnum::B);
    EXPECT_EQ(enumProperty.toJson(), nlohmann::json({{"enum", "b"}}));
}

TEST(ResourceBaseTest, ResourceBaseTest)
{
    nlohmann::json json;
    json["p1"] = 5;
    json["p2"] = nullptr;
    json["error"] = {
        {"code", "404"},
    };
    json["p3"] = 0.5;
    json["unknown"] = "unknown";
    auto resource = json.template get<DummyResource>();
    EXPECT_EQ(resource.p1().hasValue(), true);
    EXPECT_EQ(resource.p1().value(), 5);
    EXPECT_EQ(resource.p2().hasValue(), false);
    EXPECT_EQ(resource.p3().hasValue(), true);
    EXPECT_EQ(std::get<double>(resource.p3().value()), 0.5);
    EXPECT_EQ(resource.getError().hasValue(), true);
    EXPECT_EQ(resource.getError().value().getCode().hasValue(), true);
    EXPECT_EQ(resource.getError().value().getCode().value(), "404");
    EXPECT_EQ(resource.getError().value().getMessage().hasValue(), false);
    EXPECT_EQ(resource.getError().value().leftover(), nlohmann::json({}));
    EXPECT_EQ(resource.leftover(), nlohmann::json({{"unknown", "unknown"}}));
    // toJson() will drop all keys with nullptr as value
    json.erase("p2");
    EXPECT_EQ(resource.toJson(), json);
}

} // namespace redfish_binding
