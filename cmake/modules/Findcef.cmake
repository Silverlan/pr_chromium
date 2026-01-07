set(PCK "cef")

if (${PCK}_FOUND)
  return()
endif()

find_path(${PCK}_INCLUDE_DIR
  NAMES include/cef_config.h
  HINTS
    ${PRAGMA_DEPS_DIR}/cef/include
)

find_path(${PCK}_RESOURCE_DIR
  NAMES resources.pak
  HINTS
    ${PRAGMA_DEPS_DIR}/cef/Resources
)

find_library(${PCK}_LIBRARY
  NAMES cef
  HINTS
    ${PRAGMA_DEPS_DIR}/cef/lib
)

find_library(${PCK}_DLL_WRAPPER_LIBRARY
  NAMES cef_dll_wrapper
  HINTS
    ${PRAGMA_DEPS_DIR}/cef/lib
)

set(REQ_VARS ${PCK}_LIBRARY ${PCK}_DLL_WRAPPER_LIBRARY ${PCK}_INCLUDE_DIR ${PCK}_RESOURCE_DIR)
if(WIN32)
  set(rt_files
    "chrome_elf.dll"
    "d3dcompiler_47.dll"
    "dxcompiler.dll"
    "dxil.dll"
    "libcef.dll"
    "libEGL.dll"
    "libGLESv2.dll"
    "vk_swiftshader.dll"
    "vulkan-1.dll"
  )
  set(RUNTIME_VARS)
  foreach(RT_FILE ${rt_files})
    # 1) sanitize file name into a valid CMake identifier:
    #    chrome_elf.dll  → chrome_elf_dll
    string(REPLACE "." "_" safe_name "${RT_FILE}")

    # 2) build a unique variable name, e.g. CEF_RT_chrome_elf_dll
    set(var_name "${PCK}_RT_${safe_name}")
    find_file(
      ${var_name}
      NAMES ${RT_FILE}
      HINTS
        ${PRAGMA_DEPS_DIR}/cef/bin
    )
    set(REQ_VARS ${REQ_VARS} ${var_name})
    set(RUNTIME_VARS ${RUNTIME_VARS} ${${var_name}})
  endforeach()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(${PCK}
  REQUIRED_VARS ${REQ_VARS}
)

if(${PCK}_FOUND)
  set(${PCK}_LIBRARIES   ${${PCK}_LIBRARY} ${${PCK}_DLL_WRAPPER_LIBRARY})
  set(${PCK}_INCLUDE_DIRS ${${PCK}_INCLUDE_DIR})
  if(WIN32)
    set(${PCK}_RUNTIME   ${RUNTIME_VARS})
  endif()
endif()
