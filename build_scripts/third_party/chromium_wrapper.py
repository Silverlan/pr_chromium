import os, sys
from pathlib import Path

sys.path.insert(0, str(Path(os.path.abspath(os.path.dirname(__file__))).parent.parent.parent.parent / "build_scripts"))
from scripts.shared import *

def main():
	build_config_tp = config.build_config_tp
	deps_dir = config.deps_dir
	prebuilt_bin_dir = config.prebuilt_bin_dir
	generator = config.generator

	if platform == "win32":
		# Only msvc is supported for the chromium wrapper
		generator = config.generator_msvc

	chdir_mkdir(deps_dir)

	os.chdir(deps_dir)
	commit_sha = "2ed5f5845c4f1374a9ccecd4d2434badf7ffb0d4"
	chromium_wrapper_root = normalize_path(os.getcwd() +"/chromium_wrapper")
	if not check_repository_commit(chromium_wrapper_root, commit_sha, "chromium_wrapper"):
		if not Path(chromium_wrapper_root).is_dir():
			print_msg("Chromium Wrapper not found. Downloading...")
			git_clone("https://github.com/Silverlan/pr_chromium_wrapper.git", directory="chromium_wrapper")
		os.chdir("chromium_wrapper")
		reset_to_commit(commit_sha)

		# Build
		print_msg("Building chromium wrapper...")
		mkdir("build",cd=True)
		cmake_configure("..",generator,["-DPRAGMA_DEPS_DIR=" +prebuilt_bin_dir])
		cmake_build(build_config_tp)
		os.chdir(deps_dir)

		if platform == "win32":
			exe_name = "pr_chromium_subprocess.exe"
		else:
			exe_name = "pr_chromium_subprocess"

		copy_prebuilt_binaries(chromium_wrapper_root +"/build/" +build_config_tp +"/", "chromium_wrapper")
		src = Path(chromium_wrapper_root) / "build" / "subprocess" / build_config_tp / exe_name
		dst = Path(get_library_bin_dir("chromium_wrapper")) / exe_name
		dst.parent.mkdir(parents=True, exist_ok=True)
		shutil.copy2(src, dst)

	return {
		"buildDir": chromium_wrapper_root
	}

if __name__ == "__main__":
	main()
