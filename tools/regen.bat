@echo off
if x%1==x goto bad
rmdir /s/q out &^
mkdir out &&^
set CC=cl &&^
set CXX=cl &&^
cd out &&^
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=%1 &&^
cd ..
goto :EOF

:bad
echo usage: tools\regen Debug^|Release
