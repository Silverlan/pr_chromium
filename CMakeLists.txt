include(${CMAKE_SOURCE_DIR}/cmake/pr_common.cmake)

include("cmake/fetch_prebuilt_binaries.cmake")

set(PROJ_NAME pr_chromium)
pr_add_module(${PROJ_NAME} "modules/chromium/")

pr_add_dependency(${PROJ_NAME} iclient TARGET PUBLIC)
pr_add_dependency(${PROJ_NAME} ishared TARGET PUBLIC)

pr_init_module(${PROJ_NAME})

find_package(cef REQUIRED)
find_package(chromium_wrapper REQUIRED)
# add_subdirectory("external_libs/pr_chromium_wrapper")
# add_dependencies(${PROJ_NAME} pr_chromium_wrapper)
# add_dependencies(${PROJ_NAME} pr_chromium_subprocess)

set_target_properties(pr_chromium PROPERTIES FOLDER modules/chromium)
# set_target_properties(pr_chromium_wrapper PROPERTIES FOLDER modules/chromium)
# set_target_properties(pr_chromium_subprocess PROPERTIES FOLDER modules/chromium)

pr_finalize(${PROJ_NAME} FOLDER modules/chromium)
