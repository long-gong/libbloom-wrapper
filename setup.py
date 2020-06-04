import os
from setuptools import setup, Extension


bloom_ext = Extension('bloom',
   include_dirs=['.'],
   libraries=['bloom'],
   library_dirs=['./lib'],
   sources=['pybf.cpp']
)


setup(
    name='libbloom-wrapper',
    description='Python wrapping for libbloom, a simple and small bloom filter implementation in C',
    classifiers=[
        'Development Status :: 3 - Alpha',
        'Intended Audience :: Developers',
        'Topic :: Software Development :: Libraries',
        'Programming Language :: C',
        'Programming Language :: C++',
        'Programming Language :: Python :: 2',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.4',
        'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: 3.6',
        'License :: OSI Approved :: MIT License',
        ],
    platforms='unix',
    ext_package='',
    ext_modules=[bloom_ext],
    py_modules=['pybf'],
)
