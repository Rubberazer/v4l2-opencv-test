/*
 *Testing the V4l2 API on a jetson nano
 *Compile with: gcc -Wall -Werror -g -o v4l2_test1 v4l2_test1.c -lrt
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <sys/fcntl.h>
#include <sys/mman.h>

int main(void){
    int fd;
    
    if((fd = open("/dev/video0", O_RDWR)) < 0){
        perror("failed to open");
        exit(-1);
    }

    //Query device capabilities
    struct v4l2_capability cap;
    if(ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0){
        perror("VIDIOC_QUERYCAP");
        exit(-1);
    }

    if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)){
        fprintf(stderr, "The device does not handle single-planar video capture.\n");
        exit(-1);
    }
    
    //To know which formats are available isue command:  v4l2-ctl -d /dev/video0 --list-formats-ext
    //Set out video formats
    struct v4l2_format format;
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix.pixelformat = V4L2_PIX_FMT_SRGGB10; //V4L2_PIX_FMT_MJPEG;
    format.fmt.pix.width = 1280;
    format.fmt.pix.height = 720;
 
    if(ioctl(fd, VIDIOC_S_FMT, &format) < 0){
        perror("VIDIOC_S_FMT");
        exit(-1);
    }
        
    //Buffer request
    struct v4l2_requestbuffers bufrequest;
    bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufrequest.memory = V4L2_MEMORY_MMAP;
    bufrequest.count = 1;
 
    if(ioctl(fd, VIDIOC_REQBUFS, &bufrequest) < 0){
        perror("VIDIOC_REQBUFS");
        exit(-1);
    }

    //Allocate buffers
    struct v4l2_buffer bufferinfo;
    memset(&bufferinfo, 0, sizeof(bufferinfo));
 
    bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufferinfo.memory = V4L2_MEMORY_MMAP;
    bufferinfo.index = 0;
 
    if(ioctl(fd, VIDIOC_QUERYBUF, &bufferinfo) < 0){
        perror("VIDIOC_QUERYBUF");
        exit(-1);
    }

    void* buffer_start = mmap(
        NULL,
        bufferinfo.length,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        fd,
        bufferinfo.m.offset);
 
    if(buffer_start == MAP_FAILED){
        perror("mmap");
        exit(-1);
    }
 
    memset(buffer_start, 0, bufferinfo.length);

    //Get a frame
    //struct v4l2_buffer bufferinfo;
    //memset(&bufferinfo, 0, sizeof(bufferinfo));
 
    //bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    //bufferinfo.memory = V4L2_MEMORY_MMAP;
    //bufferinfo.index = 0; /* Queueing buffer index 0. */

    // Activate streaming
    int type = bufferinfo.type;
    if(ioctl(fd, VIDIOC_STREAMON, &type) < 0){
        perror("VIDIOC_STREAMON");
        exit(-1);
    }
 
    /* Here is where you typically start two loops:
    * - One which runs for as long as you want to
    *   capture frames (shoot the video).
    * - One which iterates over your buffers everytime. */
    
    // Put the buffer in the incoming queue.
    if(ioctl(fd, VIDIOC_QBUF, &bufferinfo) < 0){
        perror("VIDIOC_QBUF");
        exit(-1);
    }
 
    // The buffer's waiting in the outgoing queue.
    if(ioctl(fd, VIDIOC_DQBUF, &bufferinfo) < 0){
        perror("VIDIOC_QBUF");
        exit(-1);
    }
 
    /* Your loops end here. */
 
    // Deactivate streaming
    if(ioctl(fd, VIDIOC_STREAMOFF, &type) < 0){
        perror("VIDIOC_STREAMOFF");
        exit(-1);
    }
    
    close(fd);
    return EXIT_SUCCESS;
}
