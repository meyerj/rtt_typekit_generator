# Helper macros/functions for CMake version 2

#function(rtt_typekit_generator_export_cache file)
#  get_cmake_property(_cache_variables CACHE_VARIABLES)
#  file(WRITE "${file}" "")
#  foreach(_var ${_cache_variables})
#    get_property(${_var}_ADVANCED   CACHE ${_var} PROPERTY ADVANCED)
#    get_property(${_var}_HELPSTRING CACHE ${_var} PROPERTY HELPSTRING)
#    get_property(${_var}_TYPE       CACHE ${_var} PROPERTY TYPE)
#    get_property(${_var}_VALUE      CACHE ${_var} PROPERTY VALUE)

#    if(#NOT ${_var}_TYPE STREQUAL "INTERNAL" AND
#       #NOT ${_var}_TYPE STREQUAL "STATIC" AND
#       #NOT ${_var} STREQUAL "CMAKE_HOME_DIRECTORY" AND
#       TRUE)
#      string(REPLACE "\"" "\\\"" ${_var}_VALUE_ESCAPED "${${_var}_VALUE}")
#      string(REPLACE "\"" "\\\"" ${_var}_HELPSTRING_ESCAPED "${${_var}_HELPSTRING}")
#      file(APPEND "${file}" "set(${_var} \"${${_var}_VALUE_ESCAPED}\" CACHE ${${_var}_TYPE} \"${${_var}_HELPSTRING_ESCAPED}\" FORCE)\n")
#    endif()
#  endforeach()
#endfunction()

#function(rtt_typekit_generator_export_variables file)
#  string(REPLACE ";" " " _set_postfix "${ARGN}")
#  get_cmake_property(_variables VARIABLES)
#  list(REMOVE_ITEM _variables _set_postfix ARGC ARGV ARGN)
#  file(WRITE "${file}" "")
#  foreach(_var ${_variables})
#    string(REPLACE "\"" "\\\"" ${_var}_VALUE_ESCAPED "${${_var}}")
#    file(APPEND "${file}" "set(${_var} \"${${_var}_VALUE_ESCAPED}\" " ${_set_postfix} ")\n")
#  endforeach()
#endfunction()

#function(rtt_typekit_generator_prepend_include_directories target)
#  get_target_property(${target}_INCLUDE_DIRS ${target} INCLUDE_DIRECTORIES)
#  list(INSERT ${target}_INCLUDE_DIRS 0 ${ARGN})
#  set_target_properties(${target} PROPERTIES INCLUDE_DIRECTORIES "${${target}_INCLUDE_DIRS}")
#endfunction()

if(CMAKE_MAJOR_VERSION LESS 3)

  function(rtt_typekit_generator_target_compile_definitions target)
    get_target_property(${target}_COMPILE_DEFINITIONS ${target} COMPILE_DEFINITIONS)
    if(NOT ${target}_COMPILE_DEFINITIONS)
      set(${target}_COMPILE_DEFINITIONS)
    endif()
    foreach(arg IN LISTS ARGN)
      if(NOT arg STREQUAL "PRIVATE" AND
         NOT arg STREQUAL "PUBLIC" AND
         NOT arg STREQUAL "INTERFACE")
        list(APPEND ${target}_COMPILE_DEFINITIONS ${arg})
      endif()
    endforeach()
    set_target_properties(${target} PROPERTIES COMPILE_DEFINITIONS "${${target}_COMPILE_DEFINITIONS}")
  endfunction()

  function(rtt_typekit_generator_target_compile_options target)
    get_target_property(${target}_COMPILE_FLAGS ${target} COMPILE_FLAGS)
    if(NOT ${target}_COMPILE_FLAGS)
      set(${target}_COMPILE_FLAGS)
    endif()
    set(_before FALSE)
    foreach(arg IN LISTS ARGN)
      if(arg STREQUAL "BEFORE")
        set(_before TRUE)
      elseif(NOT arg STREQUAL "PRIVATE" AND
             NOT arg STREQUAL "PUBLIC" AND
             NOT arg STREQUAL "INTERFACE")
        if(NOT _before)
          set(${target}_COMPILE_FLAGS "${${target}_COMPILE_FLAGS} ${arg}")
        else()
          set(${target}_COMPILE_FLAGS "${arg} ${${target}_COMPILE_FLAGS}")
        endif()
      endif()
    endforeach()
    set_target_properties(${target} PROPERTIES COMPILE_FLAGS "${${target}_COMPILE_FLAGS}")
  endfunction()

else(CMAKE_MAJOR_VERSION LESS 3)

  # Forward to built-in command target_compile_definitions()
  macro(rtt_typekit_generator_target_compile_definitions)
    target_compile_definitions(${ARGN})
  endmacro()

  # Forward to built-in command target_compile_options()
  macro(rtt_typekit_generator_target_compile_options)
    target_compile_options(${ARGN})
  endmacro()

endif()
