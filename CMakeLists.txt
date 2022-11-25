cmake_minimum_required(VERSION 3.12)

include("../CMakeShared.txt")

set(PROJ_NAME pr_chromium)

project(${PROJ_NAME} CXX)

# set(PRECOMPILED_HEADER "stdafx")

link_external_library(iclient)
link_external_library(glfw)
link_external_library(iglfw)
link_external_library(prosper)
link_external_library(client)
link_external_library(wgui)
add_external_library(luajit)
if(WIN32)
	add_external_library(lua)
endif()

add_include_dir(glm)
add_include_dir(materialsystem)
add_include_dir(cmaterialsystem)

link_generic_module_libraries()
register_pragma_module(${PROJ_NAME})

if(WIN32)
	set(OS_LIB_BIN_EXT ".dll")
	set(OS_LIB_SHARED_EXT ".lib")
	set(OS_EXE_EXT ".exe")
else()
	set(OS_LIB_BIN_EXT ".so")
	set(OS_LIB_SHARED_EXT ".so")
	set(OS_EXE_EXT "")
endif()

set(DEPENDENCY_PR_CHROMIUM_WRAPPER_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/external_libs/pr_chromium_wrapper/${CONFIG_BUILD_TYPE}/pr_chromium_wrapper${OS_LIB_SHARED_EXT}" CACHE STRING "" FORCE)

add_subdirectory("external_libs/pr_chromium_wrapper")

add_dependencies(${PROJ_NAME} pr_chromium_wrapper pr_chromium_subprocess)

set_target_properties(pr_chromium PROPERTIES FOLDER modules/chromium)
set_target_properties(pr_chromium_wrapper PROPERTIES FOLDER modules/chromium)
set_target_properties(pr_chromium_subprocess PROPERTIES FOLDER modules/chromium)

set_property(GLOBAL PROPERTY PRAGMA_MODULE_SKIP_TARGET_PROPERTY_FOLDER 1)
