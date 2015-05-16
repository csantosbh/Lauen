CMAKE_MINIMUM_REQUIRED(VERSION 3.2)
INCLUDE("../../config/BuildConfiguration.cmake")
PROJECT("LauEngineFramework")

##
# Configure compiler

## Use release by default; debug otherwise
STRING(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)
IF(CMAKE_BUILD_TYPE STREQUAL release)
    SET(NACL_CXX_FLAGS ${CMAKE_CXX_FLAGS_RELEASE})
ELSE()
    SET(NACL_CXX_FLAGS ${CMAKE_CXX_FLAGS_DEBUG})
ENDIF()

##
# External libraries

##
# Configure main target

##
# Installation
INSTALL(DIRECTORY "default_assets" DESTINATION ./)
