/******************************************************************************
 *  Copyright (c) 2016, Intermodalics BVBA                                    *
 *  All rights reserved.                                                      *
 ******************************************************************************/

#ifndef RTT_TYPEKIT_GENERATOR_CORBA_TRANSPORTS_CORBA_CORBA_TYPES_H
#define RTT_TYPEKIT_GENERATOR_CORBA_TRANSPORTS_CORBA_CORBA_TYPES_H

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
    } \

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

// Specialized Type<T> for sequence types
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

// accessor template function
template <typename T>
static std::string accessor(T &, const std::string &value, const std::string &member = std::string()) {
    if (member.empty()) {
        return value;
    } else {
        return value + "." + member;
    }
}

// ToCORBA<T> generator template
template <typename T>
struct ToCORBA {
    std::string operator()(const std::string &accessor, const std::string &corba,
                           const std::string &member = std::string(),
                           const std::string &index = std::string()) {
        std::string index_suffix;
        if (!index.empty()) index_suffix = "[" + index + "]";
        if (!member.empty()) {
            return "if (!toCORBA(" + corba + "." + member + index_suffix + ", " + accessor + index_suffix + ")) return false;";
        } else {
            return "if (!toCORBA(" + corba + index_suffix + ", " + accessor + index_suffix + ")) return false;";
        }
    }
};

// toCORBA() template function
template <typename T, typename CorbaType>
inline bool toCORBA(CorbaType &corba, const T &value) {
    corba = value;
    return true;
}

// Overload toCorba() function for string types
template <typename CharT, class Traits, class Alloc, typename CorbaType>
bool toCORBA(CorbaType &corba, const std::basic_string<CharT, Traits, Alloc> &value) {
    corba = value.c_str();
    return true;
}

// Overload toCorba() function for string types and constant corba objects
template <typename CharT, class Traits, class Alloc, typename CorbaType>
bool toCORBA(const CorbaType &corba, const std::basic_string<CharT, Traits, Alloc> &value) {
    CorbaType reference(corba);
    reference = value.c_str();
    return true;
}

// FromCORBA generator template
template <typename T>
struct FromCORBA {
    std::string operator()(const std::string &accessor, const std::string &corba,
                           const std::string &member = std::string(),
                           const std::string &index = std::string()) {
        std::string index_suffix;
        if (!index.empty()) index_suffix = "[" + index + "]";
        if (!member.empty()) {
            return "if (!fromCORBA(" + accessor + index_suffix + ", " + corba + "." + member + index_suffix + ")) return false;";
        } else {
            return "if (!fromCORBA(" + accessor + index_suffix + ", " + corba + index_suffix + ")) return false;";
        }
    }
};

// fromCORBA() template function
template <typename T, typename CorbaType>
inline bool fromCORBA(T &value, const CorbaType &corba) {
    value = corba;
    return true;
}

}  // namespace corba
}  // namespace rtt_typekit_generator

#endif // RTT_TYPEKIT_GENERATOR_CORBA_TRANSPORTS_CORBA_CORBA_TYPES_H
