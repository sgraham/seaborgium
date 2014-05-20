@echo off
rmdir /s/q out &^
mkdir out &&^
mkdir out\Debug &&^
mkdir out\Release &&^
set CC=cl &&^
set CXX=cl &&^
cd out\Debug &&^
cmake ..\.. -G Ninja -DCMAKE_BUILD_TYPE=Debug &&^
cd ..\..\out\Release &&^
cmake ..\.. -G Ninja -DCMAKE_BUILD_TYPE=Release &&^
cd ..\..
