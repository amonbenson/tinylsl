#!/bin/bash
mkdir -p build
gcc -Wall -Werror -Og -o build/test **/*.c -Iinclude -Itests && ./build/test
