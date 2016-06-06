cmake_minimum_required(VERSION 2.8.3)
project(rtt_typekit_generator)

## Definitions
set(${PROJECT_NAME}_VERSION 0.0.0)

## Find Orocos RTT
find_package(OROCOS-RTT QUIET)
if(NOT OROCOS-RTT_FOUND)
  message (FATAL_ERROR "\nCould not find Orocos. Please use the shell command\n 'source orocos_toolchain/env.sh' and then run cmake again.")
endif()

# Defines the orocos_* cmake macros. See that file for additional
# documentation.
include(${OROCOS-RTT_USE_FILE_PATH}/UseOROCOS-RTT.cmake)

## Find Boost
find_package(Boost REQUIRED COMPONENTS serialization)

###########
## Build ##
###########

include_directories(
  include
)

#
# Generates and installs our package. Must be the last statement such
# that it can pick up all above settings.
#
# Giving the package name here explicitly skips invocation of catkin_package(),
# which would be bad because catkin overwrites our cmake configuration.
#
orocos_generate_package(${PROJECT_NAME})

#############
## Install ##
#############

#install(PROGRAMS 
#  scripts/create_rtt_typekit
#  DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION})

install(
  DIRECTORY include/
  DESTINATION include
)

# Sources need to be installed because they are required during typekit generation.
install(
  DIRECTORY src/
  DESTINATION share/${PROJECT_NAME}/src
)

## Install template files to both install and devel-space
#install(DIRECTORY src/templates/ DESTINATION share/${PROJECT_NAME}/src/templates/)
#if(ORO_USE_CATKIN)
#  file(MAKE_DIRECTORY "${CATKIN_DEVEL_PREFIX}/${CATKIN_PACKAGE_SHARE_DESTINATION}/src")
#  execute_process(COMMAND "${CMAKE_COMMAND}" -E create_symlink "${CMAKE_CURRENT_SOURCE_DIR}/src/templates" "${CATKIN_DEVEL_PREFIX}/${CATKIN_PACKAGE_SHARE_DESTINATION}/src/templates")
#endif()

# CMake configuration
message(STATUS "Generating cmake configuration for the install-space")
set(${PROJECT_NAME}_DIR "${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}/cmake")
set(${PROJECT_NAME}_INCLUDE_DIRS)
set(${PROJECT_NAME}_SOURCE_DIR "${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}/src")
set(${PROJECT_NAME}_TEMPLATES_DIR "${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}/src/templates")
configure_file(cmake/${PROJECT_NAME}Config.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/install/cmake/${PROJECT_NAME}Config.cmake" @ONLY)
configure_file(cmake/${PROJECT_NAME}ConfigVersion.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/install/cmake/${PROJECT_NAME}ConfigVersion.cmake" @ONLY)
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/install/cmake/${PROJECT_NAME}Config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/install/cmake/${PROJECT_NAME}ConfigVersion.cmake"
  cmake/orocos_typekit_headers.cmake
  cmake/orocos_typekit_headers-cmake-helpers.cmake
  DESTINATION share/${PROJECT_NAME}/cmake
)

if(ORO_USE_CATKIN)
  message(STATUS "Generating cmake configuration for the catkin devel-space")
  set(${PROJECT_NAME}_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
  set(${PROJECT_NAME}_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include")
  set(${PROJECT_NAME}_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
  set(${PROJECT_NAME}_TEMPLATES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/templates")
  configure_file(cmake/${PROJECT_NAME}Config.cmake.in "${CATKIN_DEVEL_PREFIX}/${CATKIN_PACKAGE_SHARE_DESTINATION}/cmake/${PROJECT_NAME}Config.cmake" @ONLY)
  configure_file(cmake/${PROJECT_NAME}ConfigVersion.cmake.in "${CATKIN_DEVEL_PREFIX}/${CATKIN_PACKAGE_SHARE_DESTINATION}/cmake/${PROJECT_NAME}ConfigVersion.cmake" @ONLY)

  include("${CATKIN_DEVEL_PREFIX}/${CATKIN_PACKAGE_SHARE_DESTINATION}/cmake/${PROJECT_NAME}Config.cmake")
else()
  message(STATUS "Generating cmake configuration for the build-space (for testing only)")
  set(${PROJECT_NAME}_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
  set(${PROJECT_NAME}_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include")
  set(${PROJECT_NAME}_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
  set(${PROJECT_NAME}_TEMPLATES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/templates")
  configure_file(cmake/${PROJECT_NAME}Config.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake" @ONLY)
  configure_file(cmake/${PROJECT_NAME}ConfigVersion.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake" @ONLY)

  include("${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake")
endif()

#############
## Testing ##
#############

if(NOT DEFINED BUILD_TESTING)
  set(BUILD_TESTING ON CACHE BOOL "Build tests")
endif()

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
