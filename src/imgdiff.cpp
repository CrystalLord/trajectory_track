#include "imgdiff.hpp"

#include <iostream>
#include <stdexcept>
#include <opencv2/opencv.hpp>


void ImgDiff::diff(cv::Mat& to,
                   cv::Mat& from,
                   cv::Mat& diff,
                   int dilateThresh)
{
    cv::absdiff(to, from, diff);
    if (dilateThresh > 0) {
        // Conduct a dilation to grab any missing elements.
        cv::dilate(diff,
                   diff,
                   cv::getStructuringElement(
                       cv::MORPH_ELLIPSE, cv::Size(dilateThresh, dilateThresh)
                   ));
    }
}

cv::Point ImgDiff::diffThreshCentre(cv::Mat& diff, unsigned char threshold, cv::Mat& output)
{
    unsigned int cols = diff.cols;
    unsigned int rows = diff.rows;
    
    unsigned char channels = diff.channels();
    
    unsigned int pixCount = 0;  // Pixel count
    unsigned int sumX = 0;
    unsigned int sumY = 0;
    
/*    
    // We split up these loops for efficiency.
    if (output == nullptr) {
        // No output image. Just find the centre and that's it.
        for (unsigned int i = 0; i < rows; ++i) {
            // Get the start of the row of interest.
            unsigned char* ptr = diff.ptr<unsigned char>(i);
            for (unsigned int j = 0; j < cols; ++j) {
                for (unsigned int curChan = 0; curChan < channels; ++curChan) {
                    if (*ptr >= threshold) {
                        ++pixCount;
                        sumX += j;
                        sumY += i;
                        ptr += channels - curChan;
                        break;
                    }
                    ++ptr;
                } 
            }
        }
    } else {*/
        if (output.channels() > 1) {
            std::cout << "[ERR] Output had more than one channel." << std::endl;
            throw std::runtime_error("Output had more than one channel.");
        }
        // We have an output image we want to write to now.
        for (unsigned int i = 0; i < rows; ++i) {
            // Get the start of the row of interest.
            unsigned char* ptr = diff.ptr<unsigned char>(i);
            unsigned char* optr = output.ptr<unsigned char>(i);
            for (unsigned int j = 0; j < cols; ++j) {
                *optr = 0;
                for (unsigned int curChan = 0; curChan < channels; ++curChan) {
                    if (*ptr >= threshold) {
                        ++pixCount;
                        sumX += j;
                        sumY += i;
                        *optr = 255;
                        ptr += channels - curChan;
                        break;
                    }
                    ++ptr;
                }
                // We assume the output pointer is only a single channel.
                ++optr;
            }
        }
    
    //}
    
    //std::cout << pixCount << " " << sumX << " " << sumY << std::endl;
    
    // Avoid dividing by zero issue.
    if (pixCount == 0) {
        pixCount = 1;
    }

    cv::Point centre(sumX/pixCount, sumY/pixCount);
    
    /*
    // Create a random access matrix.
    cv::Mat_<cv::Vec3b> _diff (diff);
    
    for (int i = -2; i <= 2; ++i) {
        for (int j = -2; j <= 2; ++j) {
            _diff(centre.y+j, centre.x+i)[0] = 255;
            _diff(centre.y+j, centre.x+i)[1] = 0;
            _diff(centre.y+j, centre.x+i)[2] = 0;
        }
    }
    diff = (cv::Mat) _diff;
    */

    return centre;
}
