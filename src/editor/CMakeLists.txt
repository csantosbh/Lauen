cmake_minimum_required(VERSION 3.2)
include("../../config/BuildConfiguration.cmake")
set(CMAKE_CXX_COMPILER "${NACL_PEPPER_FOLDER}/toolchain/linux_pnacl/bin/pnacl-clang++")
project("LauEngineCanvas")

##
# Configure compiler
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -O3 -DRELEASE")

##
# External libraries
include_directories("${NACL_PEPPER_FOLDER}/include")
link_directories(${NACL_PEPPER_FOLDER}/lib/pnacl/Release)

##
# Configure main target
file(GLOB_RECURSE server_static_files "*.html" "*.nmf")
file(GLOB_RECURSE cxx_canvas_files "*.cpp")

add_executable(lau_canvas ${cxx_canvas_files})
target_link_libraries(lau_canvas ppapi_cpp ppapi ppapi_gles2)
ADD_CUSTOM_COMMAND( TARGET lau_canvas POST_BUILD
    COMMAND ${NACL_PEPPER_BIN_FOLDER}/pnacl-finalize
            ARGS lau_canvas -o lau_canvas.pexe)

##
# Installation
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lau_canvas.pexe DESTINATION editor/)
install(FILES ${server_static_files} DESTINATION editor/)
