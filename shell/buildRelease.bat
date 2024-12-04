@echo off
rd /s /q build
git submodule init
git submodule update
cd vcpkg
call bootstrap-vcpkg.bat
cd ..
md build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
cd ..
xcopy Debug\* . /Y