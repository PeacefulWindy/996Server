sudo apt install git cmake -y
#icu require
sudo apt-get install autoconf automake autoconf-archive -y

git submodule init
git submodule update
cd vcpkg
./bootstrap-vcpkg.sh
cd ..
rm -rf build
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
cd ..
