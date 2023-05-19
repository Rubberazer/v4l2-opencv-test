/*
 *Testing the V4l2 API on a jetson nano
 *Compile with: gcc -Wall -Werror -o v4l2_test1 v4l2_test1.c -lrt
 *Execute with: sudo ./v4l2_test1
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
    
    int fd;
    
    if((fd = open("/dev/video0", O_RDWR)) < 0){
        perror("failed to open");
        exit(EXIT_FAILURE);
    }

    //Query device capabilities

    struct v4l2_capability cap;
    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0){
        perror("Failed to query capabilities");
        exit(EXIT_FAILURE);
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)){
        fprintf(stderr, "The device does not handle single-planar video capture.\n");
        exit(EXIT_FAILURE);
    }

    /* if (!(cap.capabilities & V4L2_CAP_READWRITE)){ */
    /*     fprintf(stderr, "The device does not support read/write.\n"); */
    /*     exit(EXIT_FAILURE); */
    /* } */

    if (!(cap.capabilities & V4L2_CAP_STREAMING)){
        fprintf(stderr, "The device does not support streaming.\n");
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
    
    if (ioctl(fd, VIDIOC_S_FMT, &format) < 0){
        perror("Could not set video format");
        exit(EXIT_FAILURE);
    }
    
    //Buffer request
    struct v4l2_requestbuffers bufrequest = {0};
    bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufrequest.memory = V4L2_MEMORY_MMAP;
    bufrequest.count = 1;
 
    if(ioctl(fd, VIDIOC_REQBUFS, &bufrequest) < 0){
        perror("Failed buffer request");
        exit(EXIT_FAILURE);
    }
    
    //Query buffers
    struct v4l2_buffer bufferinfo = {0};
 
    bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufferinfo.memory = V4L2_MEMORY_MMAP;
    bufferinfo.index = 0;
 
    if (ioctl(fd, VIDIOC_QUERYBUF, &bufferinfo) < 0){
        perror("Failed to query buffers");
        exit(EXIT_FAILURE);
    }
    
    void *buffer_start = mmap(NULL, bufferinfo.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, bufferinfo.m.offset);
 
    if (buffer_start == MAP_FAILED){
        perror("failed to mmap() buffer");
        exit(EXIT_FAILURE);
    }

    // Put the buffer in the incoming queue.
    if(ioctl(fd, VIDIOC_QBUF, &bufferinfo) < 0){
        perror("Failed to queue buffers");
        exit(EXIT_FAILURE);
    }
    
    // Start streaming
    unsigned type = bufferinfo.type;
    
    if(ioctl(fd, VIDIOC_STREAMON, &type) < 0){
        perror("Failed to activate streaming");
        exit(EXIT_FAILURE);
    }
 
    printf("Streaming.\n");
   
    // The buffer's waiting in the outgoing queue. This block will not trigger an error but
    // will make the program hang
    /* if(ioctl(fd, VIDIOC_DQBUF, &bufferinfo) < 0){ */
    /*     perror("Failed to dequeue buffer"); */
    /*     exit(EXIT_FAILURE); */
    /* } */

    //printf("Dequeued.\n");

    //Saving raw image to file
    int picture = open("picture.raw", O_RDWR | O_CREAT, 0666);
    fprintf(stderr, "picture == %i\n", picture);
    write(picture, buffer_start, bufferinfo.length);
    
    // Stop streaming
    if(ioctl(fd, VIDIOC_STREAMOFF, &type) < 0){
        perror("Failed to stop streaming");
        exit(EXIT_FAILURE);
        }
    
    printf("before close()\n");
    close(picture);
    close(fd);
    return EXIT_SUCCESS;
}
