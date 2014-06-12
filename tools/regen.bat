@echo off
setlocal
if x%1==x goto bad
set CLPATH=
for %%i in (cl.exe) do set CLPATH="%%~$PATH:i"
if x%CLPATH%==x goto nocl
rmdir /s/q out &^
mkdir out &&^
set CC=cl &&^
set CXX=cl &&^
cd out &&^
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=%1 &&^
cd ..
echo.
echo Run "ninja -C out" to build.
echo out\sg_test.exe for unittests.
echo out\sg.exe for main binary.
goto :EOF

:bad
echo usage: tools\regen Debug^|Release
goto :EOF

:nocl
echo cl.exe not found, must run something like "C:\Program Files (x86)\Microsoft Visual Studio 12.0\vc\vcvarsall.bat" first.
goto :EOF
