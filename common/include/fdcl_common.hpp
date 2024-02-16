#ifndef __FDCL_COMMON_HPP__
#define __FDCL_COMMON_HPP__

#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <iostream>
#include <cstdlib>

namespace fdcl {
    const char* keys  =
        "{d        |16    | dictionary: DICT_4X4_50=0, DICT_4X4_100=1, "
        "DICT_4X4_250=2, DICT_4X4_1000=3, DICT_5X5_50=4, DICT_5X5_100=5, "
        "DICT_5X5_250=6, DICT_5X5_1000=7, DICT_6X6_50=8, DICT_6X6_100=9, "
        "DICT_6X6_250=10, DICT_6X6_1000=11, DICT_7X7_50=12, DICT_7X7_100=13, "
        "DICT_7X7_250=14, DICT_7X7_1000=15, DICT_ARUCO_ORIGINAL = 16}"
        "{h        |false | Print help }"
        "{v        |<none>| Custom video source, otherwise '0' }"
        "{l        |      | Actual marker length in meter }"
        ;
}

bool parse_inputs(cv::CommandLineParser &parser, const char *about) {
    parser.about(about);

    if (parser.get<bool>("h")) {
        parser.printMessage();
        return false;
    }

    if (!parser.check()) {
        parser.printErrors();
        return false;
    }

    return true;
}

void open_video_from_arg(const cv::String &video_input, \
    cv::VideoCapture &in_video) {

    char* end = nullptr;
    int source = static_cast<int>(std::strtol(video_input.c_str(), &end, 10));
    
    if (!end || end == video_input.c_str()) {
        std::cout << "Trying to open video URL " << video_input << "\n";
        in_video.open(video_input);

    } else {
        std::cout << "Trying to open video ID " << video_input << "\n";
        in_video.open(source);

    }

}

bool parse_video_in(cv::VideoCapture &in_video, const cv::CommandLineParser \
    &parser) {
    cv::String video_input = "0";

    if (parser.has("v")) {
        video_input = parser.get<cv::String>("v");
        if (video_input.empty()) {
            std::cerr << "Video source is required with -v flag\n";
            parser.printMessage();
            return false;
        }

        open_video_from_arg(video_input, in_video);
        
    } else {
        std::cout << "Trying to open camera\n";
        in_video.open(0);
    }

    if (!in_video.isOpened()) {
        std::cerr << "Failed to open video input: " << video_input << "\n";
        return false;
    }

    std::cout << "Video input " << video_input << " successfully opened\n";
    return true;
}

void drawText(cv::InputOutputArray image, const std::string &name, 
    const double value, const cv::Point place)  {
        
    cv::Scalar text_color = cv::Scalar(0, 252, 124);

    std::ostringstream vector_to_marker;
    vector_to_marker.str(std::string());

    vector_to_marker << std::setprecision(4) 
        << name << ": " << std::setw(8) << value;
    cv::putText(image, vector_to_marker.str(), place, cv::FONT_HERSHEY_SIMPLEX, 
        0.6, text_color, 1, CV_AVX);
}

#endif