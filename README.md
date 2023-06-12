# v4l2-opencv-test

Playing with v4l2 and opencv;

This is about trying to get frames from a CSI camera plugged into a Nano and then MATtify those frames to be able to play with them on opencv.
Reason to  do this is to find alternatives to gstreamer and libargus, let's see where this goes.

Platform: Jetson Nano dev kit

Camera: imx219

STEPS


 	1 First camera frame capture: v4l2_frame_capture.c  DONE, not OpenCV yet
 
	2 Convert raw pic to RGB or something (debayer): opencv_frame_demosaicing.cpp DONE

	3 Create continuous stream: v4l2_video_capture.c, video_capture.h DONE / opencv_video.cpp DONE. Execute with ./video_capture ->This is a bad idea,
	performance is rubbish!
			

