# ArUco Markers

:bangbang: **IMPORTANT**: 
This assumes you are using OpenCV4.
If you need to use OpenCV3, please checkout the `cv3` branch before installing dependencies.
As there are a few breaking changes in OpenCV libraries, some parts of the OpenCV4 codes are not compatible with OpenCV3.


<center>
  <img src="./images/detected_cube.gif"  width="400"/>
</center>

This repository includes codes that can be used to work with the augmented reality library, [ArUco](https://www.uco.es/investiga/grupos/ava/node/26).
A few programs in the repository, including the codes to create the markers and to calibrate the cameras, are copies of the examples included with the OpenCV libraries with minor changes.
Those are added here so that everything is in one place.

## Contents
1. [Installing OpenCV](#installing-opencv)
    1. [Installing v4.5.3 (recommended)](#installing-v453-recommended)
    1. [Installing the Latest](#installing-the-latest)
    1. [Docker Build](#docker-build)
2. [Generating Markers](#generating-markers)
3. [Detecting the Markers](#detecting-the-markers)
4. [Camera Calibration](#camera-calibration)
5. [Pose Estimation](#pose-estimation)
6. [Draw a Cube](#draw-a-cube)


## Installing OpenCV

You can install the standalone ArUco library by downloading the source files which can be found in the above website and building and installing them.
But it is highly recommended to install ArUco library packed in OpenCV library.
The instruction below are for installing OpenCV with ArUco library.
These have been verified to work with Ubuntu 20.04.

If you are on a different OS, and/or prefer Docker, a dockerfile is included with this.
Please skip to the [Docker Build](#docker-build) section.

You can install OpenCV using the master branch of their repository, **OR** using the submodules added to this repository.
Building and installing OpenCV with the provided submodules guarantees that the other codes on this repository work without issues.
So it is recommended to install from the submodules.


### Installing v4.5.3 (recommended)
```
sudo apt install build-essential
sudo apt install cmake git libgtk2.0-dev pkg-config

# Image support
sudo apt install -y libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev

# Video support
sudo apt install -y libavcodec-dev libavformat-dev libswscale-dev

cd <any directory you want to use>
git clone https://github.com/fdcl-gwu/aruco-markers.git
cd aruco-markers
git submodule update --init
cd libraries/opencv
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local -DOPENCV_EXTRA_MODULES_PATH=../../opencv_contrib/modules ..
make -j4  # if you have more/less cores on your computer, substitute 4 with the number of cores
          # use command "nproc" to find the number of cores
sudo make install
```


### Installing the Latest
```
sudo apt install build-essential
sudo apt install cmake git libgtk2.0-dev pkg-config

# Image support
sudo apt install -y libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev

# Video support
sudo apt install -y libavcodec-dev libavformat-dev libswscale-dev

git clone https://github.com/opencv/opencv.git
git clone https://github.com/opencv/opencv_contrib.git

# If you need a specific version, you should checkout that version on 
# both the repositories before executing the below commands.

mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local -DOPENCV_EXTRA_MODULES_PATH=../../opencv_contrib/modules ..
make -j4  # if you have more/less cores on your computer, substitute 4 with the number of cores
          # use command "nproc" to find the number of cores
sudo make install
```


### Docker Build

Following instructions are for a Linux host. 
Though Docker should work for any OS, the GUI setup used here is only has been tested on a Linux host.
If you are on Windows or Mac, you will need to install an X Server, for example [Xming](https://sourceforge.net/projects/xming/), to get the GUI to work.

You have two options here:
1. Pulling Docker image from hub - easy and fast
2. Create the Docker image from scratch - slow, but you get more transparency

#### Pulling Docker Image
1. Install [Docker](https://www.docker.com/)
1. Open a terminal and run
    ```sh
    # Pull the Docker image
    docker pull kanishgama/aruco-markers:opencv-4.5.3

    # Enable xhost - required for GUI
    xhost +
    
    # Replace "-it aruco-markers bash" with "-it kanishgama/aruco-markers:opencv-4.5.3", then start a container
    bash docker_start.sh
    ```

#### Create Docker Image Manually
1. Install [Docker](https://www.docker.com/)
1. Open a terminal and run:
    ```sh
    cd <any directory you want to use>
    git clone https://github.com/fdcl-gwu/aruco-markers.git
    cd aruco-markers

    # Enable xhost - required for GUI
    xhost +

    # Build the docker image
    bash docker_build.sh

    # Run a container 
    bash docker_start.sh

    # After starting the container, you should be inside the docker.
    # Run the OpenCV install script (only required to do the first time).
    # This builds opencv, and install it. The process can take a considerable
    # amount of time depending on your computer.
    cd aruco-markers
    bash docker_opencv_setup.sh

    # After running the OpenCV install script, for any subsequent run, you only
    # have to run the docker run bash docker_start.sh script.
    ```

1. Follow below code compiling instructions.

## Generating Markers
To detect the markers using a camera, first you need to print the markers.
The ArUco library comes with few functions to generate the markers, and they are copied to this repository for the ease of finding them.

```
cd create_markers
mkdir build && cd build
cmake ../
make

# Create a single marker.
# For details about the parameters, run just ./generate_marker
./generate_marker --b=1 -d=16 --id=108 --ms=400 --si marker.jpg

# Create a marker board.
# For details about the parameters, run just ./generate_board
./generate_board --bb=1 -h=2 -w=4 -l=200 -s=100 -d=16 --si board.jpg
```

The generated marker should look like this:
<center>
  <img src="./images/marker.jpg"  width="150"/> 
</center>

The generated board should look like this:
<center>
  <img src="./images/board.jpg"  width="350"/>
</center>


## Detecting the Markers
First, print the [generated markers](#generating-markers).
Connect a camera to the computer and run below commands:
```
cd detect_markers
mkdir build && cd build
cmake ../
make

./detect_markers
```

All the detected markers would be drawn on the image.
<center>
  <img src="./images/detected_markers.png"  width="350"/>
</center>


## Camera Calibration
To accurately detect markers or to get accurate pose data, a camera calibration needs to be performed.

Run below commands to perform the camera calibration:
```
cd camera_calibration
mkdir build && cd build
cmake ../
make

# Below command is accurate only if you used the same parameters when you generated the markers.
# If you changed any of them, change below arguments accordingly.
./camera_calibration -d=16 -dp=../detector_params.yml -h=2 -w=4 -l=<side length of a single marker (in meters)> -s=<separation between two consecutive markers in the grid (in meters)> ../../calibration_params.yml

# If you want to calibrate with an already saved video, use `-v` flag.
./camera_calibration -v=/path/to/your/video.avi -d=16 -dp=../detector_params.yml -h=2 -w=4 -l=<side length of a single marker (in meters)> -s=<separation between two consecutive markers in the grid (in meters)> ../../calibration_params.yml
```

Then point the camera at the marker at different orientations and at different angles, and save those images by pressing key `C`. 
These instructions should appear on the screen.
Around 30 images should be good enough.


## Pose Estimation
To estimate the translation and the rotation of the ArUco marker, run below code:
```
cd pose_estimation

mkdir build && cd build
cmake ../
make

./pose_estimation -l=<side length of a single marker (in meters)>

# or, if you are trying this on an already saved video
./pose_estimation -l=<side length of a single marker (in meters)> -v=<path to the video>
```

Below image shows the output of this code. 
The distances shown in the left top corner are in meters with axes as same as those defined in OpenCV model, i.e., `x`-axis increases from left to right of the image, `y`-axis increases from top to bottom of the image, and the `z`-axis points outwards the camera, with the origin on the top left corner of the image.
The axes drawn on the markers represent the orientation of the marker with the Red-Green-Blue axes order.
<center>
  <img src="./images/pose_estimation.png"  width="300"/>
</center>


## Draw a Cube 
To estimate pose and draw a cube over the ArUco marker, run below code:
```
cd draw_cube

mkdir build && cd build
cmake ../
make

./draw_cube -l=<side length of a single marker (in meters)>

# or, if you are trying this on an already saved video
./draw_cube -l=<side length of a single marker (in meters)> -v=<path to the video>
```

Below GIF shows the output of this code.

<center>
  <img src="./images/detected_cube.gif"  width="350"/>
</center>
