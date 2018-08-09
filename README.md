# ArUco Markers

This repository includes codes that can be used to work with the augmented reality library, [ArUco](https://www.uco.es/investiga/grupos/ava/node/26).

## Contents
1. [Installation](#installation)
    1. [Installing v3.4.2 (recommended)](#installing-v342-recommended)
    2. [Installing the Latest](#installing-the-latest)
2. [Generating Markers](#generating-markers)


### Installation

You can install the standalone ArUco library by downloading the source files which can be found in the above website and building and installing them.
But it is highly recommended to install ArUco library packed in OpenCV library.
The instruction below are for installing OpenCV with ArUco library.

You can install OpenCV using the master branch of their repository, **OR** using the submodules added to this repository.
Building and installing OpenCV with the provided submodules guarantees that the other codes on this repository works without issues.
So it is recommended to install from the submodules.


#### Installing v3.4.2 (recommended)
```
sudo apt-get install build-essential
sudo apt-get install cmake git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev
sudo apt-get install python-dev python-numpy libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev libjasper-dev libdc1394-22-dev

git submodule update --init
cd libraries/opencv
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local -DOPENCV_EXTRA_MODULES_PATH=../../opencv_contrib/modules ..
make -j4  # if you have more cores on your computer, substitute 4 with the number of cores
          # use command "nproc" to find the number fo cores
sudo make install
```


#### Installing the Latest
```
sudo apt-get install build-essential
sudo apt-get install cmake git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev
sudo apt-get install python-dev python-numpy libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev libjasper-dev libdc1394-22-dev

git clone https://github.com/opencv/opencv.git
git clone https://github.com/opencv/opencv_contrib.git

# if you need a specific version, you should checkout to that version on 
# both repositories before executing the below commands

mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local -DOPENCV_EXTRA_MODULES_PATH=../../opencv_contrib/modules ..
make -j4  # if you have more cores on your computer, substitute 4 with the number of cores
          # use command "nproc" to find the number fo cores
sudo make install
```


### Generating Markers
To detect the markers using a camera, first you need to print the markers.
The ArUco library comes with few functions to generate the markers, and they are copied to this repository for the ease of finding them.

```
cd create_markers
mkdir build && cd build
cmake ../
make

# create a single marker
# for details about the parameters, run just ./generate_marker
./generate_marker --b=4 -d=16 --id=108 --ms=400 --si marker.jpg

# create a marker board
# for details about the parameters, run just ./generate_board 
./generate_board --bb=4 -h=2 -w=5 -l=200 -s=50 -d=16 --si board.jpg
```
