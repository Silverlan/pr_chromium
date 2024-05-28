include(${CMAKE_SOURCE_DIR}/cmake/pr_common.cmake)

set(PROJ_NAME pr_chromium)
pr_add_library(${PROJ_NAME} SHARED)

pr_add_dependency(${PROJ_NAME} iclient TARGET PUBLIC)
pr_add_dependency(${PROJ_NAME} ishared TARGET PUBLIC)

pr_add_headers(${PROJ_NAME} "include/")
pr_add_sources(${PROJ_NAME} "src/")

add_subdirectory("external_libs/pr_chromium_wrapper")
pr_add_dependency(${PROJ_NAME} pr_chromium_wrapper TARGET PUBLIC)
add_dependencies(${PROJ_NAME} pr_chromium_subprocess)

set_target_properties(pr_chromium PROPERTIES FOLDER modules/chromium)
set_target_properties(pr_chromium_wrapper PROPERTIES FOLDER modules/chromium)
set_target_properties(pr_chromium_subprocess PROPERTIES FOLDER modules/chromium)

pr_finalize(${PROJ_NAME} FOLDER modules/chromium)
