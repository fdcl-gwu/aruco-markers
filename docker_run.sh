docker run \
    --rm \
    --mount source="$(pwd)",target=/home/aruco-markers,type=bind \
    --net host \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    -e DISPLAY=unix$DISPLAY \
    --privileged \
    -it aruco-markers bash