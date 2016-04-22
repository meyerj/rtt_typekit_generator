/******************************************************************************
 *  Copyright (c) 2016, Intermodalics BVBA                                    *
 *  All rights reserved.                                                      *
 ******************************************************************************/

#include <cxxabi.h>
#include "corba-types.h"

namespace rtt_typekit_generator {
namespace corba {

// copied from http://stackoverflow.com/questions/281818/unmangling-the-result-of-stdtype-infoname
struct handle {
    char* p;
    handle(char* ptr) : p(ptr) { }
    ~handle() { std::free(p); }
};

std::string demangle(const char* name) {
    int status = 1;
    handle result(abi::__cxa_demangle(name, NULL, NULL, &status));
    return (status == 0) ? result.p : name;
}

}  // namespace corba
}  // namespace rtt_typekit_generator
