
cd $deps

# CEF
$cefRoot = "$deps/cef_binary_107.1.9+g1f0a21a+chromium-107.0.5304.110_windows64"
if(![System.IO.Directory]::Exists("$cefRoot")){
	echo "CEF not found. Downloading..."
	Invoke-WebRequest https://cef-builds.spotifycdn.com/cef_binary_107.1.9%2Bg1f0a21a%2Bchromium-107.0.5304.110_windows64.tar.bz2 -OutFile cef_binary_107.1.9+g1f0a21a+chromium-107.0.5304.110_windows64.tar.bz2

	# Extract CEF
	echo "Extracting CEF..."
	& $PSScriptRoot/windows/bunzip2.exe $PWD/cef_binary_107.1.9+g1f0a21a+chromium-107.0.5304.110_windows64.tar.bz2
	tar -xvzf "$PWD/cef_binary_107.1.9+g1f0a21a+chromium-107.0.5304.110_windows64.tar"
	rm cef_binary_107.1.9+g1f0a21a+chromium-107.0.5304.110_windows64.tar
	echo "Done!"
}

echo "Building CEF..."
cd "$cefRoot"
if(![System.IO.Directory]::Exists("$PWD/build")){
	mkdir build
}
cd build
cmake .. -G "$generator"
cmake --build "." --config Release
echo "Done!"

$global:cmakeArgs += " -DDEPENDENCY_CHROMIUM_INCLUDE=`"$cefRoot`" -DDEPENDENCY_CHROMIUM_LIBRARY=`"$cefRoot/Release/libcef.lib`" "
$global:cmakeArgs += " -DDEPENDENCY_LIBCEF_DLL_WRAPPER_LIBRARY=`"$cefRoot/build/libcef_dll_wrapper/Release/libcef_dll_wrapper.lib`" "
$global:cmakeArgs += " -DDEPENDENCY_CEF_LOCATION=`"$cefRoot`" "
