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

#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/calib3d.hpp>
#include <iostream>
#include <cstdlib>
#include <math.h>
#include "Python.h"

namespace {
const char* about = "Pose estimation of ArUco marker images";
const char* keys  =
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
        "{v        |<none>| Custom video source, otherwise '0' }"
        ;
}

std::vector<cv::Point3f> getToolTipPosition(double tool_width, double tool_length, double tool_depth,
  std::vector<cv::Vec3d> rvec, std::vector<cv::Vec3d> tvec){
     // compute rot_mat
     cv::Mat rot_mat;
     cv::Rodrigues(rvec[0], rot_mat);

     // transpose of rot_mat for easy columns extraction
     cv::Mat rot_mat_t = rot_mat.t();

     // The X orientation effect -> tool width
     double * tmp = rot_mat_t.ptr<double>(0);
     cv::Point3f X_orientation(tmp[0]*tool_width,
                               tmp[1]*tool_width,
                               tmp[2]*tool_width);

     // The Y orientation effect -> tool length
     tmp = rot_mat_t.ptr<double>(1);
     cv::Point3f Y_orientation(tmp[0]*tool_length,
                               tmp[1]*tool_length,
                               tmp[2]*tool_length);

     // The Z orientation effect -> tool depth
     tmp = rot_mat_t.ptr<double>(2);
     cv::Point3f Z_orientation(tmp[0]*tool_depth,
                               tmp[1]*tool_depth,
                               tmp[2]*tool_depth);

     // convert tvec to point
     cv::Point3f tvec_3f(tvec[0][0], tvec[0][1], tvec[0][2]);

     // return vector:
     std::vector<cv::Point3f> ret(1,tvec_3f);

     ret[0] = ret[0] + X_orientation - Y_orientation + Z_orientation;

     return ret;
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

    int resolution_width = 800;
    int resolution_height = 600;
    int dictionaryId = parser.get<int>("d");
    float marker_length_m = parser.get<float>("l");
    int wait_time = 10;
    Py_Initialize();

    if (marker_length_m <= 0) {
        std::cerr << "marker length must be a positive value in meter"
                  << std::endl;
        return 1;
    }

    cv::String videoInput = "0";
    cv::VideoCapture in_video;

    if (parser.has("v")) { //Used to feed in video. We will not use this option.
        videoInput = parser.get<cv::String>("v");
        if (videoInput.empty()) {
            parser.printMessage();
            return 1;
        }
        char* end = nullptr;
        int source = static_cast<int>(std::strtol(videoInput.c_str(), &end, \
            10));
        if (!end || end == videoInput.c_str()) {
            in_video.open(videoInput); // url
        } else {
            in_video.open(source); // id
        }
    } else { //Used for webcam detection. This is what we will use.
        in_video.open(0);
        //Set parameters for video capture!
        //Experiment with these values to get the results that you need.
        
        //UNCOMMENT THE FOLLOWING LINES AND FIND A WIDTH/HEIGHT SUITABLE FOR YOUR CAMERA
       // in_video.set(3, resolution_width); //Set width
        //in_video.set(4, resolution_height); //Set Height
    }

    if (!parser.check()) {
        parser.printErrors();
        return 1;
    }

    if (!in_video.isOpened()) {
        std::cerr << "failed to open video input: " << videoInput << std::endl;
        return 1;
    }

    double rads_to_degress = 180.0 / M_PI;
    const double tool_width = 35.0 / 2000.0;
    const double tool_length = 242.56 / 1000.0;
    const double tool_depth = 9.0 / 1000.0;
    std::vector<cv::Point3f> tool_tip_position;

    cv::Mat image, image_copy;
    cv::Mat camera_matrix, dist_coeffs;
    std::ostringstream vector_to_marker;

    cv::Ptr<cv::aruco::Dictionary> dictionary =
        cv::aruco::getPredefinedDictionary( \
        cv::aruco::PREDEFINED_DICTIONARY_NAME(dictionaryId));

    cv::FileStorage fs("../../calibration_params.yml", cv::FileStorage::READ);

    fs["camera_matrix"] >> camera_matrix;
    fs["distortion_coefficients"] >> dist_coeffs;

    std::cout << "camera_matrix\n" << camera_matrix << std::endl;
    std::cout << "\ndist coeffs\n" << dist_coeffs << std::endl;
    
    //socket connection
    FILE* PScriptFile = fopen("bluetooth_test.py", "r");
	   if(PScriptFile){
		PyRun_SimpleFile(PScriptFile, "bluetooth_test.py");
		fclose(PScriptFile);
	    }

	    //Run a python function
	    PyObject *pName, *pModule, *pFunc, *pArgs, *pValue;

	    pName = PyUnicode_FromString((char*)"script");
	    pModule = PyImport_Import(pName);
	    pFunc = PyObject_GetAttrString(pModule, (char*)"socket_setup");

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
            cv::aruco::estimatePoseSingleMarkers(corners, marker_length_m,
                    camera_matrix, dist_coeffs, rvecs, tvecs);

            tool_tip_position = getToolTipPosition(tool_width, tool_length, tool_depth, rvecs, tvecs);

            //std::cout << "TRANSLATION VECTORS\n" << tvecs[0] << std::endl; //0th element correspond with xyz
            //std::cout << "ROTATIONAL VECTORS\n" << rvecs[0] << std::endl;
            
            std::cout << tool_tip_position[0] << std::endl; 

	    pFunc = PyObject_GetAttrString(pModule, (char*)"main");
	    pArgs = PyTuple_Pack(1, PyUnicode_FromString((char*)tool_tip_position[0])); 
	    //^^^need to check type for tool_tip_position[0]

            // Draw axis for each marker
            for(int i=0; i < ids.size(); i++)
            {
                cv::aruco::drawAxis(image_copy, camera_matrix, dist_coeffs,
                        rvecs[i], tvecs[i], 0.1);

                // This section is going to print the data for all the detected
                // markers. If you have more than a single marker, it is
                // recommended to change the below section so that either you
                // only print the data for a specific marker, or you print the
                // data for each marker separately.

                vector_to_marker.str(std::string());
                vector_to_marker << std::setprecision(4)
                                 << "X, Y, Z: " << std::setw(8) << tool_tip_position[0];
                cv::putText(image_copy, vector_to_marker.str(),
                            cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.7,
                            cv::Scalar(0, 252, 124), 1);

                //TO DO. Convert rotation matrix to euler angles and print them where rvecs_degrees exist.
                // vector_to_marker.str(std::string());
                // vector_to_marker << std::setprecision(4)
                //                  << "x-Angle: " << std::setw(8) << rvecs_degrees[0](0);
                // cv::putText(image_copy, vector_to_marker.str(),
                //             cv::Point(10, 90), cv::FONT_HERSHEY_SIMPLEX, 0.7,
                //             cv::Scalar(0, 252, 124), 1);
                //
                // vector_to_marker.str(std::string());
                // vector_to_marker << std::setprecision(4)
                //                  << "y-Angle: " << std::setw(8) << rvecs_degrees[0](1);
                // cv::putText(image_copy, vector_to_marker.str(),
                //             cv::Point(10, 110), cv::FONT_HERSHEY_SIMPLEX, 0.7,
                //             cv::Scalar(0, 252, 124), 1);
                //
                // vector_to_marker.str(std::string());
                // vector_to_marker << std::setprecision(4)
                //                  << "z-Angle: " << std::setw(8) << rvecs_degrees[0](2);
                // cv::putText(image_copy, vector_to_marker.str(),
                //             cv::Point(10, 130), cv::FONT_HERSHEY_SIMPLEX, 0.7,
                //             cv::Scalar(0, 252, 124), 1);
            }

            // rvecs_degrees.erase(rvecs_degrees.begin(),rvecs_degrees.begin()+3);
        }

        imshow("Pose estimation", image_copy);
        char key = (char)cv::waitKey(wait_time);
        if (key == 27){
          break;
        }
    }
    //Close the python instance
    Py_Finalize(); 

    in_video.release();

    return 0;
}
