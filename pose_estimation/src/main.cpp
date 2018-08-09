#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>


int main(int argc, char **argv)
{
    int wait_time = 10;
    cv::Mat image, image_copy;
    cv::Mat camera_matrix, dist_coeffs;
    
    cv::VideoCapture in_video;
    in_video.open(0);
    cv::Ptr<cv::aruco::Dictionary> dictionary = 
        cv::aruco::getPredefinedDictionary(cv::aruco::DICT_ARUCO_ORIGINAL);
    
    cv::FileStorage fs("../../calibration_params.yml", cv::FileStorage::READ);

    fs["camera_matrix"] >> camera_matrix;
    fs["distortion_coefficients"] >> dist_coeffs;

    std::cout << "camera_matrix\n" << camera_matrix << std::endl;
    std::cout << "\ndist coeffs\n" << dist_coeffs << std::endl;

    while (in_video.grab()) 
    {

        in_video.retrieve(image);
        image.copyTo(image_copy);
        std::vector<int> ids;
        std::vector<std::vector<cv::Point2f> > corners;
        cv::aruco::detectMarkers(image, dictionary, corners, ids);
        
        // if at least one marker detected
        if (ids.size() > 0)
        {
            cv::aruco::drawDetectedMarkers(image_copy, corners, ids);
            std::vector<cv::Vec3d> rvecs, tvecs;
            cv::aruco::estimatePoseSingleMarkers(corners, 0.04, camera_matrix,
                    dist_coeffs, rvecs, tvecs);
            // draw axis for each marker
            for(int i=0; i < ids.size(); i++)
                cv::aruco::drawAxis(image_copy, camera_matrix, dist_coeffs,
                        rvecs[i], tvecs[i], 0.1);
        }
        
        cv::imshow("Pose estimation", image_copy);
        char key = (char) cv::waitKey(wait_time);
        if (key == 27)
            break;
    }

    in_video.release();
}
