from conan import ConanFile
from conan.tools.files import copy
from conan.tools.layout import basic_layout
import os

class SenkoJSONConan(ConanFile):
    name = "senkojson"
    version = "2.3.0"
    license = "MIT"
    author = "Baran <https://github.com/Baranigsiz>"
    url = "https://github.com/Baranigsiz/SenkoJSON"
    homepage = "https://github.com/Baranigsiz/SenkoJSON"
    description = "Lightning-fast, zero-overhead modern C++17/20 JSON, MessagePack & CBOR library with RFC 9535 JSONPath, RFC 6902 JSON Patch & RFC 7396 Merge Patch."
    topics = ("json", "msgpack", "cbor", "jsonpath", "json-patch", "merge-patch", "header-only", "cpp17", "cpp20")
    package_type = "header-library"
    no_copy_source = True

    def layout(self):
        basic_layout(self)

    def package(self):
        copy(self, "LICENSE", src=self.source_folder, dst=os.path.join(self.package_folder, "licenses"))
        copy(self, "*.hpp", src=os.path.join(self.source_folder, "include"), dst=os.path.join(self.package_folder, "include"))

    def package_info(self):
        self.cpp_info.bindirs = []
        self.cpp_info.libdirs = []
        self.cpp_info.set_property("cmake_file_name", "SenkoJSON")
        self.cpp_info.set_property("cmake_target_name", "senko::senko")
