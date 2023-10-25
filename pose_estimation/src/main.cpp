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

#include "opencv2/core/opengl.hpp"
#include "opencv2/highgui.hpp"
// #include "opencv2/cudaimgproc.hpp"

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
        double scale = 0.8;
        cv::resize(image_copy, image_small, cv::Size(), scale, scale);
        image_copy = image_small;

        std::vector<int> ids;
        std::vector<std::vector<cv::Point2f>> corners;
        cv::aruco::detectMarkers(image_copy, dictionary, corners, ids);

        //  Center of screen
        cv::Point center(image_copy.cols / 2, image_copy.rows / 2);

        // center circle radius
        int radius = std::min(center.x, center.y) / 20;

        // if at least one marker detected
        if (ids.size() > 0) {
            cv::aruco::drawDetectedMarkers(image_copy, corners, ids);
            std::vector<cv::Vec3d> rvecs, tvecs;
            cv::aruco::estimatePoseSingleMarkers(corners, marker_length_m,
                                                 camera_matrix, dist_coeffs, rvecs, tvecs);
            for (int i = 0; i < ids.size(); i++) {
                if (ids[i] > 16)
                    continue;

                // Draw axis for each marker
                // cv::aruco::drawAxis(image_copy, camera_matrix, dist_coeffs, rvecs[i], tvecs[i], 0.01);

                // Calculate marker distance from camera and round to 2 decimal points
                double distance = std::round(cv::norm(tvecs[i]) * 100) / 100.0;

                // Convert the rotation vector to a rotation matrix
                cv::Mat rotationMatrix;
                cv::Rodrigues(rvecs[i], rotationMatrix);

                // Extract Euler angles from the rotation matrix
                double theta_x = std::atan2(rotationMatrix.at<double>(2, 1), rotationMatrix.at<double>(2, 2));
                double theta_y = std::atan2(-rotationMatrix.at<double>(2, 0), std::sqrt(rotationMatrix.at<double>(2, 1) * rotationMatrix.at<double>(2, 1) + rotationMatrix.at<double>(2, 2) * rotationMatrix.at<double>(2, 2)));
                double theta_z = std::atan2(rotationMatrix.at<double>(1, 0), rotationMatrix.at<double>(0, 0));

                // Convert angles from radians to degrees and round to nearest whole number
                int theta_x_deg = std::round(theta_x * 180 / CV_PI); // roll (clockwise is positive)
                int theta_y_deg = std::round(theta_y * 180 / CV_PI); // yaw (right is positive)
                int theta_z_deg = std::round(theta_z * 180 / CV_PI); // pitch (up is positive)

                // Get current time and format as string with milliseconds
                auto now = std::chrono::system_clock::now();
                auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
                auto now_c = std::chrono::system_clock::to_time_t(now);
                std::stringstream timestamp_ss;
                timestamp_ss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S") << '.' << std::setfill('0') << std::setw(3) << now_ms.count();
                std::string timestamp = timestamp_ss.str();

                // print marker distance and Euler angles to screen
                std::ostringstream ss;
                ss << "(id, dist, roll, yaw, pitch): " << ids[i] << ", " << distance << ", " << theta_x_deg << ", " << theta_y_deg << ", " << theta_z_deg;
                cv::putText(image_copy, ss.str(), cv::Point(10, 30 * (i + 1)), cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0, 0, 0), 5);
                cv::putText(image_copy, ss.str(), cv::Point(10, 30 * (i + 1)), cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0, 255, 0), 2);

                // Draw arrow from center of screen to center of first marker
                cv::Point center(image_copy.cols / 2, image_copy.rows / 2);
                cv::Point marker_center(corners[0][0].x + (corners[0][2].x - corners[0][0].x) / 2, corners[0][0].y + (corners[0][2].y - corners[0][0].y) / 2);
                cv::Scalar color(0, 255, 0); // Green color
                cv::arrowedLine(image_copy, center, marker_center, color, 2);

                // Calculate pitch and yaw angles to center marker in frame
                double marker_x = tvecs[i][0];
                double marker_y = tvecs[i][1];
                double marker_z = tvecs[i][2];

                double marker_distance = std::sqrt(marker_x * marker_x + marker_y * marker_y + marker_z * marker_z);
                double pitch_target = 19.0;
                double yaw_target = 42.0;
                double pitch = (std::asin(marker_y / marker_distance) * 180 / CV_PI - pitch_target) * -1; // negative sign to invert pitch
                double yaw = std::atan2(marker_x, marker_z) * 180 / CV_PI - yaw_target;

                // Draw arrow pointing downwards if pitch is negative, else draw arrow pointing upwards
                if (pitch <= -1) {
                    cv::Point arrow_start(center.x, center.y + radius + 40);
                    cv::Point arrow_end(center.x, center.y + radius + 80);
                    cv::arrowedLine(image_copy, arrow_start, arrow_end, cv::Scalar(255, 255, 255), 5);
                    cv::arrowedLine(image_copy, arrow_start, arrow_end, cv::Scalar(0, 0, 255), 2);
                }
                else if (pitch >= 1) {
                    cv::Point arrow_start(center.x, center.y - radius - 40);
                    cv::Point arrow_end(center.x, center.y - radius - 80);
                    cv::arrowedLine(image_copy, arrow_start, arrow_end, cv::Scalar(255, 255, 255), 5);
                    cv::arrowedLine(image_copy, arrow_start, arrow_end, cv::Scalar(0, 0, 255), 2);
                }

                // Draw arrow pointing right if yaw is positive, else draw arrow pointing left
                if (yaw >= 1) {
                    cv::Point arrow_start(center.x + radius + 40, center.y);
                    cv::Point arrow_end(center.x + radius + 80, center.y);
                    cv::arrowedLine(image_copy, arrow_start, arrow_end, cv::Scalar(255, 255, 255), 5);
                    cv::arrowedLine(image_copy, arrow_start, arrow_end, cv::Scalar(0, 0, 255), 2);
                }
                else if (yaw <= -1) {
                    cv::Point arrow_start(center.x - radius - 40, center.y);
                    cv::Point arrow_end(center.x - radius - 80, center.y);
                    cv::arrowedLine(image_copy, arrow_start, arrow_end, cv::Scalar(255, 255, 255), 5);
                    cv::arrowedLine(image_copy, arrow_start, arrow_end, cv::Scalar(0, 0, 255), 2);
                }

                // Print pitch and yaw angles to terminal
                std::cout << "Pitch: " << pitch << " degrees" << std::endl;
                std::cout << "Yaw: " << yaw << " degrees" << std::endl;

                // Draw pitch and yaw angles on image
                std::ostringstream ss_paw_pitch;
                ss_paw_pitch << "Pitch: " << static_cast<int>(pitch) << ", Yaw: " << static_cast<int>(yaw) << "";
                cv::Size text_size = cv::getTextSize(ss_paw_pitch.str(), cv::FONT_HERSHEY_COMPLEX, 0.5, 1, nullptr);
                cv::Point text_pos(center.x - text_size.width / 2, center.y - radius - text_size.height - 10);
                cv::putText(image_copy, ss_paw_pitch.str(), text_pos, cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0, 0, 0), 5);
                cv::putText(image_copy, ss_paw_pitch.str(), text_pos, cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0, 255, 0), 2);

                // Display distance under the crosshair
                std::ostringstream ss_dist;
                ss_dist << std::fixed << std::setprecision(2) << distance << " m";
                text_size = cv::getTextSize(ss_dist.str(), cv::FONT_HERSHEY_COMPLEX, 0.5, 1, nullptr);
                text_pos = cv::Point(center.x - text_size.width / 2, center.y + radius + text_size.height + 10);
                cv::putText(image_copy, ss_dist.str(), text_pos, cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0, 0, 0), 5);
                cv::putText(image_copy, ss_dist.str(), text_pos, cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0, 255, 0), 2);

                // Print marker distance and Euler angles with timestamp to terminal
                std::cout << "[" << timestamp << "] (id, dist, roll, yaw, pitch): " << ids[i] << ", " << distance << ", " << theta_x_deg << ", " << theta_y_deg << ", " << theta_z_deg << std::endl;

                break;
            }
        }
        else {
            // display "Scan for ArUco marker" on screen
            std::ostringstream ss_msg;
            ss_msg << "Scan for ArUco marker . . .";
            cv::Size text_size = cv::getTextSize(ss_msg.str(), cv::FONT_HERSHEY_COMPLEX, 0.5, 1, nullptr);
            cv::Point text_pos(center.x - text_size.width / 2, center.y - radius - text_size.height - 10);
            cv::putText(image_copy, ss_msg.str(), text_pos, cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0, 0, 0), 5);
            cv::putText(image_copy, ss_msg.str(), text_pos, cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0, 255, 0), 2);
        }

        cv::Scalar color(0, 0, 255); // Red color

        // + crosshair
        cv::line(image_copy, cv::Point(center.x - radius / 2, center.y), cv::Point(center.x + radius / 2, center.y), color, 3);
        cv::line(image_copy, cv::Point(center.x, center.y - radius / 2), cv::Point(center.x, center.y + radius / 2), color, 3);

        // x crosshair
        int const offset = 3;
        cv::line(image_copy, cv::Point(center.x - radius / 2 - offset, center.y - radius / 2 - offset), cv::Point(center.x - radius / 2 + offset, center.y - radius / 2 + offset), color, 2);
        cv::line(image_copy, cv::Point(center.x - radius / 2 - offset, center.y + radius / 2 + offset), cv::Point(center.x - radius / 2 + offset, center.y + radius / 2 - offset), color, 2);
        cv::line(image_copy, cv::Point(center.x + radius / 2 + offset, center.y - radius / 2 - offset), cv::Point(center.x + radius / 2 - offset, center.y - radius / 2 + offset), color, 2);
        cv::line(image_copy, cv::Point(center.x + radius / 2 + offset, center.y + radius / 2 + offset), cv::Point(center.x + radius / 2 - offset, center.y + radius / 2 - offset), color, 2);

        // print FPS on top right corner of screen
        std::ostringstream ss_fps;
        ss_fps << "FPS: " << fps;
        cv::putText(image_copy, ss_fps.str(), cv::Point(image_copy.cols - 100, 30), cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0, 0, 0), 5);
        cv::putText(image_copy, ss_fps.str(), cv::Point(image_copy.cols - 100, 30), cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0, 255, 0), 2);

        imshow("ArUco Tracking", image_copy);
        char key = (char)cv::waitKey(wait_time);
        if (key == 27)
            break;
    }
}
