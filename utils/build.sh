#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"
source ./get_to_root.sh

mkdir -p build
cd build
CC=/usr/bin/gcc
CXX=/usr/bin/g++ 
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
cmake --build .
cd ..
ln -sf build/compile_commands.json compile_commands.json
