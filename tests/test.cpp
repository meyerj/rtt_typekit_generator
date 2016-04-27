#include <gtest/gtest.h>
#include <rtt_typekit_generator/details/introspection.h>

#include "include/foo.h"

namespace rtt_typekit_generator {
namespace introspection {

extern details::TypeIntrospection<foo::NestedStruct> foo_types_foo_NestedStruct;
extern details::TypeIntrospection<foo::Simple> foo_types_foo_Simple;

}
}

using namespace rtt_typekit_generator::introspection;

TEST(StructTest, Basics) {
    EXPECT_EQ("/foo/NestedStruct", foo_types_foo_NestedStruct.getTypeName());
    EXPECT_EQ(&typeid(foo::NestedStruct), foo_types_foo_NestedStruct.getTypeId());
    EXPECT_STREQ(foo_types_foo_NestedStruct.getTypeIdName(), typeid(foo::NestedStruct).name());

    EXPECT_TRUE(foo_types_foo_NestedStruct.isStruct());
    EXPECT_TRUE(foo_types_foo_NestedStruct.getMembers());
}

TEST(StructTest, Members) {
    ASSERT_EQ(foo_types_foo_NestedStruct.getMembers()->size(), 4);
    ASSERT_TRUE(foo_types_foo_NestedStruct.getMember("x"));
    ASSERT_TRUE(foo_types_foo_NestedStruct.getMember("y"));
    ASSERT_TRUE(foo_types_foo_NestedStruct.getMember("z"));
    ASSERT_TRUE(foo_types_foo_NestedStruct.getMember("bar"));

//    foo::NestedStruct obj;
//    obj.x = 1.0;
//    obj.y = 2.0;
//    obj.z = 3.0;
//    obj.bar.a = "a";
//    obj.bar.b = "b";
//    obj.bar.c = "c";
//    EXPECT_EQ(1.0, foo_types_foo_NestedStruct.getMemberValue(&obj, "x").as<double>());
//    EXPECT_EQ(2.0, foo_types_foo_NestedStruct.getMemberValue(&obj, "y").as<double>());
//    EXPECT_EQ(3.0, foo_types_foo_NestedStruct.getMemberValue(&obj, "z").as<double>());
//    EXPECT_EQ("a", foo_types_foo_NestedStruct.getMemberValue(&obj, "bar").as<foo::NestedStruct::Bar>().a);
//    EXPECT_EQ("b", foo_types_foo_NestedStruct.getMemberValue(&obj, "bar").as<foo::NestedStruct::Bar>().b);
//    EXPECT_EQ("c", foo_types_foo_NestedStruct.getMemberValue(&obj, "bar").as<foo::NestedStruct::Bar>().c);
}

TEST(SimpleTest, Basics) {
    EXPECT_EQ("/foo/Simple", foo_types_foo_Simple.getTypeName());
    EXPECT_EQ(&typeid(foo::Simple), foo_types_foo_Simple.getTypeId());
    EXPECT_STREQ(foo_types_foo_Simple.getTypeIdName(), typeid(foo::Simple).name());

//    EXPECT_FALSE(foo_types_foo_Simple.isStruct());
//    EXPECT_FALSE(foo_types_foo_Simple.getMembers());
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
