/******************************************************************************
 *  Copyright (c) 2016, Intermodalics BVBA                                    *
 *  All rights reserved.                                                      *
 ******************************************************************************/

#include <rtt_typekit_generator/archive.h>
#include <rtt_typekit_generator/generator.h>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/system/system_error.hpp>
#include <boost/type_traits/is_class.hpp>
#include <boost/utility/enable_if.hpp>

#include <fstream>
#include <iostream>

#include "corba-types.h"

#define TYPEKIT_NAME_UPPER boost::algorithm::to_upper_copy(std::string(TYPEKIT_NAME))
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
                          const std::string &c_type_name);

    template <typename ParentT>
    IDLPartGenerator(IDLPartGenerator<ParentT> *parent,
                          const std::string &type_name,
                          const std::string &c_type_name);

    virtual ~IDLPartGenerator();

    virtual std::string getPartName() const { return "idl"; }

    virtual void generate(std::ostream *stream);
    virtual std::ostream &generateStruct();

    template <typename MemberT> std::ostream &generateMember(const char *name, MemberT &value);
    template <typename MemberT> std::ostream &generateNativeType(const char *name, MemberT &value);
    template <typename MemberT> std::ostream &generateStructType(const char *name, MemberT &value);
    template <typename SequenceT> std::ostream &generateSequenceType(const char *name, SequenceT &sequence);

    template <typename MemberT> void introspect(const char *name, MemberT &value, T &) {
        generateMember<MemberT>(name, value);
    }

    template <typename MemberT> void generateType(
            const char *name,
            typename boost::enable_if< typename CorbaType<MemberT>::is_native, MemberT >::type &value) {
        generateNativeType<MemberT>(name, value);
    }

    template <typename MemberT> void generateType(
            const char *name,
            typename boost::enable_if< typename CorbaType<MemberT>::is_sequence, MemberT >::type &sequence) {
        generateSequenceType<MemberT>(name, sequence);
    }

    template <typename MemberT> void generateType(
            const char *name,
            typename boost::enable_if< typename CorbaType<MemberT>::is_struct, MemberT >::type &value)
    {
        generateStructType<MemberT>(name, value);
    }

private:
    Context context_;
    T prototype_;
    ContextBase *module_context_;
};

template <typename T>
IDLPartGenerator<T>::IDLPartGenerator(const std::string &type_name,
                                                const std::string &c_type_name)
    : PartGeneratorBase(type_name, c_type_name)
    , context_(context_stack_)
    , module_context_(0)
{}

template <typename T> template <typename OtherT>
IDLPartGenerator<T>::IDLPartGenerator(
        IDLPartGenerator<OtherT> *parent,
        const std::string &type_name,
        const std::string &c_type_name)
    : PartGeneratorBase(parent, type_name, c_type_name)
    , context_(context_stack_)
{
}

template <typename T>
IDLPartGenerator<T>::~IDLPartGenerator()
{
}

template <typename T>
void IDLPartGenerator<T>::generate(std::ostream *_stream) {
    Context c(this->context_stack_, _stream);
    module_context_ = &c;

    stream() << "// type '" << getTypeName() << "'" << std::endl;
    stream() << indent(2) << "module rtt_typekit_generator {" << std::endl;
    for(Namespaces::const_iterator it = getNamespaces().begin();
        it != getNamespaces().end();
        ++it) {
        stream() << indent(2) << "module " << *it << " {" << std::endl;
    }
    stream() << indent(2) << "module corba {" << std::endl;

    stream() << indent();
    generateStruct() << ";" << std::endl;

    stream() << indent(-2) << "};" << std::endl;
    for(Namespaces::const_iterator it = getNamespaces().begin();
        it != getNamespaces().end();
        ++it) {
        stream() << indent(-2) << "};" << std::endl;
    }

    stream() << indent(-2) << "};" << std::endl;
    stream() << std::endl;
    module_context_ = 0;
}

template <typename T>
std::ostream &IDLPartGenerator<T>::generateStruct() {
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

    return stream() << struct_stream.str();
}

template <typename T> template <typename MemberT>
std::ostream &IDLPartGenerator<T>::generateMember(
        const char *name,
        MemberT &field)
{
    stream() << indent();
    generateType<MemberT>(name, field);
    if (name) {
        stream() << " " << name << ";" << std::endl;
    } else {
        stream() << " data" << ";" << std::endl;
    }

    return stream();
}

template <typename T> template <typename MemberT>
std::ostream &IDLPartGenerator<T>::generateNativeType(
        const char *name,
        MemberT &field)
{
    return stream() << CorbaType<MemberT>::getIDLTypeName();
}

template <typename T> template <typename MemberT>
std::ostream &IDLPartGenerator<T>::generateStructType(
        const char *name,
        MemberT &value)
{
    IDLPartGenerator<MemberT> member_generator(
                this,
                getTypeName() + "/" + name + "Type",
                "_" + std::string(name) + "_type");

    return member_generator.generateStruct();
}

template <typename T> template <typename SequenceT>
std::ostream &IDLPartGenerator<T>::generateSequenceType(
        const char *name,
        SequenceT &sequence)
{
    if (CorbaType<typename SequenceT::value_type>::is_struct::value) {
        // structs used in sequences must be defined outside the current struct.
        std::string nested_type_name = getCTypeName() + "_" + name;
        {
            context_stack_->push(module_context_);
            generateType<typename SequenceT::value_type>(
                        nested_type_name.c_str(),
                        *sequence.data());
            stream() << ";" << std::endl << indent();
            context_stack_->pop();
        }

        stream() << "sequence<_" << nested_type_name << "_type>";

    } else {
        stream() << "sequence<";
        generateType<typename SequenceT::value_type>(name, *sequence.data());
        stream() << ">";
    }

    return stream();
}

class TransportGenerator : public TransportGeneratorBase
{
    virtual std::string getName() const { return "corba"; }
    virtual std::string getDescription() const { return "CORBA transport"; }
    virtual void generate(const std::string &output_directory);
};

void TransportGenerator::generate(const std::string &output_directory) {
    /**************************************************************************
     * Generate IDL
     *************************************************************************/
    std::string idl_file_name(output_directory + TYPEKIT_NAME + "Types.idl");
    std::clog << "- Generating " << idl_file_name << "..." << std::endl;
    std::ofstream idl_file(idl_file_name.c_str());
    if (!idl_file) throw boost::system::system_error(errno, boost::system::system_category());

    // header
    idl_file << "// Generated by rtt_typekit_generator" << std::endl
             // << std::endl
             // << "#ifndef " << IDL_GUARD_NAME << std::endl
             // << "#define " << IDL_GUARD_NAME << std::endl
             << std::endl;

    // types
    for(TypeGenerators::const_iterator type_it = part_generators_.begin();
        type_it != part_generators_.end();
        ++type_it) {
        std::string type = type_it->first;
        PartGeneratorMap::const_iterator generator = type_it->second.find("idl");

        if (generator == type_it->second.end() || !generator->second) {
            std::clog << "  - WARNING: Could not find an IDL generator for type '"
                      <<      type << "'!" << std::endl;
            continue;
        }

        generator->second->generate(&idl_file);

        std::clog << "  - " << type << std::endl;
    }

    // footer
    // idl_file << "#endif  // " << IDL_GUARD_NAME << std::endl;
}

// declare generators
DECLARE_TRANSPORT_GENERATOR("corba", TransportGenerator);
DECLARE_PART_GENERATOR_TEMPLATE("corba", IDLPartGenerator);

}  // namespace corba
}  // namespace rtt_typekit_generator
