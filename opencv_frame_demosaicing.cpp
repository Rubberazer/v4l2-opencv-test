/*
 * To compile: g++ -Wall -Werror -o opencv_frame_demosaicing opencv_frame_demosaicing.cpp -I/usr/local/include/opencv -I/usr/include/opencv4/ -L/usr/lib/aarch64-linux-gnu/ -lrt -lopencv_videoio -lopencv_core -lopencv_imgproc -lopencv_ml -lopencv_video -lopencv_highgui -lopencv_imgcodecs -lopencv_img_hash -lopencv_cudaimgproc

 */

#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/hal/interface.h>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/cuda.hpp>

#include <iostream>
#include <fstream>


using namespace std;

int main(int argc, char* argv[])
{
ifstream ifs("picture.raw", std::ifstream::binary);

ifs.seekg (0, ifs.end);
int size = ifs.tellg();
ifs.seekg (0, ifs.beg);

cout << size << endl;
cout << "Reading image into buffer..." << endl;

char* buffer = new char[size];
ifs.read (buffer,size);
ifs.close();

cv::Mat src_host(2464, 3264, CV_16UC1, buffer);
src_host.convertTo(src_host, CV_8U);

cout << "buffer to src_host" << endl;
//cv::Mat dst;
//cv::cvtColor(src_host, dst, cv::COLOR_BayerRG2RGB);

cv::cuda::GpuMat src, dst;
src.upload(src_host);

// uploaded
cout << "Upload to GPU complete" << endl;

// Debayer here
cv::cuda::cvtColor(src, dst, cv::COLOR_BayerRGGB2RGB);

// done
cout << "Demosaicing complete" << endl;

// have a look
cv::Mat result;
dst.download(result);

cv::namedWindow("Debayered Image", cv::WINDOW_KEEPRATIO);
cv::resizeWindow("Debayered Image", 3264/2, 2464/2);
cv::imshow("Debayered Image", result);
cv::waitKey(0);

delete[] buffer;
return 0;
}
