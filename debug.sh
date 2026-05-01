#!/bin/bash

if [ "$1" == "configure" ]; then
    echo "Configuring the CMake project..."
    cmake --preset x86-debug-linux
elif [ "$1" == "build" ]; then
    echo "Building the CMake project..."
    cmake --build out/build/x86-debug-linux
elif [ "$1" == "clean" ]; then
    echo "Cleaning up and configure CMake Project..."
    rm -rf out/build/x86-debug-linux
    cmake --preset x86-debug-linux
else
    echo "Usage: $0 {configure|build|clean}"
    exit 1
fi