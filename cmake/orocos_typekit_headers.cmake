include(CMakeParseArguments)

if(CMAKE_MAJOR_VERSION LESS 3)
  include(${CMAKE_CURRENT_LIST_DIR}/orocos_typekit_headers-cmake2-helpers.cmake)
endif()

macro(orocos_generate_typekit_headers name)
  # For now there can be only one typekit per package:
  if(DEFINED ${PROJECT_NAME}_TYPEKITS)
    message(SEND_ERROR "orocos_generate_typekit() / orocos_generate_typekit_headers() can only be called once per CMake project!")
    return()
  endif()

  cmake_parse_arguments(_typekit "" "EXPORT;OUTPUT_DIRECTORY" "HEADERS;TYPES;NAMES;DEPENDS;LIBRARIES" ${ARGN})
  set(_typekit_NAME ${name})
  set(_typekit_PROJECT_NAME ${_typekit_NAME})
  set(_typekit_TARGET ${_typekit_NAME})
  string(REPLACE "-" "_" _typekit_CNAME ${_typekit_NAME})
  if(NOT DEFINED _typekit_INCLUDE_DIRS)
    get_directory_property(_typekit_INCLUDE_DIRS INCLUDE_DIRECTORIES)
  endif()
  if(NOT DEFINED _typekit_OUTPUT_DIRECTORY)
    set(_typekit_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/rtt_typekit_generator/${_typekit_NAME})
  endif()
  if(DEFINED _typekit_EXPORT)
    set(_typekit_EXPORT EXPORT ${_typekit_EXPORT})
  endif()

  # automatically assign type names if none were given by the user
  list(LENGTH _typekit_TYPES _typekit_TYPES_length1)
  math(EXPR _typekit_TYPES_length2 "${_typekit_TYPES_length1} - 1")
  if(NOT DEFINED _typekit_NAMES)
    set(_typekit_NAMES)
    foreach(_i RANGE ${_typekit_TYPES_length2})
      list(GET _typekit_TYPES ${_i} _type)
      string(REPLACE "::" "/" _name ${_type})
      set(_name "/${_name}")
      list(APPEND _typekit_NAMES ${_name})
    endforeach()
  endif()

  message(STATUS "Generating RTT typekit '${_typekit_NAME}' for types:")
  foreach(_i RANGE ${_typekit_TYPES_length2})
    list(GET _typekit_TYPES ${_i} _type)
    list(GET _typekit_NAMES ${_i} _name)
    message(STATUS "- ${_name} (${_type})")
  endforeach()

  # set default values from ${PROJECT_NAME}
  if(NOT DEFINED _typekit_VERSION)
    if (DEFINED ${PROJECT_NAME}_VERSION)
      set(_typekit_VERSION ${${PROJECT_NAME}_VERSION})
    else()
      set(_typekit_VERSION "0.0.0")
    endif()
  endif()

  # create target directories
  file(MAKE_DIRECTORY ${_typekit_OUTPUT_DIRECTORY})
  file(MAKE_DIRECTORY ${_typekit_OUTPUT_DIRECTORY}/corba)
  set(_typekit_INCLUDE_DIR ${_typekit_OUTPUT_DIRECTORY}/include/orocos/${_typekit_NAME}/typekit)
  file(MAKE_DIRECTORY ${_typekit_INCLUDE_DIR})
  include_directories(BEFORE ${_typekit_OUTPUT_DIRECTORY}/include/orocos)

  # find headers and generate includes.h
  file(WRITE ${_typekit_INCLUDE_DIR}/includes.h "")
  set(_typekit_INCLUDES)
  set(_typekit_HEADERS_ABSOLUTE)
  foreach(_header ${_typekit_HEADERS})
    if(IS_ABSOLUTE ${_header})
      set(_header_ABSOLUTE ${_header})
    else()
      find_file(_header_ABSOLUTE ${_header} PATHS ${_typekit_INCLUDE_DIRS} NO_DEFAULT_PATH)
      if(NOT _header_ABSOLUTE)
        set(_message "Generated typekit '${_typekit_NAME}' depends on header '${_header}', but this header could not be found in one of the following include directories:\n")
        foreach(_include_dir ${_typekit_INCLUDE_DIRS})
          set(_message ${_message} " - ${_include_dir}\n")
        endforeach()
        message(FATAL_ERROR ${_message})
      endif()
    endif()
    list(APPEND _typekit_HEADERS_ABSOLUTE ${_header_ABSOLUTE})
    #file(APPEND ${_typekit_INCLUDE_DIR}/includes.h "#include \"${_header_ABSOLUTE}\"\n")
    file(APPEND ${_typekit_INCLUDE_DIR}/includes.h "#include <${_header}>\n")
    #set(_typekit_INCLUDES "${_typekit_INCLUDES}#include \"${_header_ABSOLUTE}\"\n")
    set(_typekit_INCLUDES "${_typekit_INCLUDES}#include <${_header}>\n")
    unset(_header_ABSOLUTE CACHE)
  endforeach()

  # generate types.h
  file(WRITE ${_typekit_INCLUDE_DIR}/types.h "#define TYPEKIT_NAME \"${_typekit_NAME}\"\n\n")
  list(LENGTH _typekit_TYPES _typekit_TYPES_length1)
  math(EXPR _typekit_TYPES_length2 "${_typekit_TYPES_length1} - 1")
  foreach(_i RANGE ${_typekit_TYPES_length2})
    list(GET _typekit_TYPES ${_i} _type)
    list(GET _typekit_NAMES ${_i} _name)
    if(NOT _name)
      string(REPLACE "::" "." _name ${_type})
    endif()
    file(APPEND ${_typekit_INCLUDE_DIR}/types.h "#define TYPE${_i} ${_type}\n")
    file(APPEND ${_typekit_INCLUDE_DIR}/types.h "#define TYPE_NAME${_i} \"${_name}\"\n")
    file(APPEND ${_typekit_INCLUDE_DIR}/types.h "#define C_TYPE_NAME${_i} \"${_type}\"\n")
  endforeach()
  file(APPEND ${_typekit_INCLUDE_DIR}/types.h "\n#define TYPE_CNT ${_typekit_TYPES_length1}\n")
  file(APPEND ${_typekit_INCLUDE_DIR}/types.h "#define TYPE_NAME_CNT ${_typekit_TYPES_length1}\n")
  file(APPEND ${_typekit_INCLUDE_DIR}/types.h "\n#define TYPE(i) TYPE ## i\n")
  file(APPEND ${_typekit_INCLUDE_DIR}/types.h "#define TYPE_NAME(i) TYPE_NAME ## i\n")
  file(APPEND ${_typekit_INCLUDE_DIR}/types.h "#define C_TYPE_NAME(i) C_TYPE_NAME ## i\n")

  # generate CMakeLists.txt
  configure_file(${rtt_typekit_generator_TEMPLATES_DIR}/CMakeLists.txt.in ${_typekit_OUTPUT_DIRECTORY}/CMakeLists.txt @ONLY)
  configure_file(${rtt_typekit_generator_TEMPLATES_DIR}/corba/CMakeLists.txt.in ${_typekit_OUTPUT_DIRECTORY}/corba/CMakeLists.txt @ONLY)

  # generate Types.hpp
  set(_typekit_EXTERN_TEMPLATE_DECLARATIONS)
  foreach(_i RANGE ${_typekit_TYPES_length2})
    list(GET _typekit_TYPES ${_i} _type)
    set(_typekit_EXTERN_TEMPLATE_DECLARATIONS "${_typekit_EXTERN_TEMPLATE_DECLARATIONS}extern template class RTT_EXPORT RTT::internal::DataSource< ${_type} >;\n")
    set(_typekit_EXTERN_TEMPLATE_DECLARATIONS "${_typekit_EXTERN_TEMPLATE_DECLARATIONS}extern template class RTT_EXPORT RTT::internal::AssignableDataSource< ${_type} >;\n")
    set(_typekit_EXTERN_TEMPLATE_DECLARATIONS "${_typekit_EXTERN_TEMPLATE_DECLARATIONS}extern template class RTT_EXPORT RTT::internal::ValueDataSource< ${_type} >;\n")
    set(_typekit_EXTERN_TEMPLATE_DECLARATIONS "${_typekit_EXTERN_TEMPLATE_DECLARATIONS}extern template class RTT_EXPORT RTT::internal::ConstantDataSource< ${_type} >;\n")
    set(_typekit_EXTERN_TEMPLATE_DECLARATIONS "${_typekit_EXTERN_TEMPLATE_DECLARATIONS}extern template class RTT_EXPORT RTT::internal::ReferenceDataSource< ${_type} >;\n")
    set(_typekit_EXTERN_TEMPLATE_DECLARATIONS "${_typekit_EXTERN_TEMPLATE_DECLARATIONS}extern template class RTT_EXPORT RTT::OutputPort< ${_type} >;\n")
    set(_typekit_EXTERN_TEMPLATE_DECLARATIONS "${_typekit_EXTERN_TEMPLATE_DECLARATIONS}extern template class RTT_EXPORT RTT::InputPort< ${_type} >;\n")
    set(_typekit_EXTERN_TEMPLATE_DECLARATIONS "${_typekit_EXTERN_TEMPLATE_DECLARATIONS}extern template class RTT_EXPORT RTT::Property< ${_type} >;\n")
    set(_typekit_EXTERN_TEMPLATE_DECLARATIONS "${_typekit_EXTERN_TEMPLATE_DECLARATIONS}extern template class RTT_EXPORT RTT::Attribute< ${_type} >;\n")
    set(_typekit_EXTERN_TEMPLATE_DECLARATIONS "${_typekit_EXTERN_TEMPLATE_DECLARATIONS}\n")
  endforeach()
  configure_file(${rtt_typekit_generator_TEMPLATES_DIR}/Types.hpp.in ${_typekit_INCLUDE_DIR}/Types.hpp @ONLY)

  if(ORO_USE_CATKIN)
    file(COPY
      ${_typekit_INCLUDE_DIR}/includes.h
      ${_typekit_INCLUDE_DIR}/types.h
      ${_typekit_INCLUDE_DIR}/Types.hpp
      DESTINATION ${CATKIN_DEVEL_PREFIX}/include/orocos/${_typekit_NAME}/typekit
    )
  endif()

  # generate typekit plugin
  configure_file(${rtt_typekit_generator_TEMPLATES_DIR}/typekit.cpp.in ${_typekit_OUTPUT_DIRECTORY}/typekit.cpp @ONLY)

  # generate CORBA transport plugin
  configure_file(${rtt_typekit_generator_TEMPLATES_DIR}/corba/transport.cpp.in ${_typekit_OUTPUT_DIRECTORY}/corba/transport.cpp @ONLY)

  # create stand-alone introspection library
  configure_file(${rtt_typekit_generator_TEMPLATES_DIR}/introspection.cpp.in ${_typekit_OUTPUT_DIRECTORY}/introspection.cpp @ONLY)
  add_library(${_typekit_TARGET}_introspection SHARED ${_typekit_OUTPUT_DIRECTORY}/introspection.cpp)
  target_link_libraries(${_typekit_TARGET}_introspection ${_typekit_LIBRARIES})

  # create generator target
#  configure_file(${rtt_typekit_generator_TEMPLATES_DIR}/generator.cpp.in ${_typekit_OUTPUT_DIRECTORY}/generator.cpp @ONLY)
  add_executable(${_typekit_TARGET}_generator
#    ${_typekit_OUTPUT_DIRECTORY}/generator.cpp
    ${rtt_typekit_generator_SOURCE_DIR}/generator.cpp
    ${rtt_typekit_generator_SOURCE_DIR}/transports/corba.cpp
  )
  target_compile_options(${_typekit_TARGET}_generator PRIVATE
    -include ${_typekit_INCLUDE_DIR}/includes.h
    -include ${_typekit_INCLUDE_DIR}/types.h
  )
  set_source_files_properties(
    ${rtt_typekit_generator_SOURCE_DIR}/generator.cpp
    ${rtt_typekit_generator_SOURCE_DIR}/transports/corba.cpp
    OBJECT_DEPENDS ${_typekit_HEADERS_ABSOLUTE}
  )
  target_link_libraries(${_typekit_TARGET}_generator ${_typekit_LIBRARIES} ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})
  set_target_properties(${_typekit_TARGET}_generator PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${_typekit_OUTPUT_DIRECTORY})
  get_target_property(${_typekit_TARGET}_generator_LOCATION ${_typekit_TARGET}_generator LOCATION)

  # create generate target
  add_custom_command(
    OUTPUT ${_typekit_OUTPUT_DIRECTORY}/stamp
    COMMAND ${${_typekit_TARGET}_generator_LOCATION}
    COMMAND touch stamp
    DEPENDS ${_typekit_TARGET}_generator ${_typekit_HEADERS_ABSOLUTE}
    WORKING_DIRECTORY "${_typekit_OUTPUT_DIRECTORY}"
    VERBATIM
    COMMENT "Generating code for typekit ${_typekit_NAME}..."
  )
  add_custom_target(${_typekit_TARGET}_generate_typekit ALL
    DEPENDS ${_typekit_OUTPUT_DIRECTORY}/stamp
  )

  # add generated typekit to the clean target
#  set_property(DIRECTORY
#    APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
#    ${_typekit_OUTPUT_DIRECTORY}/types.h
#    ${_typekit_OUTPUT_DIRECTORY}/includes.h
#    ${_typekit_OUTPUT_DIRECTORY}/CMakeLists.txt
#    ${_typekit_OUTPUT_DIRECTORY}/Types.hpp
#    ${_typekit_OUTPUT_DIRECTORY}/typekit.cpp
#    ${_typekit_OUTPUT_DIRECTORY}/introspection.cpp
#    ${_typekit_OUTPUT_DIRECTORY}/generator.cpp
#  )

  # export some variables
  set(${_typekit_TARGET}_OUTPUT_DIRECTORY ${_typekit_OUTPUT_DIRECTORY})
  set(${_typekit_TARGET}_EXPORTED_TARGETS ${_typekit_TARGET}_generate_typekit)
  set(${PROJECT_NAME}_TYPEKITS ${${PROJECT_NAME}_TYPEKITS} ${_typekit_NAME})
  set(${PROJECT_NAME}_TYPEKIT_TYPES ${${PROJECT_NAME}_TYPEKIT_TYPES} ${_typekit_TYPES})

  # cleanup internal variables
  unset(_header)
  unset(_header_ABSOLUTE)
  unset(_message)
  unset(_name)
  unset(_type)
  unset(_typekit_CNAME)
  unset(_typekit_DEPENDS)
  unset(_typekit_EXTERN_TEMPLATE_DECLARATIONS)
  unset(_typekit_EXPORT)
  unset(_typekit_HEADERS)
  unset(_typekit_HEADERS_ABSOLUTE)
  unset(_typekit_INCLUDES)
  unset(_typekit_LIBRARIES)
  unset(_typekit_NAME)
  unset(_typekit_NAMES)
  unset(_typekit_OUTPUT_DIRECTORY)
  unset(_typekit_PROJECT_NAME)
  unset(_typekit_TARGET)
  unset(_typekit_TYPES)
  unset(_typekit_TYPES_length1)
  unset(_typekit_TYPES_length2)
  unset(_typekit_VERSION)
endmacro()

macro(orocos_typekit_headers name)
  # Generate...
  orocos_generate_typekit_headers(${name} OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/rtt_typekit_generator/${name} ${ARGN})
  # ... and build the typekit
  add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/rtt_typekit_generator/${name} rtt_typekit_generator/${name})
endmacro()
