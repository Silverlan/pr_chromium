import os
import sys
import tarfile
from pathlib import Path
from urllib.parse import quote
import shutil
import subprocess
import config

if build_all:
	# Required so we can import relative packages
	sys.path.insert(0, os.path.dirname(__file__))

	os.chdir(deps_dir)

	from third_party import cef
	cef_info = cef.main()

	from third_party import chromium_wrapper
	chromium_wrapper_info = chromium_wrapper.main()
