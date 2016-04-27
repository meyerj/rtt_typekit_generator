# Generate typekit foo-types for type 'foo::Foo' in header 'include/foo.h'

include_directories(include)

find_package(Eigen3 REQUIRED)
set(EIGEN_INCLUDE_DIRS ${EIGEN3_INCLUDE_DIR})
include_directories(${EIGEN_INCLUDE_DIRS})

orocos_find_package(kdl_typekit REQUIRED)
include_directories(${kdl_typekit_INCLUDE_DIRS})

orocos_typekit_headers(
  foo-types
  HEADERS foo.h
  TYPES foo::Simple foo::Vector foo::NestedStruct foo::NestedStruct::Bar foo::EigenVector foo::NestedKDL
)

enable_testing()

# Always build tests in the build directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_executable(foo-types-test test.cpp)
target_link_libraries(foo-types-test foo-types_introspection gtest gtest_main)

add_test(NAME run-foo-types-test COMMAND ${CMAKE_CURRENT_BINARY_DIR}/foo-types-test)
