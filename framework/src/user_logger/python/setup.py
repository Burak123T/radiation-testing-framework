# Available at setup time due to pyproject.toml
from pybind11.setup_helpers import Pybind11Extension, build_ext
from setuptools import setup

__version__ = "0.0.1"

# The main interface is through Pybind11Extension.
# * You can add cxx_std=11/14/17, and then build_ext can be removed.
# * You can set include_pybind11=false to add the include directory yourself,
#   say from a submodule.
#
# Note:
#   Sort input source files if you glob sources to ensure bit-for-bit
#   reproducible builds (https://github.com/pybind/python_example/pull/53)

ext_modules = [
    Pybind11Extension(
        "rad_logger",
        ["rad_logger_pybind.cpp"],
        # only needed if the library is in a non-standard location
        # libraries=["rad_logger"], 
        # library_dirs=["/usr/local/lib"],
        # runtime_library_dirs=["/usr/local/lib"],
        # include_dirs=["/usr/local/include"],
        define_macros=[("VERSION_INFO", __version__)],
    ),
]

setup(
    name="rad_logger",
    version=__version__,
    author="Robert Bayer",
    author_email="roba@itu.dk",
    url="https://github.com/Resource-Aware-Data-systems-RAD/radiation-testing-framework",
    description="Python bindings for the uprobe radiation logger",
    long_description="",
    ext_modules=ext_modules,
    # Currently, build_ext only provides an optional "highest supported C++
    # level" feature, but in the future it may provide more features.
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
    python_requires=">=3.7",
)