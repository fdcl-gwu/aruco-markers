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