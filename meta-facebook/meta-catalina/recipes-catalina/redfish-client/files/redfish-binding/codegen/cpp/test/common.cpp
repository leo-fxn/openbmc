#include "common.hpp"

#include <gtest/gtest.h>

namespace redfishlib
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

TEST(PropertyTest, PrimitiveTypeTest)
{
    Property<int> intProperty{"int"};
    EXPECT_EQ(intProperty.hasValue(), false);
    intProperty.setValue(nlohmann::json(25));
    EXPECT_EQ(intProperty.hasValue(), true);
    EXPECT_EQ(intProperty.value(), 25);
    intProperty.setValue(nlohmann::json(nullptr));
    EXPECT_EQ(intProperty.hasValue(), false);

    Property<double> doubleProperty{"double"};
    EXPECT_EQ(doubleProperty.hasValue(), false);
    doubleProperty.setValue(nlohmann::json(2.01));
    EXPECT_EQ(doubleProperty.hasValue(), true);
    EXPECT_EQ(doubleProperty.value(), 2.01);

    Property<bool> boolProperty{"bool"};
    EXPECT_EQ(boolProperty.hasValue(), false);
    boolProperty.setValue(nlohmann::json(false));
    EXPECT_EQ(boolProperty.hasValue(), true);
    EXPECT_EQ(boolProperty.value(), false);

    Property<std::string> stringProperty{"string"};
    EXPECT_EQ(stringProperty.hasValue(), false);
    stringProperty.setValue(nlohmann::json("test"));
    EXPECT_EQ(stringProperty.hasValue(), true);
    EXPECT_EQ(stringProperty.value(), "test");
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
    objectProperty.setValue(nlohmann::json(nullptr));
    EXPECT_EQ(objectProperty.hasValue(), false);
}

TEST(PropertyTest, ContainerTypeTest)
{
    std::vector<int> intVector{10, 1, 3};
    Property<std::vector<int>> intVectorProperty{"intVector"};
    EXPECT_EQ(intVectorProperty.hasValue(), false);
    intVectorProperty.setValue(nlohmann::json(intVector));
    EXPECT_EQ(intVectorProperty.hasValue(), true);
    EXPECT_EQ(intVectorProperty.value(), intVector);

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

    variantProperty.setValue(nlohmann::json(5));
    EXPECT_EQ(variantProperty.hasValue(), true);
    EXPECT_EQ(std::get<int>(variantProperty.value()), 5);

    variantProperty.setValue(nlohmann::json("test"));
    EXPECT_EQ(variantProperty.hasValue(), true);
    EXPECT_EQ(std::get<std::string>(variantProperty.value()), "test");

    variantProperty.setValue(nlohmann::json(object));
    EXPECT_EQ(variantProperty.hasValue(), true);
    EXPECT_EQ(std::get<DummyObject>(variantProperty.value()), object);

    variantProperty.setValue(nlohmann::json(nullptr));
    EXPECT_EQ(variantProperty.hasValue(), false);
}

class DummyResource : public ResourceBaseWithError
{
  public:
    DummyResource()
    {
        registerProperty(&p1_);
        registerProperty(&p2_);
        registerProperty(&p3_);
    }

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

  private:
    Property<int> p1_{"p1"};
    Property<std::string> p2_{"p2"};
    Property<std::variant<double, bool>> p3_{"p3"};
};

TEST(ResourceBaseTest, ParseTest)
{
    nlohmann::json json;
    json["p1"] = 5;
    json["p2"] = nullptr;
    json["error"] = {
        {"code", "404"},
    };
    json["unknown"] = "unknown";
    auto resource = json.template get<DummyResource>();
    EXPECT_EQ(resource.p1().hasValue(), true);
    EXPECT_EQ(resource.p1().value(), 5);
    EXPECT_EQ(resource.p2().hasValue(), false);
    EXPECT_EQ(resource.p3().hasValue(), false);
    EXPECT_EQ(resource.error().hasValue(), true);
    EXPECT_EQ(resource.error().value().code().hasValue(), true);
    EXPECT_EQ(resource.error().value().code().value(), "404");
    EXPECT_EQ(resource.error().value().message().hasValue(), false);
    EXPECT_EQ(resource.error().value().leftover(), nlohmann::json());
    EXPECT_EQ(resource.leftover(), nlohmann::json({{"unknown", "unknown"}}));
}

} // namespace redfishlib
