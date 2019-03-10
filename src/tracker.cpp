#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <chrono>

#include "inputhandler.hpp"
#include "imagereduce.hpp"
#include "tracker.hpp"
#include "imgdiff.hpp"

const int TR_NEWWIDTH = 128*2;

static void help(char* progName)
{
    std::cout << std::endl << "Sharpens an image based using a kernel"
        << std::endl;
}

int tr::trackerProc(cv::VideoCapture& vidCap, int frameStart)
{
    cv::Ptr<cv::Tracker> tracker;
    cv::Mat frame;
    vidCap.read(frame); // Get the first frame.
    cv::Mat shrinkFrame;
    cv::Mat greysc;
    cv::VideoWriter output;
    
    // Calculate the size of the rescale.
    float aspectRatio = ((float) frame.cols)/((float) frame.rows);
    cv::Size newSize = cv::Size(TR_NEWWIDTH,
            (int) TR_NEWWIDTH/aspectRatio);
    output.open("YAY.avi", cv::VideoWriter::fourcc('M', 'J', 'P', 'G'),
                40, newSize, true);

    
    // Get the first frame.
    tr::updateMatrices(vidCap, frame, shrinkFrame, greysc, newSize);
    
    
    // Capture frame-by-frame
    for (int i = 1; i < frameStart; ++i) {
        tr::updateMatrices(vidCap, frame, shrinkFrame, greysc, newSize);
    }
    cv::Rect2d bbox(35*2, 3*2, 70*2, 20*2);  // Create bounding box.
    // Display the bounding box.
    cv::rectangle(shrinkFrame, bbox, cv::Scalar(255, 0, 0), 1, 1);
    
    // Create the tracker.
    tracker = cv::TrackerMOSSE::create();
    tracker->init(greysc, bbox);

    while(true) {
        tr::updateMatrices(vidCap, frame, shrinkFrame, greysc, newSize);
        cv::rectangle(shrinkFrame, bbox, cv::Scalar(255, 0, 0), 1, 1);
  
        // If the frame is empty, break immediately
        if (frame.empty()) {
            break;
        }
        
        bool trackOkay = tracker->update(greysc, bbox);
        if (trackOkay) {
            cv::rectangle(shrinkFrame, bbox, cv::Scalar(255, 0, 0), 1, 1);
        } else {
            std::cout << "Failing to track" << std::endl;
        }

        // Display the resulting frame
        cv::imshow("Frame", shrinkFrame);
        output.write(shrinkFrame);
 
        // Press  ESC on keyboard to exit
        char c = (char) cv::waitKey(25);
        if (c == 27) {
            break;
        }
    }
    vidCap.release();
    output.release();
    return 0;
    /*

    cv::Mat src1 = cv::imread("", cv::IMREAD_COLOR);
    cv::Mat src2 = cv::imread("", cv::IMREAD_COLOR);
    printf("width: %d\n", src1.cols);
    printf("height: %d\n", src1.rows);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Conduct resize.
    cv::Mat scaled1;
    cv::Mat scaled2;
    cv::resize(src1, scaled1, cv::Size(src1.cols/4, src2.rows/4), 0, 0,
            cv::INTER_NEAREST);
    cv::resize(src2, scaled2, cv::Size(src2.cols/4, src2.rows/4), 0, 0,
            cv::INTER_NEAREST);
    
    // Conduct Box Blurs
    cv::Mat blur1;
    cv::Mat blur2;
    cv::blur(scaled1, blur1, cv::Size(20,20));
    cv::blur(scaled2, blur2, cv::Size(20,20)); 

    // Calculate deltas.
    cv::Mat delta = cv::Mat(blur2.cols, blur2.rows, CV_8UC3);
    ImgDiff::diff(blur1, blur2, delta, 0);
    ImgDiff::diffThreshCentre(delta, 50);
    
    auto stop = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_seconds = stop - start;

    std::cout << "Calcs Per Sec: " << 1.0/elapsed_seconds.count() << std::endl;
    
    cv::namedWindow("Display Image", cv::WINDOW_AUTOSIZE);
    printf("swidth: %d\n", delta.cols);
    printf("sheight: %d\n", delta.rows);
    cv::imshow("Display Image", delta);
    cv::waitKey(0);
    return 0;
    */
}

void tr::updateMatrices(
        cv::VideoCapture& vidCap,
        cv::Mat& frame,
        cv::Mat& shrinkFrame,
        cv::Mat& grey,
        cv::Size newSize)
{
    vidCap.read(frame);
    cv::resize(frame, shrinkFrame, newSize);
    cv::cvtColor(shrinkFrame, grey,
                 cv::ColorConversionCodes::COLOR_BGR2GRAY, 0);
}

void mask(cv::Mat& m)
{
    cv::Mat hsv;
    cv::cvtColor(m, hsv, cv::ColorConversionCodes::COLOR_BGR2HSV, 0);
    for (size_t i = 0; i < hsv.rows; ++i) {
        unsigned char* hsvRowStart = hsv.ptr<unsigned char>(i);
        unsigned char* mRowStart = m.ptr<unsigned char>(i);
        for (size_t j = 0; j < hsv.cols*hsv.channels(); j += 3) {
            unsigned char* h = hsvRowStart;
            unsigned char* b = mRowStart;
            unsigned char* g = mRowStart + 1;
            unsigned char* r = mRowStart + 2;
            if (*h > 0 && *h < 40) {
                *b = 0;
                *g = 0;
                *r = 255;
            }
            hsvRowStart += 3;
            mRowStart += 3;
        }
    }
}
