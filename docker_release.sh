docker build --tag <tag-name>  -t aruco-markers .
docker tag aruco-markers kanishgama/aruco-markers:<tag-name>
docker push kanishgama/aruco-markers:<tag-name>