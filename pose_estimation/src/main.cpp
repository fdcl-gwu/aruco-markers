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
    const char *about = "Advanced ArUco Marker Tracker";
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

    const cv::Scalar red(0, 0, 255);       // Red color
    const cv::Scalar white(255, 255, 255); // white color
    const cv::Scalar black(0, 0, 0);       // black color
    const cv::Scalar green(0, 255, 0);     // green color
    const cv::Scalar blue(255, 0, 0);      // blue color
    const cv::Scalar yellow(0, 255, 255);  // yellow color
    const int font_style = cv::FONT_HERSHEY_COMPLEX;
    const double font_scale = 0.5;

    // need to adjust these values for each camera
    const double pitch_target = 22.0;
    const double yaw_target = 48.0;
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
        int source = static_cast<int>(std::strtol(videoInput.c_str(), &end, 10));
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

    cv::Mat image, image_small;
    cv::Mat camera_matrix, dist_coeffs;

    cv::Ptr<cv::aruco::Dictionary> dictionary =
        cv::aruco::getPredefinedDictionary(
            cv::aruco::PREDEFINED_DICTIONARY_NAME(dictionaryId));

    cv::FileStorage fs("../../calibration_params.yml", cv::FileStorage::READ);

    fs["camera_matrix"] >> camera_matrix;
    fs["distortion_coefficients"] >> dist_coeffs;

    assert(in_video.grab() != 0 && "Failed to grab frame");
    in_video.retrieve(image);

    int fps = 0;
    cv::TickMeter tm;
    tm.start();

    //  Center of screen and radius of crosshair
    const cv::Point center(image.cols / 2, image.rows / 2);
    const int radius = std::min(center.x, center.y) / 20;

    while (in_video.grab()) {
        // grab frame from video stream
        in_video.retrieve(image);

        // detect markers
        std::vector<int> ids;
        std::vector<std::vector<cv::Point2f>> corners;
        cv::aruco::detectMarkers(image, dictionary, corners, ids);

        // + crosshair
        cv::line(image, cv::Point(center.x - radius / 2, center.y), cv::Point(center.x + radius / 2, center.y), white, 5);
        cv::line(image, cv::Point(center.x, center.y - radius / 2), cv::Point(center.x, center.y + radius / 2), white, 5);
        cv::line(image, cv::Point(center.x - radius / 2, center.y), cv::Point(center.x + radius / 2, center.y), red, 3);
        cv::line(image, cv::Point(center.x, center.y - radius / 2), cv::Point(center.x, center.y + radius / 2), red, 3);

        // print FPS on top right corner of screen
        std::ostringstream ss_fps;
        ss_fps << "FPS: " << fps;
        cv::putText(image, ss_fps.str(), cv::Point(image.cols - 100, 30), font_style, font_scale, black, 5);
        cv::putText(image, ss_fps.str(), cv::Point(image.cols - 100, 30), font_style, font_scale, green, 2);

        // display timestamp on bottom left corner of screen
        auto now = std::chrono::system_clock::now();
        auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        auto now_c = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss_timestamp;
        ss_timestamp << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S") << '.' << std::setfill('0') << std::setw(3) << now_ms.count();
        std::string timestamp = ss_timestamp.str();
        cv::putText(image, timestamp, cv::Point(10, image.rows - 30), font_style, font_scale, black, 5);
        cv::putText(image, timestamp, cv::Point(10, image.rows - 30), font_style, font_scale, green, 2);

        // if at least one marker detected
        if (ids.size() > 0) {
            cv::aruco::drawDetectedMarkers(image, corners, ids, cv::Scalar(0, 0, 255));
            std::vector<cv::Vec3d> rvecs, tvecs;
            cv::aruco::estimatePoseSingleMarkers(corners, marker_length_m, camera_matrix, dist_coeffs, rvecs, tvecs);
            for (int i = 0; i < ids.size(); i++) {
                if (ids[i] > 16)
                    continue;

                // Convert the rotation vector to a rotation matrix
                cv::Mat rotationMatrix;
                cv::Rodrigues(rvecs[i], rotationMatrix);

                // Extract Euler angles from the rotation matrix
                const double theta_x = std::atan2(rotationMatrix.at<double>(2, 1), rotationMatrix.at<double>(2, 2));
                const double theta_y = std::atan2(-rotationMatrix.at<double>(2, 0), std::sqrt(rotationMatrix.at<double>(2, 1) * rotationMatrix.at<double>(2, 1) + rotationMatrix.at<double>(2, 2) * rotationMatrix.at<double>(2, 2)));
                const double theta_z = std::atan2(rotationMatrix.at<double>(1, 0), rotationMatrix.at<double>(0, 0));

                // Convert angles from radians to degrees and round to nearest whole number
                const int theta_x_deg = std::round(theta_x * 180 / CV_PI); // roll (clockwise is positive)
                const int theta_y_deg = std::round(theta_y * 180 / CV_PI); // yaw (right is positive)
                const int theta_z_deg = std::round(theta_z * 180 / CV_PI); // pitch (up is positive)

                // print marker distance and Euler angles to screen
                std::ostringstream ss_orientation;
                ss_orientation << "Roll: " << theta_x_deg << ", Yaw: " << theta_y_deg << ", Pitch: " << theta_z_deg;
                cv::putText(image, ss_orientation.str(), cv::Point(10, 30 * (i + 1)), font_style, font_scale, black, 5);
                cv::putText(image, ss_orientation.str(), cv::Point(10, 30 * (i + 1)), font_style, font_scale, green, 2);

                // Calculate pitch and yaw angles to center marker in frame
                const double marker_x = tvecs[i][0];
                const double marker_y = tvecs[i][1];
                const double marker_z = tvecs[i][2];

                // Calculate marker distance from camera and round to 2 decimal points
                const double distance = cv::norm(tvecs[i]);
                const double pitch = std::asin(marker_y / distance) * 180 / CV_PI - pitch_target;
                const double yaw = yaw_target - std::atan2(marker_x, marker_z) * 180 / CV_PI;

                if (pitch >= 1) {
                    // draw arrow pointing down
                    cv::Point arrow_start(center.x, center.y + radius + 40);
                    cv::Point arrow_end(center.x, center.y + radius + 80);
                    cv::arrowedLine(image, arrow_start, arrow_end, white, 5);
                    cv::arrowedLine(image, arrow_start, arrow_end, red, 3);
                }
                else if (pitch <= -1) {
                    // draw arrow pointing up
                    cv::Point arrow_start(center.x, center.y - radius - 40);
                    cv::Point arrow_end(center.x, center.y - radius - 80);
                    cv::arrowedLine(image, arrow_start, arrow_end, white, 5);
                    cv::arrowedLine(image, arrow_start, arrow_end, red, 3);
                }

                if (yaw <= -1) {
                    // draw arrow pointing right
                    cv::Point arrow_start(center.x + radius + 40, center.y);
                    cv::Point arrow_end(center.x + radius + 80, center.y);
                    cv::arrowedLine(image, arrow_start, arrow_end, white, 5);
                    cv::arrowedLine(image, arrow_start, arrow_end, red, 3);
                }
                else if (yaw >= 1) {
                    // draw arrow pointing left
                    cv::Point arrow_start(center.x - radius - 40, center.y);
                    cv::Point arrow_end(center.x - radius - 80, center.y);
                    cv::arrowedLine(image, arrow_start, arrow_end, white, 5);
                    cv::arrowedLine(image, arrow_start, arrow_end, red, 3);
                }

                // if pitch between -1 and 1, and yaw between -1 and 1, draw crosshair X
                if (pitch > -1 && pitch < 1 && yaw > -1 && yaw < 1) {
                    // x crosshair
                    int const offset = 3;
                    cv::line(image, cv::Point(center.x - radius / 2 - offset, center.y - radius / 2 - offset), cv::Point(center.x - radius / 2 + offset, center.y - radius / 2 + offset), white, 5);
                    cv::line(image, cv::Point(center.x - radius / 2 - offset, center.y + radius / 2 + offset), cv::Point(center.x - radius / 2 + offset, center.y + radius / 2 - offset), white, 5);
                    cv::line(image, cv::Point(center.x + radius / 2 + offset, center.y - radius / 2 - offset), cv::Point(center.x + radius / 2 - offset, center.y - radius / 2 + offset), white, 5);
                    cv::line(image, cv::Point(center.x + radius / 2 + offset, center.y + radius / 2 + offset), cv::Point(center.x + radius / 2 - offset, center.y + radius / 2 - offset), white, 5);

                    cv::line(image, cv::Point(center.x - radius / 2 - offset, center.y - radius / 2 - offset), cv::Point(center.x - radius / 2 + offset, center.y - radius / 2 + offset), red, 3);
                    cv::line(image, cv::Point(center.x - radius / 2 - offset, center.y + radius / 2 + offset), cv::Point(center.x - radius / 2 + offset, center.y + radius / 2 - offset), red, 3);
                    cv::line(image, cv::Point(center.x + radius / 2 + offset, center.y - radius / 2 - offset), cv::Point(center.x + radius / 2 - offset, center.y - radius / 2 + offset), red, 3);
                    cv::line(image, cv::Point(center.x + radius / 2 + offset, center.y + radius / 2 + offset), cv::Point(center.x + radius / 2 - offset, center.y + radius / 2 - offset), red, 3);
                }

                // Print pitch and yaw angles to terminal
                std::cout << "Pitch: " << pitch << " degrees" << std::endl;
                std::cout << "Yaw: " << yaw << " degrees" << std::endl;

                // Draw pitch and yaw angles on image
                std::ostringstream ss_paw_pitch;
                ss_paw_pitch << "Pitch: " << static_cast<int>(pitch) << ", Yaw: " << static_cast<int>(yaw) << "";
                cv::Size text_size = cv::getTextSize(ss_paw_pitch.str(), font_style, font_scale, 1, nullptr);
                cv::Point text_pos(center.x - text_size.width / 2, center.y - radius - text_size.height - 5);
                cv::putText(image, ss_paw_pitch.str(), text_pos, font_style, font_scale, black, 5);
                cv::putText(image, ss_paw_pitch.str(), text_pos, font_style, font_scale, green, 2);

                // Display distance under the crosshair
                std::ostringstream ss_dist;
                ss_dist << std::fixed << std::setprecision(2) << distance << " m";
                text_size = cv::getTextSize(ss_dist.str(), font_style, font_scale, 1, nullptr);
                text_pos = cv::Point(center.x - text_size.width / 2, center.y + radius + text_size.height + 10);
                cv::putText(image, ss_dist.str(), text_pos, font_style, font_scale, black, 5);
                cv::putText(image, ss_dist.str(), text_pos, font_style, font_scale, green, 2);

                // Print marker distance and Euler angles with timestamp to terminal
                std::cout << "[" << timestamp << "] (id, dist, roll, yaw, pitch): " << ids[i] << ", " << distance << ", " << theta_x_deg << ", " << theta_y_deg << ", " << theta_z_deg << std::endl;

                break;
            }
        }
        else {
            // display "Scan for ArUco marker" on screen
            std::ostringstream ss_msg;
            ss_msg << "Scan for ArUco marker . . .";
            cv::Size text_size = cv::getTextSize(ss_msg.str(), font_style, font_scale, 1, nullptr);
            cv::Point text_pos(center.x - text_size.width / 2, center.y - radius - text_size.height - 10);
            cv::putText(image, ss_msg.str(), text_pos, font_style, font_scale, black, 5);
            cv::putText(image, ss_msg.str(), text_pos, font_style, font_scale, green, 2);
        }

        imshow("ArUco Tracking", image);
        char key = (char)cv::waitKey(wait_time);
        if (key == 27)
            break;

        // Calculate FPS
        tm.stop();
        double fps_current = 1000.0 / tm.getTimeMilli();
        fps = (int)fps_current;
        tm.reset();
        tm.start();
    }
}
