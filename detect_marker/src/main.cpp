#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>

int main(int argc, char **argv)
{
    int wait_time = 10;
    cv::VideoCapture in_video;
    in_video.open(0);
    cv::Ptr<cv::aruco::Dictionary> dictionary = 
        cv::aruco::getPredefinedDictionary(cv::aruco::DICT_ARUCO_ORIGINAL);
    
    while (in_video.grab()) 
    {
        cv::Mat image, image_copy;
        in_video.retrieve(image);
        image.copyTo(image_copy);
        std::vector<int> ids;
        std::vector<std::vector<cv::Point2f> > corners;
        cv::aruco::detectMarkers(image, dictionary, corners, ids);
        // if at least one marker detected
        if (ids.size() > 0)
            cv::aruco::drawDetectedMarkers(image_copy, corners, ids);
        
        cv::imshow("Detected markers", image_copy);
        char key = (char) cv::waitKey(wait_time);
        if (key == 27)
            break;
    }

    in_video.release();
}
