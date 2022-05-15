#!/bin/bash
#cmake . -D CMAKE_BUILD_TYPE=Release # builds the release version, but doesn't link sdl correctly
mkdir -p build
cmake -D CMAKE_BUILD_TYPE=Debug -B"$(pwd)/build" -S"$(pwd)"
if [ $? -eq 0 ]; then
  make
  mv compilerbau_pkk build/
else
  echo "cmake error"
fi
