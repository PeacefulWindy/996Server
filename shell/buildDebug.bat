@echo off
git submodule init
git submodule update
cd vcpkg
call bootstrap-vcpkg.bat
cd ..
rd /s /q build
md build
cd build
cmake ..
cmake --build .
cd ..
xcopy Debug\* . /Y
