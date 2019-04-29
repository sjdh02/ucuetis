@echo off
cd build-win\
cmake .. -Thost=x64 -G"Visual Studio 15 2017 Win64"
msbuild ALL_BUILD.vcxproj
cd ..
