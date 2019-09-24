import glob
import os

from conans import CMake, ConanFile, tools


class FCLConan(ConanFile):
    name = "fcl"
    version = "0.6.0RC"
    description = "Flexible Collision Library"
    # topics can get used for searches, GitHub topics, Bintray tags etc. Add here keywords about the library
    topics = ("simulation", "physics")
    url = "https://github.com/rhololkeolke/conan-fcl"
    homepage = "https://flexible-collision-library.github.io/"
    author = "Devin Schwab <dschwab@andrew.cmu.edu>"
    license = (
        "BSD-3-Clause"
    )  # Indicates license type of the packaged library; please use SPDX Identifiers https://spdx.org/licenses/
    exports = ["LICENSE.md"]  # Packages the license for the
    exports_sources = ["CMakeLists.txt"]
    # conanfile.py
    generators = "cmake"

    # Options may need to change depending on the packaged library.
    settings = "os", "arch", "compiler", "build_type"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "use_sse": [True, False],
        "use_native_arch": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "use_sse": True,
        "use_native_arch": False,
    }

    # Custom attributes for Bincrafters recipe conventions
    _source_subfolder = "source_subfolder"
    _build_subfolder = "build_subfolder"

    requires = (
        "libccd/2.1@rhololkeolke/stable",
        "octomap/1.9.0@rhololkeolke/stable",
        "eigen/3.3.7@conan/stable",
    )

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def source(self):
        # git = tools.Git(folder="hello")
        # git.clone("https://github.com/conan-io/hello.git")
        git = tools.Git(folder=self._source_subfolder)
        git.clone(
            "https://github.com/flexible-collision-library/fcl.git", branch=self.version
        )
        tools.replace_in_file(
            f"{self._source_subfolder}/CMakeLists.txt",
            "find_package(octomap QUIET)",
            'set(PC_OCTOMAP_VERSION "1.9.0")\n'
            "set(PC_OCTOMAP_INCLUDE_DIRS ${CONAN_INCLUDE_DIRS_OCTOMAP})\n"
            "set(PC_OCTOMAP_LIBRARY_DIRS ${CONAN_LIB_DIRS_OCTOMAP})",
        )
        tools.replace_in_file(
            f"{self._source_subfolder}/CMakeLists.txt",
            'set(OCTOMAP_VERSION "${PC_OCTOMAP_VERSION}"',
            'set(OCTOMAP_VERSION "1.9.0"',
        )

    def _configure_cmake(self):
        cmake = CMake(self)
        cmake.definitions["BUILD_TESTING"] = False
        cmake.definitions["FCL_STATIC_LIBRARY"] = not self.options.shared
        cmake.definitions["CMAKE_POSITION_INDEPENDENT_CODE"] = self.options.fPIC
        cmake.definitions["FCL_USE_X64_SSE"] = self.options.use_sse
        cmake.definitions["FCL_USE_HOST_NATIVE_ARCH"] = self.options.use_native_arch
        cmake.configure(build_folder=self._build_subfolder)
        return cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        self.copy(pattern="LICENSE", dst="licenses", src=self._source_subfolder)
        cmake = self._configure_cmake()
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = tools.collect_libs(self)
