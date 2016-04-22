/******************************************************************************
 *  Copyright (c) 2016, Intermodalics BVBA                                    *
 *  All rights reserved.                                                      *
 ******************************************************************************/

#ifndef RTT_TYPEKIT_GENERATOR_CORBA_TYPES_H
#define RTT_TYPEKIT_GENERATOR_CORBA_TYPES_H

#include <boost/type_traits/integral_constant.hpp>
#include <boost/preprocessor/seq/enum.hpp>

#include <string>

namespace rtt_typekit_generator {
namespace corba {

std::string demangle(const char *name);

template <typename T>
struct Type {
    typedef T type;
    typedef type value_type;
    typedef boost::false_type is_native;
    typedef boost::true_type is_struct;
    typedef boost::false_type is_sequence;

    static const char *getTypeName() { return "(unknown)"; }
    static const char *getCTypeName() { return ""; }
    static const char *getIDLTypeName() { return "any"; }
};

template <typename T>
static std::string accessor(T &, const std::string &value) {
    return value;
}
template <typename T>
static std::string accessor(T &, const std::string &value, const std::string &member_name) {
    return value + "." + member_name;
}

template <typename T>
struct ToCORBA {
    std::string operator()(const std::string &corba, const std::string &accessor) {
        return corba + " = " + accessor;
    }
    std::string operator()(const std::string &corba, const std::string &member_name, const std::string &accessor) {
        return corba + "." + member_name + " = " + accessor;
    }
};

template <typename T>
struct FromCORBA {
    std::string operator()(const std::string &accessor, const std::string &corba) {
        return accessor + " = " + corba;
    }
    std::string operator()(const std::string &accessor, const std::string &corba, const std::string &member_name) {
        return accessor + " = " + corba + "." + member_name;
    }
};

#define DEFINE_NATIVE_CORBA_TYPE(_type, _name, _idl_name) \
    template <> \
    struct Type< _type > { \
        typedef _type type; \
        typedef type value_type; \
        typedef boost::true_type is_native; \
        typedef boost::false_type is_struct; \
        typedef boost::false_type is_sequence; \
        static const char *getTypeName() { return _name; } \
        static const char *getCTypeName() { return #_type; } \
        static const char *getIDLTypeName() { return _idl_name; } \
    } \

#define DEFINE_NATIVE_CORBA_TYPE_TEMPLATE(_type, _name, _idl_name, \
                                          _template_parameter_list, \
                                          _template_parameter_arguments) \
    template <BOOST_PP_SEQ_ENUM(_template_parameter_list)> \
    struct Type< _type<BOOST_PP_SEQ_ENUM(_template_parameter_arguments)> > { \
        typedef _type<BOOST_PP_SEQ_ENUM(_template_parameter_arguments)> type; \
        typedef type value_type; \
        typedef boost::true_type is_native; \
        typedef boost::false_type is_struct; \
        typedef boost::false_type is_sequence; \
        static const char *getTypeName() { return _name; } \
        static const char *getCTypeName() { return #_type; } \
        static const char *getIDLTypeName() { return _idl_name; } \
    }

DEFINE_NATIVE_CORBA_TYPE(float, "float", "float");
DEFINE_NATIVE_CORBA_TYPE(double, "double", "double");
DEFINE_NATIVE_CORBA_TYPE(long double, "long double", "long double");
DEFINE_NATIVE_CORBA_TYPE(signed short int, "short", "short");
DEFINE_NATIVE_CORBA_TYPE(signed long int, "long", "long");
DEFINE_NATIVE_CORBA_TYPE(signed long long int, "long long", "long long");
DEFINE_NATIVE_CORBA_TYPE(unsigned short int, "unsigned short", "unsigned short");
DEFINE_NATIVE_CORBA_TYPE(unsigned long int, "unsigned long", "unsigned long");
DEFINE_NATIVE_CORBA_TYPE(unsigned long long int, "unsigned long long", "unsigned long long");
DEFINE_NATIVE_CORBA_TYPE(char, "char", "char");
DEFINE_NATIVE_CORBA_TYPE(wchar_t, "wchar", "wchar");
DEFINE_NATIVE_CORBA_TYPE(bool, "bool", "boolean");
DEFINE_NATIVE_CORBA_TYPE(unsigned char, "unsigned char", "octet");
DEFINE_NATIVE_CORBA_TYPE_TEMPLATE(std::basic_string, "string", "string", (class Traits)(class Alloc), (char)(Traits)(Alloc));
DEFINE_NATIVE_CORBA_TYPE_TEMPLATE(std::basic_string, "wstring", "wstring", (class Traits)(class Alloc), (wchar_t)(Traits)(Alloc));

// Sequence types
template <typename T>
struct Type< std::vector<T> > {
    typedef std::vector<T> type;
    typedef T value_type;
    typedef boost::false_type is_native;
    typedef boost::false_type is_struct;
    typedef boost::true_type is_sequence;

    static const char *getTypeName() { Type<T>::getTypeName(); }
    static const char *getCTypeName() { Type<T>::getCTypeName(); }
    static const char *getIDLTypeName() { Type<T>::getIDLTypeName(); }
};

// String types
template <typename CharT, class Traits, class Alloc>
struct ToCORBA< std::basic_string<CharT, Traits, Alloc> > {
    std::string operator()(const std::string &corba, const std::string &accessor) {
        return corba + " = " + accessor + ".c_str()";
    }
    std::string operator()(const std::string &corba, const std::string &member_name, const std::string &accessor) {
        return corba + "." + member_name + " = " + accessor + ".c_str()";
    }
};

}  // namespace corba
}  // namespace rtt_typekit_generator

#endif // RTT_TYPEKIT_GENERATOR_CORBA_TYPES_H
