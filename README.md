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

You can install OpenCV using the master branch of their repository, *OR* using the submodules added to this repository.
Building and installing OpenCV with the provided submodules guarantees that the other codes on this repository works without issues.
So it is recommended to install from the submodules.

#### Installing v3.4.2 (recommended)
```
git submodule update --init
cd libraries/opencv
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local -DOPENCV_EXTRA_MODULES_PATH=../../opencv_contrib/modules ..
make 
sudo make install
```

#### Installing the Latest
```
git clone https://github.com/opencv/opencv.git
git clone https://github.com/opencv/opencv_contrib.git

# if you need a specific version, you should checkout to that version on 
# both repositories before executing the below commands

mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local -DOPENCV_EXTRA_MODULES_PATH=../../opencv_contrib/modules ..
make 
sudo make install
```

### Generating Markers

