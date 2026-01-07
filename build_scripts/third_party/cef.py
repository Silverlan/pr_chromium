import os, sys
from urllib.parse import quote
from pathlib import Path

sys.path.insert(0, str(Path(os.path.abspath(os.path.dirname(__file__))).parent.parent.parent.parent / "build_scripts"))
from scripts.shared import *

def main():
	build_config_tp = config.build_config_tp
	deps_dir = config.deps_dir
	generator = config.generator
	chdir_mkdir(deps_dir)

	if platform == "linux":
		suffix = "linux64"
	else:
		suffix = "windows64"

	cefVer = "cef_binary_136.1.2+g4ff4593+chromium-136.0.7103.49"
	cefRoot = deps_dir +"/cef"
	if not Path(cefRoot).is_dir():
		print_msg("CEF not found. Downloading...")
		tarName = "cef_binary_"+cefVer+"_" +suffix +".tar"
		http_extract("https://cef-builds.spotifycdn.com/"+quote(cefVer)+"_" +suffix +".tar.bz2",tarName,"tar.bz2")
		dir_name = cefVer+"_" +suffix
		mv(dir_name, "cef")

		if platform == "linux":
			os.chdir(cefRoot+"/Release")
			subprocess.run(["strip","--strip-unneeded","libcef.so"])
			os.chdir(deps_dir)

	if platform == "linux":
		import patch
		patchData = patch.fromfile(str(Path(__file__).resolve().parent)+"/patches/0001-fix-trivially-copyable-cef.patch")
		if patchData:
			print_msg("Patching CEF...")
			patchData.apply(1,cefRoot)
	print_msg("Building CEF...")
	os.chdir(cefRoot)
	mkdir("build",cd=True)
	cmake_configure("..",generator,["-DCMAKE_BUILD_TYPE=Release"])
	cmake_build("Release",["libcef_dll_wrapper"])

	copy_prebuilt_binaries(cefRoot +"/Release", "cef")
	copy_prebuilt_binaries(cefRoot +"/build/libcef_dll_wrapper/Release", "cef")
	copy_prebuilt_directory(cefRoot +"/include", dest_dir=get_library_root_dir("cef") +"/include/include/")
	copy_prebuilt_directory(cefRoot +"/Release/", dest_dir=get_library_root_dir("cef") +"/Resources/")
	copy_prebuilt_directory(cefRoot +"/Resources/", dest_dir=get_library_root_dir("cef") +"/Resources/")
	
	return {
		"buildDir": cefRoot
	}


if __name__ == "__main__":
	main()
