#pragma once

#include <opencv2/opencv.hpp>
#include <string>

class Recorder
{
public:
    int record(cv::VideoCapture& vidCap, float duration, std::string outputPath);
};
