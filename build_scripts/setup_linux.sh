
cd $deps

# CEF
cefRoot="$deps/cef_binary_107.1.9+g1f0a21a+chromium-107.0.5304.110_linux64"
if [ ! -d "$cefRoot" ]; then
	echo "CEF not found. Downloading..."
	wget "https://cef-builds.spotifycdn.com/cef_binary_107.1.9%2Bg1f0a21a%2Bchromium-107.0.5304.110_linux64.tar.bz2"

	# Extract CEF
	echo "Extracting CEF..."
	tar -xvf "$PWD/cef_binary_107.1.9+g1f0a21a+chromium-107.0.5304.110_linux64.tar.bz2"
	rm cef_binary_107.1.9+g1f0a21a+chromium-107.0.5304.110_linux64.tar.bz2
	echo "Done!"
fi

echo "Building CEF..."
cd "$cefRoot"
if [ ! -d "$PWD/build" ]; then
	mkdir build
fi
cd build
cmake .. -G "$generator"
cmake --build "." --config Release --target libcef_dll_wrapper
echo "Done!"

cmakeArgs=" $cmakeArgs -DDEPENDENCY_CHROMIUM_INCLUDE=\"$cefRoot\" -DDEPENDENCY_CHROMIUM_LIBRARY=\"$cefRoot/Release/libcef.so\" "
cmakeArgs=" $cmakeArgs -DDEPENDENCY_LIBCEF_DLL_WRAPPER_LIBRARY=\"$cefRoot/build/libcef_dll_wrapper/libcef_dll_wrapper.a\" "
cmakeArgs=" $cmakeArgs -DDEPENDENCY_CEF_LOCATION=\"$cefRoot\" "
