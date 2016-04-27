/******************************************************************************
 *  Copyright (c) 2016, Intermodalics BVBA                                    *
 *  All rights reserved.                                                      *
 ******************************************************************************/

#ifndef RTT_TYPEKIT_GENERATOR_TRANSPORTS_CORBA_CORBA_CONVERSION_H
#define RTT_TYPEKIT_GENERATOR_TRANSPORTS_CORBA_CORBA_CONVERSION_H

#include <rtt_typekit_generator/archive.h>
#include <rtt_typekit_generator/generator.h>
#include <rtt_typekit_generator/transports/corba/corba-types.h>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/type_traits/is_class.hpp>
#include <boost/utility/enable_if.hpp>

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

    template <typename MemberT> std::ostream &generateMember(
            const std::string &name,
            MemberT &value);
    template <typename MemberT> std::ostream &generateMember(
            const std::string &name,
            typename boost::enable_if< typename Type<MemberT>::is_sequence, MemberT >::type &value);

    template <typename NestedT> void generateNestedType(
            const std::string &name,
            NestedT &value) {}

    template <typename NestedT> void generateNestedType(
            const std::string &name,
            typename boost::enable_if< typename Type<NestedT>::is_struct, NestedT >::type &value);

    template <typename MemberT> void introspect(const char *name_, MemberT &value, T &) {
        std::string name(name_ ? name_ : "");
        generateMember<MemberT>(name, value);
    }

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

        stream() << indent(+2) << "namespace corba {" << std::endl;

        switch(mode_) {
            case GENERATE_TO_CORBA:
                stream() << indent()  << "static inline bool toCORBA( " << getCCorbaName() << "& corba, " << getCFullName() << " const& value )" << std::endl;
                break;
            case GENERATE_FROM_CORBA:
                stream() << indent()  << "static inline bool fromCORBA( " << getCFullName() << "& value, " << getCCorbaName() << " const& corba )" << std::endl;
                break;
        }
        stream() << indent(2) << "{" << std::endl;

        archive_type archive(this, prototype_);
        archive();

        stream() << indent() << "return true;" << std::endl;
        stream() << indent(-2) << "}" << std::endl;
        stream() << indent(-2) << "}" << std::endl;
   }

    return stream() << struct_stream.str();
}

template <typename T> template <typename NestedT>
void ConversionPartGenerator<T>::generateNestedType(
        const std::string &name,
        typename boost::enable_if< typename Type<NestedT>::is_struct, NestedT >::type &value)
{
    // go back to rtt_typekit_generator namespace context
    assert(namespace_context_);
    context_stack_->push(namespace_context_);

    std::string type_name = name + "Type";
    std::string c_type_name = getCTypeName() + "_" + std::string(name) + "_type";

    // generate toCORBA() / fromCORBA() overloads for the nested type
    {
        ConversionPartGenerator<NestedT> member_generator(
                this, type_name, c_type_name, value);

        // prepend rtt_typekit_generator namespace for nested types
        member_generator.setNamespacePrefix("::rtt_typekit_generator");

        member_generator.stream() << indent() << "// nested type '" << member_generator.getTypeName() << "'" << std::endl;

        // We only need to define the type once.
        if (mode_ == GENERATE_TO_CORBA) {
            for(Namespaces::const_iterator it = member_generator.getNamespaces().begin(); it != member_generator.getNamespaces().end(); ++it) {
                member_generator.stream() << indent(2) << "namespace " << *it << " {" << std::endl;
            }
            member_generator.stream() << indent() << "typedef ::" << demangle(typeid(member_generator.prototype_).name()) << " " << member_generator.getCTypeName() << ";" << std::endl;
            for(Namespaces::const_reverse_iterator it = member_generator.getNamespaces().rbegin(); it != member_generator.getNamespaces().rend(); ++it) {
                member_generator.stream() << indent(-2) << "}" << std::endl;
            }
        }

        member_generator.generateStruct();
    }

    context_stack_->pop();
}

template <typename T> template <typename MemberT>
std::ostream &ConversionPartGenerator<T>::generateMember(
        const std::string &name,
        MemberT &field)
{
    // generate conversion methods for the nested type (if required)
    generateNestedType<MemberT>(name, field);

    std::string acc = accessor(prototype_, "value", name);
    switch(mode_) {
        case GENERATE_TO_CORBA:
            stream() << indent() << ToCORBA<MemberT>()(acc, "corba", name) << std::endl;
            break;
        case GENERATE_FROM_CORBA:
            stream() << indent() << FromCORBA<MemberT>()(acc, "corba", name) << std::endl;
            break;
    }

    return stream();
}

template <typename T> template <typename SequenceMemberT>
std::ostream &ConversionPartGenerator<T>::generateMember(
        const std::string &name,
        typename boost::enable_if< typename Type<SequenceMemberT>::is_sequence, SequenceMemberT >::type &sequence)
{
    typedef typename SequenceMemberT::value_type value_type;
    value_type prototype;

    // generate conversion methods for the nested type (if required)
    generateNestedType<value_type>(name, prototype);

    std::string acc = accessor(prototype_, "value", name);
    std::string corba = "corba";
    if (!name.empty()) {
        corba = corba + "." + name;
    }

    switch(mode_) {
        case GENERATE_TO_CORBA:
            stream() << indent()   << corba << ".length(" << acc << ".size());" << std::endl;
            stream() << indent(+2) << "for(std::size_t i = 0; i < " << acc << ".size(); ++i) {" << std::endl;
            stream() << indent()   <<   ToCORBA<value_type>()(acc, "corba", name, "i") << std::endl;
            stream() << indent(-2) << "}" << std::endl;
            break;
        case GENERATE_FROM_CORBA:
            stream() << indent() << acc << ".resize(" << corba << ".length());" << std::endl;
            stream() << indent(2) << "for(std::size_t i = 0; i < " << corba << ".length(); ++i) {" << std::endl;
            stream() << indent()   <<   FromCORBA<value_type>()(acc, "corba", name, "i") << std::endl;
            stream() << indent(-2) << "}" << std::endl;
            break;
    }

    return stream();
}

}  // namespace corba
}  // namespace rtt_typekit_generator

#endif // RTT_TYPEKIT_GENERATOR_TRANSPORTS_CORBA_CORBA_CONVERSION_H
