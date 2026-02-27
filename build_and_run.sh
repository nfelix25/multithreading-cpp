#! /bin/bash
# Build and run cpp file passed in as arg named file
verbose=1

cpp_file=$1

g++ "$cpp_file"
if [ $? -eq 0 ]; then
    ./a.out
fi
