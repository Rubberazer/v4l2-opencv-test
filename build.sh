gcc -Wall -Werror -o v4l2_frame_capture v4l2_frame_capture.c -lrt

g++ -Wall -Werror -o opencv_frame_demosaicing opencv_frame_demosaicing.cpp -I/usr/include/opencv4/ -L/usr/lib/aarch64-linux-gnu/ -lrt -lopencv_videoio -lopencv_core -lopencv_imgproc -lopencv_ml -lopencv_video -lopencv_highgui -lopencv_imgcodecs -lopencv_cudaimgproc -lopencv_cudaarithm

