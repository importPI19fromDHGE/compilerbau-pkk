#!/bin/bash
#cmake . -D CMAKE_BUILD_TYPE=Release # builds the release version, but doesn't link sdl correctly
cmake . -D CMAKE_BUILD_TYPE=Debug
if [ $? -eq 0 ]; then
  make
else
  echo "cmake error"
fi
