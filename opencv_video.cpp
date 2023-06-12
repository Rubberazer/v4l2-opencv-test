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
#include <unistd.h>

#include <iostream>
#include <fstream>

using namespace std;

int main(int argc, char* argv[]){

    unsigned size = 0;
    unsigned nbufs = 20;
    char const *camera = "/dev/video0";

    int fd = camera_init(camera, 1280, 720, nbufs, &size);

    char *frame = new char[size];
    camera_stream_on(fd, frame);

    sleep(2);

    while(1){
    
        cv::Mat src_host(720, 1280, CV_16UC1, frame);  //(2464, 3264, CV_16UC1, buffer);
        src_host.convertTo(src_host, CV_8U);

        cv::cuda::GpuMat src, dst;
        src.upload(src_host);

        // Debayer here
        cv::cuda::cvtColor(src, dst, cv::COLOR_BayerRGGB2RGB);
        cv::cuda::gammaCorrection(dst, dst, true);

        // have a look
        cv::Mat result;
        dst.download(result);
        cv::imshow("Debayered Image", result);
    
        if ((char)27 == cv::waitKey(1)){
            break;
        }
    }
    camera_stream_off(fd, nbufs);
    delete[] frame;
    return 0;
}
