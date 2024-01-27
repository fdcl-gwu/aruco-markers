git submodule update --init

cd ./libraries/opencv
mkdir build
cd ./build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local -DOPENCV_EXTRA_MODULES_PATH=../../opencv_contrib/modules ..
make -j4
make install
cd ../../../