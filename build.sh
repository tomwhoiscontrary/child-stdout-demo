#! /bin/bash -eu

rm -rf build
mkdir build

gcc simple_echo.c -o build/simple_echo
gcc simple_app.c -o build/simple_app
gcc simple_manager.c -o build/simple_manager
