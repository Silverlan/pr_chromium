set(PCK "chromium_wrapper")

if (${PCK}_FOUND)
  return()
endif()

find_file(${PCK}_RUNTIME
  NAMES pr_chromium_wrapper.dll pr_chromium_wrapper.so
  HINTS ${PRAGMA_DEPS_DIR}/chromium_wrapper/bin
)

find_program(${PCK}_EXECUTABLE
  NAMES pr_chromium_subprocess.exe pr_chromium_subprocess
  HINTS ${PRAGMA_DEPS_DIR}/chromium_wrapper/bin
)

set(REQ_VARS ${PCK}_RUNTIME ${PCK}_EXECUTABLE)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(${PCK}
  REQUIRED_VARS ${REQ_VARS}
)
