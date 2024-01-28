echo "Starting docker image .."

if [ "$(docker ps -a -q -f name=aruco-markers-container)" ]; 
then
    echo "Previously created container exists, starting it .."
    docker start --interactive aruco-markers-container
else
    echo "No previously created container, creating a new one .."

    docker run \
        --name aruco-markers-container \
        --mount source="$(pwd)",target=/home/aruco-markers,type=bind \
        --net host \
        -v /tmp/.X11-unix:/tmp/.X11-unix \
        -e DISPLAY=unix$DISPLAY \
        --privileged \
        -it aruco-markers bash
fi

echo "Docker run complete"