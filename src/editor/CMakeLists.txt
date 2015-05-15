CMAKE_MINIMUM_REQUIRED(VERSION 3.2)
INCLUDE("../../config/BuildConfiguration.cmake")
PROJECT("LauEngineCanvas")

##
# Configure compiler
SET(NACL_CXX_COMPILER "${NACL_PEPPER_FOLDER}/toolchain/linux_pnacl/bin/pnacl-clang++")
SET(CMAKE_CXX_FLAGS_DEBUG -Wall -g -DDEBUG)
SET(CMAKE_CXX_FLAGS_RELEASE -Wall -O3 -DRELEASE)

## Use release by default; debug otherwise
STRING(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)
IF(CMAKE_BUILD_TYPE STREQUAL release)
    SET(NACL_CXX_FLAGS ${CMAKE_CXX_FLAGS_RELEASE})
ELSE()
    SET(NACL_CXX_FLAGS ${CMAKE_CXX_FLAGS_DEBUG})
ENDIF()

##
# External libraries
set(NACL_INCLUDE_DIRECTORIES "-I${NACL_PEPPER_FOLDER}/include")
SET(NACL_LINK_DIRECTORIES -L${NACL_PEPPER_FOLDER}/lib/pnacl/Release)
SET(NACL_LINK_LIBRARIES -lppapi_cpp -lppapi -lppapi_gles2)

##
# Configure main target
set(server_static_files "dist/")
FILE(GLOB_RECURSE cxx_canvas_files "*.cpp")

##
# Now take a breath. There's a reason why I'm not using cmake's
# builtin target generation functionalities: I need to use a different
# compiler here; one provided by NaCL. And I can't change compilers
# multiple times during a single configure.

# Incremental build: Generate .o files
FOREACH(cxx_canvas_file ${cxx_canvas_files})
    GET_FILENAME_COMPONENT(cxx_canvas_filename_ext ${cxx_canvas_file} NAME)
    GET_FILENAME_COMPONENT(cxx_canvas_filename ${cxx_canvas_file} NAME_WE)
    SET(o_canvas_files "${cxx_canvas_filename}.o" ${o_canvas_files})
    ADD_CUSTOM_COMMAND(OUTPUT "${cxx_canvas_filename}.o"
        COMMAND ${NACL_CXX_COMPILER} -c ${cxx_canvas_file}
        ${NACL_INCLUDE_DIRECTORIES} ${NACL_CXX_FLAGS} -o ${cxx_canvas_filename}.o
        DEPENDS ${cxx_canvas_file}
        COMMENT "Building ${cxx_canvas_filename_ext}...")
ENDFOREACH()

# Build nacl app
ADD_CUSTOM_COMMAND(OUTPUT lau_canvas
    COMMAND ${NACL_CXX_COMPILER} ${o_canvas_files}
            ${NACL_LINK_DIRECTORIES} ${NACL_LINK_LIBRARIES} -o lau_canvas
    DEPENDS ${o_canvas_files}
    COMMENT "Linking NaCL canvas...")

# Post build nacl-finalize
ADD_CUSTOM_COMMAND( OUTPUT lau_canvas.pexe
    COMMAND ${NACL_PEPPER_BIN_FOLDER}/pnacl-finalize
    lau_canvas -o lau_canvas.pexe
    DEPENDS lau_canvas
    COMMENT "Finalizing NaCL canvas...")
ADD_CUSTOM_TARGET(canvas ALL DEPENDS lau_canvas.pexe)

##
# Installation
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/lau_canvas.pexe DESTINATION editor/)
INSTALL(DIRECTORY ${server_static_files} DESTINATION editor)
