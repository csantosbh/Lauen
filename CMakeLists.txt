cmake_minimum_required(VERSION 3.2)
include("config/BuildConfiguration.cmake")

##
# Configure compiler
if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(-std=c++11)
endif()
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -O3 -DRELEASE")

##
# External libraries
project("LauEngine")
set(CMAKE_INSTALL_PREFIX ${INSTALLATION_FOLDER})
include_directories("/usr/include/python2.7/")
include_directories("third_party/rapidjson/include/")
include_directories("src/")

##
# Configure main target
file(GLOB_RECURSE config_lau_files "config/*.cfg")
file(GLOB_RECURSE py_lau_files "src/*.py")

##
# Add subprojects
#add_subdirectory("src/editor")
include(ExternalProject)
ExternalProject_Add(editor SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/editor/ 
    CMAKE_CACHE_ARGS
    -DCMAKE_INSTALL_PREFIX:PATH=${INSTALLATION_FOLDER})

##
# Auto-copy Python dependencies
#foreach(py_lau_file ${py_lau_files})
#    get_filename_component(py_filename ${py_lau_file} NAME)
#    set(lau_script_dependencies "${CMAKE_CURRENT_BINARY_DIR}/scripts/${py_filename}" ${lau_script_dependencies})
#    add_custom_command(
#        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/scripts/${py_filename}"
#        COMMAND ${CMAKE_COMMAND} -E copy ${py_lau_file} "${CMAKE_CURRENT_BINARY_DIR}/scripts/${py_filename}"
#        DEPENDS ${py_lau_file}
#        COMMENT "Updating ${py_lau_file}"
#    )
#endforeach()
#add_custom_target(lau_scripts ALL DEPENDS ${lau_script_dependencies})
install(FILES ${py_lau_files} DESTINATION scripts/)

##
# Auto-copy configuration files
#foreach(config_lau_file ${config_lau_files})
#    get_filename_component(config_filename ${config_lau_file} NAME)
#    set(lau_config_dependencies "${CMAKE_CURRENT_BINARY_DIR}/config/${config_filename}" ${lau_config_dependencies})
#    add_custom_command(
#        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/config/${config_filename}"
#        COMMAND ${CMAKE_COMMAND} -E copy ${config_lau_file} "${CMAKE_CURRENT_BINARY_DIR}/config/${config_filename}"
#        DEPENDS ${config_lau_file}
#        COMMENT "Updating ${config_lau_file}"
#    )
#endforeach()
#add_custom_target(lau_configs ALL DEPENDS ${lau_config_dependencies})
install(FILES ${config_lau_files} DESTINATION config/)
