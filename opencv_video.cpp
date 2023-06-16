/*
 * To compile: g++ -Wall -Werror -c -o opencv_video.o opencv_video.cpp 

 */
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/hal/interface.h>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/cuda.hpp>
#include "video_capture.h"

#include <iostream>
#include <fstream>

using namespace std;

int main(int argc, char* argv[]){

    unsigned size = 0;
    unsigned nbufs = 5;
    char const *camera = "/dev/video0";

    int fd = camera_init(camera, 1280, 720, nbufs, &size);

    char *frame = new char[size];
    camera_stream_on(fd, frame);
    
    cv::cuda::GpuMat src, dst;
    cv::Mat result, output;
    
    while(1){
    
        cv::Mat src_host(720, 1280, CV_16UC1, frame);  //(2464, 3264, CV_16UC1, buffer);
        src_host.convertTo(src_host, CV_8U);

        //src.upload(src_host);

        // Debayer here
        //cv::cuda::cvtColor(src, dst, cv::COLOR_BayerBG2BGR); //cv::cuda::cvtColor(src, dst, cv::COLOR_BayerRGGB2RGB);
        cv::cvtColor(src_host, result, cv::COLOR_BayerBG2BGR);
        
        //cv::cuda::gammaCorrection(dst, dst, true);

        // have a look
       
        //dst.download(result);
        
        cv::convertScaleAbs(result, result, 1.1, 2.0);
        cv::addWeighted(result, 1.5, result, 0, 1.5, output);  
        cv::imshow("Debayered Image", output);
    
        if ((char)27 == cv::waitKey(1)){
            break;
        }
    }
    camera_stream_off(fd, nbufs);
    delete[] frame;
    return 0;
}
