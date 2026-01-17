include("${CMAKE_SOURCE_DIR}/cmake/install_helper.cmake")

set(version "2026-01-11")
set(base_url "https://github.com/Silverlan/pr_chromium_wrapper/releases/download")
set(chromium_wrapper_toolset)

if(WIN32)
    set(chromium_wrapper_toolset "msvc")
endif()

pr_fetch_prebuilt_binaries("${PRAGMA_DEPS_DIR}/chromium_wrapper" "${base_url}" "${version}" TOOLSET ${chromium_wrapper_toolset})
