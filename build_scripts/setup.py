import os
import sys
import tarfile
import argparse
from pathlib import Path
from urllib.parse import quote
import shutil
import subprocess
import config

parser = argparse.ArgumentParser(description='pr_chromium build script', allow_abbrev=False, formatter_class=argparse.ArgumentDefaultsHelpFormatter, epilog="")
parser.add_argument("--build-chromium-wrapper", type=str2bool, nargs='?', const=True, default=False, help="Build the chromium wrapper library (otherwise uses pre-built binaries).")
args,unknown = parser.parse_known_args()
args = vars(args)

build_chromium_wrapper = build_all or args["build_chromium_wrapper"]
if build_chromium_wrapper:
	# Required so we can import relative packages
	sys.path.insert(0, os.path.dirname(__file__))

	os.chdir(deps_dir)

	from third_party import cef
	cef_info = cef.main()

	from third_party import chromium_wrapper
	chromium_wrapper_info = chromium_wrapper.main()
else:
	print_msg("Downloading prebuilt cycles binaries...")
	subprocess.run(["cmake", "-DCMAKE_SOURCE_DIR=" +config.pragma_root, "-DPRAGMA_DEPS_DIR=" +config.prebuilt_bin_dir, "-P", "cmake/fetch_prebuilt_binaries.cmake"],check=True)
