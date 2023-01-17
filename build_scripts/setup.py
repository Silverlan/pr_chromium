import os
from sys import platform
import tarfile
from pathlib import Path

os.chdir(deps_dir)

########## CEF ##########
if platform == "linux":
	suffix = "linux64"
else:
	suffix = "windows64"

cefRoot = deps_dir +"/cef_binary_107.1.9+g1f0a21a+chromium-107.0.5304.110_" +suffix
if not Path(cefRoot).is_dir():
	print_msg("CEF not found. Downloading...")
	tarName = "cef_binary_107.1.9+g1f0a21a+chromium-107.0.5304.110_" +suffix +".tar"
	http_extract("https://cef-builds.spotifycdn.com/cef_binary_107.1.9%2Bg1f0a21a%2Bchromium-107.0.5304.110_" +suffix +".tar.bz2",tarName,"tar.bz2")

print_msg("Building CEF...")
os.chdir(cefRoot)
mkdir("build",cd=True)
cmake_configure("..",generator)
cmake_build("Debug",["libcef_dll_wrapper"])

cmake_args.append("-DDEPENDENCY_CHROMIUM_INCLUDE=" +cefRoot +"")
cmake_args.append("-DDEPENDENCY_CEF_LOCATION=" +cefRoot +"")

if platform == "linux":
	cmake_args.append("-DDEPENDENCY_CHROMIUM_LIBRARY=" +cefRoot +"/Debug/libcef.so")
	if(generator=="Ninja Multi-Config"):
		cmake_args.append("-DDEPENDENCY_LIBCEF_DLL_WRAPPER_LIBRARY=" +cefRoot +"/build/libcef_dll_wrapper/Debug/libcef_dll_wrapper.a")
	else:
		cmake_args.append("-DDEPENDENCY_LIBCEF_DLL_WRAPPER_LIBRARY=" +cefRoot +"/build/libcef_dll_wrapper/libcef_dll_wrapper.a")

else:
	cmake_args.append("-DDEPENDENCY_CHROMIUM_LIBRARY=" +cefRoot +"/Debug/libcef.lib")
	cmake_args.append("-DDEPENDENCY_LIBCEF_DLL_WRAPPER_LIBRARY=" +cefRoot +"/build/libcef_dll_wrapper/Debug/libcef_dll_wrapper.lib")
