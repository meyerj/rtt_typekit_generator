/******************************************************************************
 *  Copyright (c) 2016, Intermodalics BVBA                                    *
 *  All rights reserved.                                                      *
 ******************************************************************************/

#ifndef RTT_TYPEKIT_GENERATOR_CORBA_CONVERSION_H
#define RTT_TYPEKIT_GENERATOR_CORBA_CONVERSION_H

#include <rtt_typekit_generator/archive.h>
#include <rtt_typekit_generator/generator.h>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/type_traits/is_class.hpp>
#include <boost/utility/enable_if.hpp>

#include "corba-types.h"

#ifndef TYPEKIT_NAME_UPPER
    #define TYPEKIT_NAME_UPPER boost::algorithm::replace_all_copy(boost::algorithm::to_upper_copy(std::string(TYPEKIT_NAME)), "-", "_")
#endif
#define CONVERSIONS_GUARD_NAME ("RTT_TYPEKIT_GENERATOR_" + TYPEKIT_NAME_UPPER  + "_IDL")

namespace rtt_typekit_generator {
namespace corba {

template <typename T>
class ConversionPartGenerator : public PartGeneratorBase
{
public:
    typedef T type;
    typedef Archive< ConversionPartGenerator<T> > archive_type;

    ConversionPartGenerator(const std::string &type_name,
                            const std::string &c_type_name,
                            const T &prototype = T());

    template <typename ParentT>
    ConversionPartGenerator(ConversionPartGenerator<ParentT> *parent,
                          const std::string &type_name,
                          const std::string &c_type_name,
                          const T &prototype = T());

    virtual ~ConversionPartGenerator();

    virtual std::string getPartName() const { return "conversion"; }

    virtual void generate(std::ostream *stream);
    virtual std::ostream &generateStruct();

    template <typename MemberT> std::ostream &generateMember(const std::string &name, MemberT &value,
                                                             AttributesMap &attributes);
    template <typename MemberT> std::ostream &generateNativeMember(const std::string &name, MemberT &value,
                                                                   AttributesMap &attributes);
    template <typename MemberT> std::ostream &generateStructMember(const std::string &name, MemberT &value,
                                                                   AttributesMap &attributes);
    template <typename SequenceT> std::ostream &generateSequenceMember(const std::string &name, SequenceT &sequence,
                                                                       AttributesMap &attributes);

    template <typename MemberT> void introspect(const char *name_and_attributes, MemberT &value, T &) {
        AttributesMap attributes;
        std::string name = stripAttributes(name_and_attributes, attributes);
        generateMember<MemberT>(name, value, attributes);
    }

    template <typename MemberT> void generateMember(
            const std::string &name,
            typename boost::enable_if< typename Type<MemberT>::is_native, MemberT >::type &value,
            AttributesMap &attributes) {
        generateNativeMember<MemberT>(name, value, attributes);
    }

    template <typename MemberT> void generateMember(
            const std::string &name,
            typename boost::enable_if< typename Type<MemberT>::is_sequence, MemberT >::type &sequence,
            AttributesMap &attributes) {
        generateSequenceMember<MemberT>(name, sequence, attributes);
    }

    template <typename MemberT> void generateMember(
            const std::string &name,
            typename boost::enable_if< typename Type<MemberT>::is_struct, MemberT >::type &value,
            AttributesMap &attributes)
    {
        generateStructMember<MemberT>(name, value, attributes);
    }

    template <typename MemberT> void generateNestedType(
            const std::string &name,
            MemberT &field,
            AttributesMap &attributes)
    {}

    template <typename MemberT> void generateNestedType(
            const std::string &name,
            typename boost::enable_if< typename Type<MemberT>::is_struct, MemberT >::type &value,
            AttributesMap &attributes);

private:
    template <typename OtherT> friend class ConversionPartGenerator;

    Context context_;
    T prototype_;
    ContextBase *namespace_context_;
    enum Mode { GENERATE_TO_CORBA, GENERATE_FROM_CORBA } mode_;
};

template <typename T>
ConversionPartGenerator<T>::ConversionPartGenerator(
        const std::string &type_name,
        const std::string &c_type_name,
        const T &prototype)
    : PartGeneratorBase(type_name, c_type_name)
    , context_(context_stack_)
    , prototype_(prototype)
    , namespace_context_()
    , mode_()
{}

template <typename T> template <typename OtherT>
ConversionPartGenerator<T>::ConversionPartGenerator(
        ConversionPartGenerator<OtherT> *parent,
        const std::string &type_name,
        const std::string &c_type_name,
        const T &prototype)
    : PartGeneratorBase(parent, type_name, c_type_name)
    , context_(context_stack_)
    , prototype_(prototype)
    , namespace_context_(parent->namespace_context_)
    , mode_(static_cast<ConversionPartGenerator<T>::Mode>(parent->mode_))
{
}

template <typename T>
ConversionPartGenerator<T>::~ConversionPartGenerator()
{
}

template <typename T>
void ConversionPartGenerator<T>::generate(std::ostream *_stream) {
    Context c(this->context_stack_, _stream);
    namespace_context_ = &c;

    stream() << "// type '" << getTypeName() << "'" << std::endl;
    stream() << indent(2) << "namespace rtt_typekit_generator {" << std::endl;

    mode_ = GENERATE_TO_CORBA;
    generateStruct();

    mode_ = GENERATE_FROM_CORBA;
    generateStruct();

    stream() << indent(-2) << "}" << std::endl;
    stream() << std::endl;
    namespace_context_ = 0;
}

template <typename T>
std::ostream &ConversionPartGenerator<T>::generateStruct() {
    // Generate temporary stream context for structs.
    std::ostringstream struct_stream;
    {
        Context c(context_stack_, &struct_stream);

        switch(mode_) {
            case GENERATE_TO_CORBA:
                stream() << indent()  << "static bool toCORBA( " << getCCorbaName() << "& corba, " << getCFullName() << " const& value )" << std::endl;
                break;
            case GENERATE_FROM_CORBA:
                stream() << indent()  << "static bool fromCORBA( " << getCFullName() << "& value, " << getCCorbaName() << " const& corba )" << std::endl;
                break;
        }
        stream() << indent(2) << "{" << std::endl;

        archive_type archive(this, prototype_);
        archive();

        stream() << indent() << "return true;" << std::endl;
        stream() << indent(-2) << "}" << std::endl;
    }

    return stream() << struct_stream.str();
}

template <typename T> template <typename MemberT>
void ConversionPartGenerator<T>::generateNestedType(
        const std::string &name,
        typename boost::enable_if< typename Type<MemberT>::is_struct, MemberT >::type &value,
        AttributesMap &attributes)
{
    // go back to rtt_typekit_generator namespace context
    assert(namespace_context_);
    context_stack_->push(namespace_context_);

    std::string type_name = name + "Type";
    std::string c_type_name = getCTypeName() + "_" + std::string(name) + "_type";
    if (attributes.count("name")) {
        type_name = attributes.at("name");
    }
    if (attributes.count("type")) {
        c_type_name = attributes.at("type");
    }

    {
        ConversionPartGenerator<MemberT> member_generator(
                this, type_name, c_type_name, value);

        // prepend rtt_typekit_generator namespace for nested types
        member_generator.setNamespacePrefix("::rtt_typekit_generator");

        // We only need to define the type once.
        if (mode_ == GENERATE_TO_CORBA) {
            for(Namespaces::const_iterator it = member_generator.getNamespaces().begin(); it != member_generator.getNamespaces().end(); ++it) {
                stream() << indent(2) << "namespace " << *it << " {" << std::endl;
            }
            stream() << indent() << "typedef ::" << demangle(typeid(member_generator.prototype_).name()) << " " << member_generator.getCTypeName() << ";" << std::endl;
            for(Namespaces::const_reverse_iterator it = member_generator.getNamespaces().rbegin(); it != member_generator.getNamespaces().rend(); ++it) {
                stream() << indent(-2) << "}" << std::endl;
            }
        }

        member_generator.generateStruct();
    }
    context_stack_->pop();
}

template <typename T> template <typename MemberT>
std::ostream &ConversionPartGenerator<T>::generateNativeMember(
        const std::string &name,
        MemberT &field,
        AttributesMap &attributes)
{
    if (!name.empty()) {
        std::string acc = accessor(prototype_, "value", name);
        if (attributes.count("accessor")) {
            acc = "value" + attributes.at("accessor");
        }
        switch(mode_) {
            case GENERATE_TO_CORBA:
                stream() << indent() << ToCORBA<MemberT>()("corba", name, acc) << ";" << std::endl;
                break;
            case GENERATE_FROM_CORBA:
                stream() << indent() << FromCORBA<MemberT>()(acc, "corba", name) << ";" << std::endl;
                break;
        }

    } else {
        std::string acc = accessor(prototype_, "value");
        if (attributes.count("accessor")) {
            acc = attributes.at("accessor");
        }
        switch(mode_) {
            case GENERATE_TO_CORBA:
                stream() << indent() << ToCORBA<MemberT>()("corba", acc) << ";" << std::endl;
                break;
            case GENERATE_FROM_CORBA:
                stream() << indent() << FromCORBA<MemberT>()(acc, "corba") << ";" << std::endl;
                break;
        }

    }

    return stream();
}

template <typename T> template <typename MemberT>
std::ostream &ConversionPartGenerator<T>::generateStructMember(
        const std::string &name,
        MemberT &field,
        AttributesMap &attributes)
{
    // generate conversion methods for the nested type
    generateNestedType<MemberT>(name, field, attributes);

    if (!name.empty()) {
        switch(mode_) {
            case GENERATE_TO_CORBA:
                stream() << indent() << "if (!toCORBA(corba." << name << ", value." << name << ")) return false;" << std::endl;
                break;
            case GENERATE_FROM_CORBA:
                stream() << indent() << "if (!fromCORBA(value." << name << ", corba." << name << ")) return false;" << std::endl;
                break;
        }
    } else {
        switch(mode_) {
            case GENERATE_TO_CORBA:
                stream() << indent() << "if (!toCORBA(corba, value)) return false;" << std::endl;
                break;
            case GENERATE_FROM_CORBA:
                stream() << indent() << "if (!fromCORBA(value, corba)) return false;" << std::endl;
                break;
        }
    }

    return stream();
}

template <typename T> template <typename SequenceT>
std::ostream &ConversionPartGenerator<T>::generateSequenceMember(
        const std::string &name,
        SequenceT &sequence,
        AttributesMap &attributes)
{
    typedef typename SequenceT::value_type value_type;
    value_type prototype;
    generateNestedType<value_type>(name, prototype, attributes);
    return stream();
}

}  // namespace corba
}  // namespace rtt_typekit_generator

#endif // RTT_TYPEKIT_GENERATOR_CORBA_CONVERSION_H
