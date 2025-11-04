#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"
source ./get_to_root.sh

GDB_DEMO_DIR="gdb_demo"
prog=./build/bin/gdb_demo

echo $prog

gdb -q -x ./$GDB_DEMO_DIR/gdb_in -ex "set logging on" $prog > ./$GDB_DEMO_DIR/gdb_out
 