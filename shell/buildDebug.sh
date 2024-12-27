sudo apt install git cmake
git submodule init
git submodule update
cd vcpkg
./bootstrap-vcpkg.sh
cd ..
rm -rf build
mkdir build
cd build
cmake ..
cmake --build .
cd ..
