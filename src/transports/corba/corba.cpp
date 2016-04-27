/******************************************************************************
 *  Copyright (c) 2016, Intermodalics BVBA                                    *
 *  All rights reserved.                                                      *
 ******************************************************************************/

#include <rtt_typekit_generator/generator.h>

#include <boost/system/system_error.hpp>

#include <fstream>
#include <iostream>

#include "corba-idl.h"
#include "corba-conversion.h"

namespace rtt_typekit_generator {
namespace corba {

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
    idl_file
            << "// Generated by rtt_typekit_generator" << std::endl
            << std::endl;

    // for each type...
    for(TypeGenerators::const_iterator type_it = part_generators_.begin();
        type_it != part_generators_.end();
        ++type_it) {
        std::string type = type_it->first;
        PartGeneratorMap::const_iterator generator = type_it->second.find("idl");

        if (generator == type_it->second.end() || !generator->second) {
            std::clog << "  - WARNING: Could not find a generator named 'idl' for type '"
                      <<      type << "'!" << std::endl;
            continue;
        }

        generator->second->generate(&idl_file);

        std::clog << "  - " << type << std::endl;
    }
    idl_file.close();

    /**************************************************************************
     * Generate Conversions.hpp
     *************************************************************************/
    std::string conversions_file_name(output_directory + "Conversions.hpp");
    std::clog << "- Generating " << conversions_file_name << "..." << std::endl;
    std::ofstream conversions_file(conversions_file_name.c_str());
    if (!conversions_file) throw boost::system::system_error(errno, boost::system::system_category());

    // header
    conversions_file
            << "// Generated by rtt_typekit_generator" << std::endl
            << std::endl
            << "#ifndef RTT_TYPEKIT_GENERATOR_" << TYPEKIT_NAME_UPPER << "_CONVERSIONS_HPP" << std::endl
            << "#define RTT_TYPEKIT_GENERATOR_" << TYPEKIT_NAME_UPPER << "_CONVERSIONS_HPP" << std::endl
            << std::endl
            << "#include <rtt_typekit_generator/transports/corba/corba-types.h>" << std::endl
            << std::endl
            << "#include \"" << TYPEKIT_NAME << "/typekit/includes.h\"" << std::endl
            << "#include \"" << TYPEKIT_NAME << "/typekit/types.h\"" << std::endl
            << "#include \"" << TYPEKIT_NAME << "TypesC.h\"" << std::endl
            << std::endl;

    // for each type...
    for(TypeGenerators::const_iterator type_it = part_generators_.begin();
        type_it != part_generators_.end();
        ++type_it) {
        std::string type = type_it->first;
        PartGeneratorMap::const_iterator generator = type_it->second.find("conversion");

        if (generator == type_it->second.end() || !generator->second) {
            std::clog << "  - WARNING: Could not find a generator named 'conversion' for type '"
                      <<      type << "'!" << std::endl;
            continue;
        }

        generator->second->generate(&conversions_file);

        std::clog << "  - " << type << std::endl;
    }

    conversions_file << "#endif  // RTT_TYPEKIT_GENERATOR_" << TYPEKIT_NAME_UPPER << "_CONVERSIONS_HPP" << std::endl;
    conversions_file.close();
}

// declare generators
DECLARE_TRANSPORT_GENERATOR("corba", TransportGenerator);
DECLARE_PART_GENERATOR_TEMPLATE("corba", IDLPartGenerator);
DECLARE_PART_GENERATOR_TEMPLATE("corba", ConversionPartGenerator);

}  // namespace corba
}  // namespace rtt_typekit_generator