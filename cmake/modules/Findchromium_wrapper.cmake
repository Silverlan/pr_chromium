set(PCK "chromium_wrapper")

if (${PCK}_FOUND)
  return()
endif()

if(WIN32)
  find_file(${PCK}_RUNTIME
    NAMES pr_chromium_wrapper.dll
    HINTS ${PRAGMA_DEPS_DIR}/chromium_wrapper/bin
  )
else()
  find_library(${PCK}_RUNTIME
    NAMES pr_chromium_wrapper
    HINTS ${PRAGMA_DEPS_DIR}/chromium_wrapper/lib
  )
endif()

find_program(${PCK}_EXECUTABLE
  NAMES pr_chromium_subprocess.exe pr_chromium_subprocess
  HINTS ${PRAGMA_DEPS_DIR}/chromium_wrapper/bin
)

set(REQ_VARS ${PCK}_RUNTIME ${PCK}_EXECUTABLE)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(${PCK}
  REQUIRED_VARS ${REQ_VARS}
)
