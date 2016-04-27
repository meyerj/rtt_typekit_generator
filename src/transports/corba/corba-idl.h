/******************************************************************************
 *  Copyright (c) 2016, Intermodalics BVBA                                    *
 *  All rights reserved.                                                      *
 ******************************************************************************/

#ifndef RTT_TYPEKIT_GENERATOR_TRANSPORTS_CORBA_CORBA_IDL_H
#define RTT_TYPEKIT_GENERATOR_TRANSPORTS_CORBA_CORBA_IDL_H

#include <rtt_typekit_generator/archive.h>
#include <rtt_typekit_generator/generator.h>
#include <rtt_typekit_generator/transports/corba/corba-types.h>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/type_traits/is_class.hpp>
#include <boost/utility/enable_if.hpp>

#ifndef TYPEKIT_NAME_UPPER
    #define TYPEKIT_NAME_UPPER boost::algorithm::replace_all_copy(boost::algorithm::to_upper_copy(std::string(TYPEKIT_NAME)), "-", "_")
#endif
#define IDL_GUARD_NAME ("RTT_TYPEKIT_GENERATOR_" + TYPEKIT_NAME_UPPER  + "_IDL")

namespace rtt_typekit_generator {
namespace corba {

template <typename T>
class IDLPartGenerator : public PartGeneratorBase
{
public:
    typedef T type;
    typedef Archive< IDLPartGenerator<T> > archive_type;

    IDLPartGenerator(const std::string &type_name,
                     const std::string &c_type_name,
                     const T &prototype = T());

    template <typename ParentT>
    IDLPartGenerator(IDLPartGenerator<ParentT> *parent,
                     const std::string &type_name,
                     const std::string &c_type_name,
                     const T &prototype = T());

    virtual ~IDLPartGenerator();

    virtual std::string getPartName() const { return "idl"; }

    virtual void generate(std::ostream *stream);
    template <typename MemberT> std::ostream &generateMember(const std::string &name, MemberT &value);

    virtual std::string getStruct();

    template <typename MemberT> std::string getType(
            const std::string &name,
            typename boost::enable_if< typename Type<MemberT>::is_native, MemberT >::type &value) {
        return getNativeType<MemberT>(name, value);
    }
    template <typename MemberT> std::string getNativeType(const std::string &name, MemberT &value);

    template <typename MemberT> std::string getType(
            const std::string &name,
            typename boost::enable_if< typename Type<MemberT>::is_struct, MemberT >::type &value)
    {
        return getStructType<MemberT>(name, value);
    }
    template <typename MemberT> std::string getStructType(const std::string &name, MemberT &value);

    template <typename MemberT> std::string getType(
            const std::string &name,
            typename boost::enable_if< typename Type<MemberT>::is_sequence, MemberT >::type &sequence) {
        return getSequenceType<MemberT>(name, sequence);
    }
    template <typename SequenceT> std::string getSequenceType(const std::string &name, SequenceT &sequence);

    template <typename MemberT> void introspect(const char *name_, MemberT &value, T &) {
        std::string name(name_ ? name_ : "");
        generateMember<MemberT>(name, value);
    }

private:
    template <typename OtherT> friend class IDLPartGenerator;

    Context context_;
    T prototype_;
    ContextBase *namespace_context_;
};

template <typename T>
IDLPartGenerator<T>::IDLPartGenerator(
        const std::string &type_name,
        const std::string &c_type_name,
        const T &prototype)
    : PartGeneratorBase(type_name, c_type_name)
    , context_(context_stack_)
    , prototype_(prototype)
    , namespace_context_()
{}

template <typename T> template <typename OtherT>
IDLPartGenerator<T>::IDLPartGenerator(
        IDLPartGenerator<OtherT> *parent,
        const std::string &type_name,
        const std::string &c_type_name,
        const T &prototype)
    : PartGeneratorBase(parent, type_name, c_type_name)
    , context_(context_stack_)
    , prototype_(prototype)
    , namespace_context_(parent->namespace_context_)
{
}

template <typename T>
IDLPartGenerator<T>::~IDLPartGenerator()
{
}

template <typename T>
void IDLPartGenerator<T>::generate(std::ostream *_stream) {
    Context c(this->context_stack_, _stream);
    namespace_context_ = &c;

    stream() << "// type '" << getTypeName() << "'" << std::endl;
    stream() << indent(2) << "module rtt_typekit_generator {" << std::endl;
    for(Namespaces::const_iterator it = getNamespaces().begin();
        it != getNamespaces().end();
        ++it) {
        stream() << indent(2) << "module " << *it << " {" << std::endl;
    }
    stream() << indent(2) << "module corba {" << std::endl;

    stream() << indent() << getStruct() << ";" << std::endl;

    stream() << indent(-2) << "};" << std::endl;
    stream() << indent(-2) << "};" << std::endl;
    for(Namespaces::const_reverse_iterator it = getNamespaces().rbegin();
        it != getNamespaces().rend();
        ++it) {
        stream() << indent(-2) << "};" << std::endl;
    }

    stream() << std::endl;
    namespace_context_ = 0;
}

template <typename T>
std::string IDLPartGenerator<T>::getStruct() {
    // Generate temporary stream context for structs.
    std::ostringstream struct_stream;
    {
        Context c(context_stack_, &struct_stream);

        stream() << "struct " << getCTypeName() << " {" << std::endl;
        indent(2);
        archive_type archive(this, prototype_);
        archive();
        stream() << indent(-2) << "}";
    }

    return struct_stream.str();
}

template <typename T> template <typename MemberT>
std::ostream &IDLPartGenerator<T>::generateMember(
        const std::string &name,
        MemberT &field)
{
    stream() << indent() << getType<MemberT>(name, field);
    if (!name.empty()) {
        stream() << " " << name << ";" << std::endl;
    } else {
        stream() << " data" << ";" << std::endl;
    }

    return stream();
}

template <typename T> template <typename MemberT>
std::string IDLPartGenerator<T>::getNativeType(
        const std::string &name,
        MemberT &field)
{
    return Type<MemberT>::getIDLTypeName();
}

template <typename T> template <typename MemberT>
std::string IDLPartGenerator<T>::getStructType(
        const std::string &name,
        MemberT &value)
{
    std::string nested_type_name;

    assert(namespace_context_);
    context_stack_->push(namespace_context_);
    {
        std::string type_name = name + "Type";
        std::string c_type_name = getCTypeName() + "_" + std::string(name) + "_type";
        IDLPartGenerator<MemberT> member_generator(
                    this, type_name, c_type_name, value);

        stream() << indent() << member_generator.getStruct() << ";" << std::endl;

        nested_type_name = member_generator.getCTypeName();
    }
    context_stack_->pop();

    return nested_type_name;
}

template <typename T> template <typename SequenceT>
std::string IDLPartGenerator<T>::getSequenceType(
        const std::string &name,
        SequenceT &sequence)
{
    typedef typename SequenceT::value_type value_type;
    value_type prototype;
    return "sequence<" + getType<value_type>(name, prototype) + ">";
}

}  // namespace corba
}  // namespace rtt_typekit_generator

#endif // RTT_TYPEKIT_GENERATOR_TRANSPORTS_CORBA_CORBA_IDL_H
