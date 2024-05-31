# setup.py
from setuptools import setup, Extension
from Cython.Build import cythonize

setup(
    ext_modules=cythonize([
        Extension("memory_analyzer", ["memory_analyzer.pyx"],
                  libraries=["memory_analyzer_pass"])
    ])
)
