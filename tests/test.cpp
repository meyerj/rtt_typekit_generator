#include <gtest/gtest.h>
#include <rtt_typekit_generator/details/introspection.h>

#include "include/foo.h"

namespace rtt_typekit_generator {

extern details::TypeIntrospection<foo::NestedStruct> type0;
extern details::TypeIntrospection<foo::Simple> type1;

}

using namespace rtt_typekit_generator;

TEST(StructTest, Basics) {
    EXPECT_EQ(type0.getTypeName(), "foo.NestedStruct");
    EXPECT_EQ(type0.getTypeId(), &typeid(foo::NestedStruct));
    EXPECT_STREQ(type0.getTypeIdName(), typeid(foo::NestedStruct).name());

    EXPECT_TRUE(type0.isStruct());
    EXPECT_TRUE(type0.getMembers());
}

TEST(StructTest, Members) {
    ASSERT_EQ(type0.getMembers()->size(), 3);
    ASSERT_TRUE(type0.getMember("x"));
    ASSERT_TRUE(type0.getMember("y"));
    ASSERT_TRUE(type0.getMember("z"));

    foo::NestedStruct obj;
    obj.x = 1.0;
    obj.y = 2.0;
    obj.z = 3.0;
    EXPECT_EQ(type0.getMemberValue(&obj, "x").as<double>(), 1.0);
    EXPECT_EQ(type0.getMemberValue(&obj, "y").as<double>(), 2.0);
    EXPECT_EQ(type0.getMemberValue(&obj, "z").as<double>(), 3.0);
}

TEST(SimpleTest, Basics) {
    EXPECT_EQ(type1.getTypeName(), "foo.Simple");
    EXPECT_EQ(type1.getTypeId(), &typeid(foo::Simple));
    EXPECT_STREQ(type1.getTypeIdName(), typeid(foo::Simple).name());

    EXPECT_FALSE(type1.isStruct());
    EXPECT_FALSE(type1.getMembers());
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
