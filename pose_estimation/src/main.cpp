/*
 * Copyright (c) 2019 Flight Dynamics and Control Lab
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <cstdlib>
#include <iostream>
#include <opencv2/aruco.hpp>
#include <opencv2/opencv.hpp>

namespace {
    const char *about = "Pose estimation of ArUco marker images";
    const char *keys =
        "{d        |16    | dictionary: DICT_4X4_50=0, DICT_4X4_100=1, "
        "DICT_4X4_250=2, DICT_4X4_1000=3, DICT_5X5_50=4, DICT_5X5_100=5, "
        "DICT_5X5_250=6, DICT_5X5_1000=7, DICT_6X6_50=8, DICT_6X6_100=9, "
        "DICT_6X6_250=10, DICT_6X6_1000=11, DICT_7X7_50=12, DICT_7X7_100=13, "
        "DICT_7X7_250=14, DICT_7X7_1000=15, DICT_ARUCO_ORIGINAL = 16}"
        "{h        |false | Print help }"
        "{l        |      | Actual marker length in meter }"
        "{v        |<none>| Custom video source, otherwise '0' }"
        "{h        |false | Print help }"
        "{l        |      | Actual marker length in meter }"
        "{v        |<none>| Custom video source, otherwise '0' }";
}

int main(int argc, char **argv)
{
    cv::CommandLineParser parser(argc, argv, keys);
    parser.about(about);

    if (argc < 2) {
        parser.printMessage();
        return 1;
    }

    if (parser.get<bool>("h")) {
        parser.printMessage();
        return 0;
    }

    int dictionaryId = parser.get<int>("d");
    float marker_length_m = parser.get<float>("l");
    int wait_time = 10;

    if (marker_length_m <= 0) {
        std::cerr << "marker length must be a positive value in meter"
                  << std::endl;
        return 1;
    }

    cv::String videoInput = "0";
    cv::VideoCapture in_video;
    if (parser.has("v")) {
        videoInput = parser.get<cv::String>("v");
        if (videoInput.empty()) {
            parser.printMessage();
            return 1;
        }
        char *end = nullptr;
        int source = static_cast<int>(std::strtol(videoInput.c_str(), &end,
                                                  10));
        if (!end || end == videoInput.c_str()) {
            in_video.open(videoInput); // url
        }
        else {
            in_video.open(source); // id
        }
    }
    else {
        in_video.open(0);
    }

    if (!parser.check()) {
        parser.printErrors();
        return 1;
    }

    if (!in_video.isOpened()) {
        std::cerr << "failed to open video input: " << videoInput << std::endl;
        return 1;
    }

    cv::Mat image, image_copy;
    cv::Mat camera_matrix, dist_coeffs;
    std::ostringstream vector_to_marker;

    cv::Ptr<cv::aruco::Dictionary> dictionary =
        cv::aruco::getPredefinedDictionary(
            cv::aruco::PREDEFINED_DICTIONARY_NAME(dictionaryId));

    cv::FileStorage fs("../../calibration_params.yml", cv::FileStorage::READ);

    fs["camera_matrix"] >> camera_matrix;
    fs["distortion_coefficients"] >> dist_coeffs;

    int fps = 0;
    cv::TickMeter tm;
    tm.start();

    while (in_video.grab()) {
        tm.stop();
        double fps_current = 1000.0 / tm.getTimeMilli();
        fps = (int)fps_current;
        tm.reset();
        tm.start();

        in_video.retrieve(image);
        image.copyTo(image_copy);

        // Resize image to display in a smaller window
        cv::Mat image_small;
        cv::resize(image_copy, image_small, cv::Size(), 0.5, 0.5);
        image_copy = image_small;

        std::vector<int> ids;
        std::vector<std::vector<cv::Point2f>> corners;
        cv::aruco::detectMarkers(image_copy, dictionary, corners, ids);

        // Draw red circle with plus sign in center
        cv::Point center(image_copy.cols / 2, image_copy.rows / 2);
        int radius = std::min(center.x, center.y) / 20;
        cv::Scalar color(0, 0, 255); // Red color
        cv::circle(image_copy, center, radius, color, 2);
        cv::line(image_copy, cv::Point(center.x - radius / 2, center.y), cv::Point(center.x + radius / 2, center.y), color, 2);
        cv::line(image_copy, cv::Point(center.x, center.y - radius / 2), cv::Point(center.x, center.y + radius / 2), color, 2);

        // if at least one marker detected
        if (ids.size() > 0) {
            // cv::aruco::drawDetectedMarkers(image_copy, corners, ids);
            std::vector<cv::Vec3d> rvecs, tvecs;
            cv::aruco::estimatePoseSingleMarkers(corners, marker_length_m,
                                                 camera_matrix, dist_coeffs, rvecs, tvecs);
            for (int i = 0; i < ids.size(); i++) {
                if (ids[i] > 16)
                    continue;

                // Draw axis for each marker
                // cv::aruco::drawAxis(image_copy, camera_matrix, dist_coeffs, rvecs[i], tvecs[i], 0.01);

                // calculate marker distance from camera
                double distance = cv::norm(tvecs[i]);

                // Convert the rotation vector to a rotation matrix
                cv::Mat rotationMatrix;
                cv::Rodrigues(rvecs[i], rotationMatrix);

                // Extract Euler angles from the rotation matrix
                double theta_x = std::atan2(rotationMatrix.at<double>(2, 1), rotationMatrix.at<double>(2, 2));
                double theta_y = std::atan2(-rotationMatrix.at<double>(2, 0), std::sqrt(rotationMatrix.at<double>(2, 1) * rotationMatrix.at<double>(2, 1) + rotationMatrix.at<double>(2, 2) * rotationMatrix.at<double>(2, 2)));
                double theta_z = std::atan2(rotationMatrix.at<double>(1, 0), rotationMatrix.at<double>(0, 0));

                // Convert angles from radians to degrees
                double theta_x_deg = theta_x * 180 / CV_PI;
                double theta_y_deg = theta_y * 180 / CV_PI;
                double theta_z_deg = theta_z * 180 / CV_PI;

                // print marker distance and Euler angles to terminal
                std::cout << "(id, r, x, y, z): " << ids[i] << ", " << distance << ", " << theta_x_deg << ", " << theta_y_deg << ", " << theta_z_deg << std::endl;
                ;

                // print marker distance and Euler angles to screen
                std::ostringstream ss;
                ss << "(id, r, x, y, z): " << ids[i] << ", " << distance << ", " << theta_x_deg << ", " << theta_y_deg << ", " << theta_z_deg;
                cv::putText(image_copy, ss.str(), cv::Point(10, 30 * (i + 1)), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);

                // Draw arrow from center of screen to center of first marker
                cv::Point center(image_copy.cols / 2, image_copy.rows / 2);
                cv::Point marker_center(corners[0][0].x + (corners[0][2].x - corners[0][0].x) / 2, corners[0][0].y + (corners[0][2].y - corners[0][0].y) / 2);
                cv::Scalar color(0, 255, 0); // Green color
                cv::arrowedLine(image_copy, center, marker_center, color, 2);

                break;
            }
        }

        // print FPS on top right corner of screen
        std::ostringstream ss_fps;
        ss_fps << "FPS: " << fps;
        cv::putText(image_copy, ss_fps.str(), cv::Point(image_copy.cols - 100, 30), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);

        imshow("Pose estimation", image_copy);
        char key = (char)cv::waitKey(wait_time);
        if (key == 27)
            break;
    }
}
