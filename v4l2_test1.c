/*
 *Testing the V4l2 API on a jetson nano
 *Compile with: gcc -Wall -Werror -g -o v4l2_test1 v4l2_test1.c -lrt 
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <linux/videodev2.h> 

int main(void){

    printf("I can start\n");
    
    int fd;
    
    if((fd = open("/dev/video0", O_RDWR)) < 0){
        perror("failed to open");
        exit(EXIT_FAILURE);
    }

    //Query device capabilities

    struct v4l2_capability cap;
    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0){
        perror("VIDIOC_QUERYCAP");
        exit(EXIT_FAILURE);
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)){
        fprintf(stderr, "The device does not handle single-planar video capture.\n");
        exit(EXIT_FAILURE);
    }

    if (!(cap.capabilities & V4L2_CAP_READWRITE)){
        fprintf(stderr, "The device does not support read/write.\n");
        exit(EXIT_FAILURE);
    }

    if (!(cap,capabilities & V4L2_CAP_STREAMING)){
        fprint(stderr, "The device does not support streaming.\n");
        exit(EXIT_FAILURE);
    }
    
    //To know which formats are available isue command:  v4l2-ctl -d /dev/video0 --list-formats-ext
    //Set out video formats

    struct v4l2_format format = {0};
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix.pixelformat = V4L2_PIX_FMT_SRGGB10; //V4L2_PIX_FMT_MJPEG;
    format.fmt.pix.width = 1280;
    format.fmt.pix.height = 720;
    format.fmt.pix.field = V4L2_FIELD_NONE;
    
    if(ioctl(fd, VIDIOC_S_FMT, &format) < 0){
        perror("Could not set video format");
        exit(EXIT_FAILURE);
    }
    printf("I arrive to the third\n");
    
    //Buffer request
    struct v4l2_requestbuffers bufrequest;
    bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufrequest.memory = V4L2_MEMORY_MMAP;
    bufrequest.count = 1;
 
    if(ioctl(fd, VIDIOC_REQBUFS, &bufrequest) < 0){
        perror("VIDIOC_REQBUFS");
        exit(EXIT_FAILURE);
    }
    printf("I arrive to the fourth\n");
    
    //Allocate buffers
    struct v4l2_buffer bufferinfo;
    memset(&bufferinfo, 0, sizeof(bufferinfo));
 
    bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufferinfo.memory = V4L2_MEMORY_MMAP;
    bufferinfo.index = 0;
 
    if(ioctl(fd, VIDIOC_QUERYBUF, &bufferinfo) < 0){
        perror("VIDIOC_QUERYBUF");
        exit(EXIT_FAILURE);
    }
    printf("I arrive to the fifth\n");
    
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
    printf("Arrive to the streaming\n");

    // Put the buffer in the incoming queue.
    if(ioctl(fd, VIDIOC_QBUF, &bufferinfo) < 0){
        perror("VIDIOC_QBUF");
        exit(-1);
    }
    
    // Activate streaming
    int type = bufferinfo.type;
    if(ioctl(fd, VIDIOC_STREAMON, &type) < 0){
        perror("VIDIOC_STREAMON");
        exit(EXIT_FAILURE);
    }
 
    printf("buffer on the queue\n");

    // The buffer's waiting in the outgoing queue. This block will not through an error but will make the program hang
    if(ioctl(fd, VIDIOC_DQBUF, &bufferinfo) < 0){
        perror("VIDIOC_QBUF");
        exit(EXIT_FAILURE);
    }
    
    // Your loops end here
 
    // Deactivate streaming
    if(ioctl(fd, VIDIOC_STREAMOFF, &type) < 0){
        perror("VIDIOC_STREAMOFF");
        exit(EXIT_FAILURE);
        }
    printf("before close()\n");
    close(fd);
    return EXIT_SUCCESS;
}
