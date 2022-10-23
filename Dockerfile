FROM ubuntu:20.04
RUN apt update

# Need to do this to avoid docker hanging while installing cmake.
ENV TZ=America/New_York
RUN ln -snf "/usr/share/zoneinfo/$TZ" /etc/localtime
RUN echo "$TZ" > /etc/timezone
RUN apt install -y tzdata

RUN apt install -y build-essential cmake git libgtk2.0-dev pkg-config
WORKDIR /home
RUN git clone https://github.com/fdcl-gwu/aruco-markers.git
WORKDIR /home/aruco-markers
RUN git submodule update --init
WORKDIR /home/aruco-markers/libraries/opencv
RUN mkdir build
WORKDIR /home/aruco-markers/libraries/opencv/build
RUN cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local -DOPENCV_EXTRA_MODULES_PATH=../../opencv_contrib/modules ..
RUN make -j2
RUN make install
WORKDIR /home/aruco-markers
ENV DISPLAY=host.docker.internal:0.0