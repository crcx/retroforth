#!/usr/bin/env python3
# encoding: utf-8

import setuptools

with open("README", "r", encoding="utf-8") as fh:
    long_description = fh.read()

setuptools.setup(
    name="retroforth",
    version="2021.1",
    author="Charles Childers",
    author_email="crc@forthworks.com",
    description="RetroForth is a modern, pragmatic Forth",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="http://forthworks.com/retro",
    packages=setuptools.find_packages(),
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: ISC License (ISCL)",
        "Operating System :: OS Independent",
    ],
    python_requires='>=3.6',
)
