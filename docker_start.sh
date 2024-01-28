# docker start --interactive aruco-markers-container

echo "Starting docker image .."

if [ "$(docker ps -a -q -f name=aruco-markers-container)" ]; 
then
    echo "Previously created container exists, starting it .."
    docker start --interactive aruco-markers-container
else
    echo "No previously created container, creating a new one .."
    sh ./docker_create_container.sh
fi

echo "Docker run complete"