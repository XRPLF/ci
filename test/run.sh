#!/bin/bash

set -ex

# Copy the test code, as we do not want to modify the directory.
cp -r /tmp test
cd test

# Install Conan dependencies and build the project.
conan install . --output-folder=build --build=missing
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build .

# Run the compressor executable.
./compressor
