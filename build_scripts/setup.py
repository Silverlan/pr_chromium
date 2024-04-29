import os
from sys import platform
import tarfile
from pathlib import Path
from urllib.parse import quote
import shutil
import subprocess


os.chdir(deps_dir)

########## CEF ##########
if platform == "linux":
	suffix = "linux64"
else:
	suffix = "windows64"

cefVer = "cef_binary_120.2.3+g7d89c0c+chromium-120.0.6099.199"
cefRoot = deps_dir +"/"+cefVer+"_" +suffix
if not Path(cefRoot).is_dir():
	print_msg("CEF not found. Downloading...")
	tarName = "cef_binary_"+cefVer+"_" +suffix +".tar"
	http_extract("https://cef-builds.spotifycdn.com/"+quote(cefVer)+"_" +suffix +".tar.bz2",tarName,"tar.bz2")

	if platform == "linux":
		shutil.rmtree(cefRoot+"/Debug")
		os.chdir(cefRoot+"/Release")
		subprocess.run(["strip","--strip-unneeded","libcef.so"])
		os.chdir(deps_dir)
        
    


if platform == "linux":
	import patch

	patchData = patch.fromfile(moduleDir+"/build_scripts/patches/0001-fix-trivially-copyable-cef.patch")
	if patchData:
		print_msg("Patching CEF...")
		patchData.apply(1,cefRoot)
print_msg("Building CEF...")
os.chdir(cefRoot)
mkdir("build",cd=True)
cmake_configure("..",generator,["-DCMAKE_BUILD_TYPE=Release"])
cmake_build("Release",["libcef_dll_wrapper"])

cmake_args.append("-DDEPENDENCY_CHROMIUM_INCLUDE=" +cefRoot +"")
cmake_args.append("-DDEPENDENCY_CEF_LOCATION=" +cefRoot +"")

if platform == "linux":
	cmake_args.append("-DDEPENDENCY_CHROMIUM_LIBRARY=" +cefRoot +"/Release/libcef.so")
	if(generator=="Ninja Multi-Config"):
		cmake_args.append("-DDEPENDENCY_LIBCEF_DLL_WRAPPER_LIBRARY=" +cefRoot +"/build/libcef_dll_wrapper/Release/libcef_dll_wrapper.a")
	else:
		cmake_args.append("-DDEPENDENCY_LIBCEF_DLL_WRAPPER_LIBRARY=" +cefRoot +"/build/libcef_dll_wrapper/libcef_dll_wrapper.a")

else:
	cmake_args.append("-DDEPENDENCY_CHROMIUM_LIBRARY=" +cefRoot +"/Release/libcef.lib")
	cmake_args.append("-DDEPENDENCY_LIBCEF_DLL_WRAPPER_LIBRARY=" +cefRoot +"/build/libcef_dll_wrapper/Release/libcef_dll_wrapper.lib")
