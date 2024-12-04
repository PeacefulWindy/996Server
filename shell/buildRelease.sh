sudo apt install git cmake
git submodule init
git submodule update
cd vcpkg
./bootstrap-vcpkg.sh
cd ..
rm -rf build
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build .
cd ..
