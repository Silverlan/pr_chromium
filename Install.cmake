pr_install_create_directory(modules/chromium)
pr_install_targets(pr_chromium pr_chromium_wrapper pr_chromium_subprocess INSTALL_DIR "modules/chromium/")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/external_libs/pr_chromium_wrapper/cmake/modules")
pr_install_binaries(cef INSTALL_DIR "modules/chromium")

pr_install_directory("${cef_RESOURCE_DIR}/" INSTALL_DIR "modules/chromium")
