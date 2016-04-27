/******************************************************************************
 *  Copyright (c) 2016, Intermodalics BVBA                                    *
 *  All rights reserved.                                                      *
 ******************************************************************************/

#ifndef RTT_TYPEKIT_GENERATOR_TESTS_INCLUDE_FOO_H
#define RTT_TYPEKIT_GENERATOR_TESTS_INCLUDE_FOO_H

#include <iostream>
#include <string>
#include <vector>

#include <Eigen/Core>
#include <kdl_typekit/typekit/Types.hpp>  // for boost::serialization only

namespace foo {

struct Simple {
    std::string member;
};

struct Vector {
    std::vector<std::string> strings;
};

struct NestedStruct {
    double x;
    double y;
    double z;

    struct Bar {
        std::string a;
        std::string b;
        std::string c;
    } bar;
};

typedef Eigen::Vector3d EigenVector;

struct NestedKDL {
    KDL::Vector v;
    std::vector<KDL::Frame> frames;
};

static inline std::ostream &operator<<(std::ostream &os, const Simple &t) {
    return os << "{ \"" << t.member << "\"}";
}

static inline std::istream &operator>>(std::istream &is, Simple &t) {
    return is >> t.member;
}

}  // namespace foo


#include <boost/serialization/nvp.hpp>

namespace boost {
namespace serialization {

template <class Archive>
void serialize(Archive& ar, foo::Simple& v, const unsigned int /* version */) {
    ar & v.member;
//    ar & boost::serialization::make_nvp("member", v.member);
}

template <class Archive>
void serialize(Archive& ar, foo::Vector& v, const unsigned int /* version */) {
    ar & boost::serialization::make_nvp("strings", v.strings);
}

template <class Archive>
void serialize(Archive& ar, foo::NestedStruct::Bar& v, const unsigned int /* version */) {
    ar & boost::serialization::make_nvp("a", v.a);
    ar & boost::serialization::make_nvp("b", v.b);
    ar & boost::serialization::make_nvp("c", v.c);
}

template <class Archive>
void serialize(Archive& ar, foo::NestedStruct& v, const unsigned int /* version */) {
    ar & boost::serialization::make_nvp("x", v.x);
    ar & boost::serialization::make_nvp("y", v.y);
    ar & boost::serialization::make_nvp("z", v.z);
    ar & boost::serialization::make_nvp("bar", v.bar);
}

template <class Archive>
void serialize(Archive& ar, foo::EigenVector& v, const unsigned int /* version */) {
    ar & boost::serialization::make_nvp("x", v.x());
    ar & boost::serialization::make_nvp("y", v.y());
    ar & boost::serialization::make_nvp("z", v.z());
}

template <class Archive>
void serialize(Archive& ar, foo::NestedKDL& v, const unsigned int /* version */) {
    ar & boost::serialization::make_nvp("v", v.v);
    ar & boost::serialization::make_nvp("frames", v.frames);
}

}  // namespace serialization
}  // namespace boost

namespace rtt_typekit_generator {
namespace corba {

static std::string accessor(foo::EigenVector &, const std::string &value, const std::string &member) {
    return value + "." + member + "()";
}

static std::string accessor(KDL::Vector &, const std::string &value, const std::string &member) {
    if (member == "X") return value + "[0]";
    if (member == "Y") return value + "[1]";
    if (member == "Z") return value + "[2]";
    throw std::runtime_error("Unexpected member '" + member + "' of type KDL::Vector");
}

static std::string accessor(KDL::Rotation &, const std::string &value, const std::string &member) {
    if (member == "X_x") return value + ".data[0]";
    if (member == "X_y") return value + ".data[3]";
    if (member == "X_z") return value + ".data[6]";
    if (member == "Y_x") return value + ".data[1]";
    if (member == "Y_y") return value + ".data[4]";
    if (member == "Y_z") return value + ".data[7]";
    if (member == "Z_x") return value + ".data[2]";
    if (member == "Z_y") return value + ".data[5]";
    if (member == "Z_z") return value + ".data[8]";
    throw std::runtime_error("Unexpected member '" + member + "' of type KDL::Vector");
}

}  // corba
}  // rtt_typekit_generator

#endif // RTT_TYPEKIT_GENERATOR_TESTS_INCLUDE_FOO_H
