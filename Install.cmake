pr_install_create_directory(modules/chromium)
pr_install_targets(pr_chromium pr_chromium_wrapper pr_chromium_subprocess INSTALL_DIR "modules/chromium/")

pr_install_directory("${DEPENDENCY_CEF_LOCATION}/Resources/" INSTALL_DIR "modules/chromium")
# Copy everything except .lib files (see https://stackoverflow.com/a/43499846/1879228 )
pr_install_directory("${DEPENDENCY_CEF_LOCATION}/Release/" INSTALL_DIR "modules/chromium" PATTERN "*" PATTERN "*.lib" EXCLUDE)
