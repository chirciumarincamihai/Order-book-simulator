from setuptools import setup, Extension
import pybind11

ext_modules = [
    Extension(
        "orderbook_engine",      
        include_dirs=[pybind11.get_include()],
        language='c++'
    ),
]

setup(
    name="orderbook_engine",
    ext_modules=ext_modules,
)