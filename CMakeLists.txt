CMAKE_MINIMUM_REQUIRED(VERSION 3.2)
INCLUDE("config/BuildConfiguration.cmake")

##
# Configure compiler
IF(CMAKE_COMPILER_IS_GNUCXX)
    ADD_DEFINITIONS(-std=c++11)
ENDIF()
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g -DDEBUG")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -O3 -DRELEASE")

##
# External libraries
PROJECT("LauEngine")
SET(CMAKE_INSTALL_PREFIX ${INSTALLATION_FOLDER})
INCLUDE_DIRECTORIES("/usr/include/python2.7/")
INCLUDE_DIRECTORIES("third_party/rapidjson/include/")
INCLUDE_DIRECTORIES("src/")

##
# Configure main target
FILE(GLOB py_server_files "src/server")
FILE(GLOB template_project "src/template_project")

##
# Add subprojects
ADD_SUBDIRECTORY("src/editor")
ADD_SUBDIRECTORY("src/engine")

##
# Configure installation
INSTALL(PROGRAMS src/main.py DESTINATION .)
INSTALL(DIRECTORY ${template_project} DESTINATION ./)
