/*
 * Testing the V4l2 API on a jetson nano
 * video capture
 * gcc -o v4l_video_capture.o v4l_video_capture.c  -I./ -lrt
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
#include <linux/tegra-v4l2-camera.h> //To be able to manipulate picture width & height
#include <pthread.h>

#include "video_capture.h"

//Globals
static volatile unsigned global_int;
static pthread_t callThd[1]; // number of threads, only one is needed for capturing
static pthread_attr_t attr;
static int pth_err;
static void *status_thread;
static int thread_n = 0;
struct {
    void *start;
    size_t length;
} *buffers;
struct {
    unsigned nbuf;
    unsigned index;
    void *buffer;
} parameters;


int camera_init(const char *camera, unsigned width, unsigned height, unsigned nbufs){    
    int fd;
    // int nbufs = 20;
    // char *camera = "/dev/video0";
    
    if((fd = open(*camera, O_RDWR)) < 0){
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

    if (!(cap.capabilities & V4L2_CAP_STREAMING)){
        fprintf(stderr, "The device does not support streaming.\n");
        exit(EXIT_FAILURE);
    }

    //Setup format, size, width, height
    struct v4l2_format format = {0};
    struct v4l2_ext_controls controls = {0};
    struct v4l2_ext_control control = {0};
    int id = TEGRA_CAMERA_CID_SENSOR_MODE_ID; //Magic number that allows us to manipulate the format
    
    control.id = id;
    control.value = 4;
    controls.ctrl_class = V4L2_CTRL_ID2CLASS(id);
    controls.count = 1;
    controls.controls = &control;
    
    if (ioctl(fd, VIDIOC_S_EXT_CTRLS, &controls) < 0){
        perror("Could not setup camera controls");
        exit(EXIT_FAILURE);
    }
    
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix.pixelformat = V4L2_PIX_FMT_SRGGB10; //V4L2_PIX_FMT_MJPEG;
    format.fmt.pix.width = width; 
    format.fmt.pix.height = height;
    format.fmt.pix.field = V4L2_FIELD_NONE;
    
    if (ioctl(fd, VIDIOC_S_FMT, &format) < 0){
        perror("Could not set video format");
        exit(EXIT_FAILURE);
    }
    
    //Buffer request
    struct v4l2_requestbuffers bufrequest = {0};
    bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufrequest.memory = V4L2_MEMORY_MMAP;
    bufrequest.count = nbufs;
 
    if(ioctl(fd, VIDIOC_REQBUFS, &bufrequest) < 0){
        perror("Failed buffer request");
        exit(EXIT_FAILURE);
    }
    buffers = calloc(bufrequest.count, sizeof(*buffers));
    
    //Query buffers
    unsigned int i;

    for (i = 0; i < bufrequest.count; i++) {
 
        struct v4l2_buffer bufferinfo = {0};
        bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        bufferinfo.memory = V4L2_MEMORY_MMAP;
        bufferinfo.index = i;
 
        if (ioctl(fd, VIDIOC_QUERYBUF, &bufferinfo) < 0){
            perror("Failed to query buffers");
            exit(EXIT_FAILURE);
        }

        buffers[i].length = bufferinfo.length;
        buffers[i].start = mmap(NULL, bufferinfo.length, PROT_READ | PROT_WRITE, MAP_SHARED,
                                fd, bufferinfo.m.offset);
 
        if (buffers[i].start == MAP_FAILED){
            perror("failed to mmap() buffer");
            exit(EXIT_FAILURE);
        }
    }
    
    // Put the buffer in the incoming queue this is needed only for some cameras
    struct v4l2_buffer bufferq = {0};
    bufferq.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufferq.memory = V4L2_MEMORY_
    bufferq.index = 0;

    if (ioctl(fd, VIDIOC_QBUF, &bufferq) < 0){
        perror("Failed to queue buffers");
        exit(EXIT_FAILURE);
    }
    
    parameters.nbuf = nbuf;
    parameters.index = bufferq.index;
    
    return fd;
}

void *callback(void *arg){

}

void *camera_stream_on(int fd, char * frame){
    
    // Start streaming
    unsigned type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    
    if(ioctl(fd, VIDIOC_STREAMON, &type) < 0){
        perror("Failed to activate streaming");
        exit(EXIT_FAILURE);
    }

    printf("Streaming and waiting for frame.\n");
    
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    struct timeval tv = {0};
    tv.tv_sec = 2;
    int r = select(fd+1, &fds, NULL, NULL, &tv);
    if (-1 == r){
        perror("Waiting for Frame");
        exit(1);
    }
    
    // The buffer's waiting in the outgoing queue.
     struct v4l2_buffer bufferdq = {0};
     bufferdq.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
     bufferdq.memory = V4L2_MEMORY_MMAP;
     bufferdq.index = 0;
    
    if (ioctl(fd, VIDIOC_DQBUF, &bufferdq) < 0){
        perror("Failed to dequeue buffer");
        exit(EXIT_FAILURE);
    }

    printf("Dequeued first time. Creating thread\n");

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pth_err = pthread_create(&callThd[thread_n], &attr, callback, (void *)parameters); 
    if (pth_err !=0){
        perror("Thread not created, exiting the function  with error: ");
        exit(EXIT_FAILURE);
    }

    //Saving raw image to file
    /* int picture = open("picture.raw", O_RDWR | O_CREAT, 0666); */
    /* fprintf(stderr, "picture == %i\n", picture); */
    /* write(picture, buffers[bufferdq.index].start, bufferdq.bytesused); */

    return 0;
}

int camera_stream_off(int fd, unsigned nbufs){
    // Stop streaming
    if(ioctl(fd, VIDIOC_STREAMOFF, &type) < 0){
        perror("Failed to stop streaming");
        exit(EXIT_FAILURE);
    }
    
    // Closing 
    close(fd);
    
    for(int i = 0;i < thread_n; i++) {
        pthread_cancel(callThd[i]);
      //printf("Thread number: %d cancelled\n",i);
    }
    
    //Joining threads
     for(int j = 0;j < thread_n; j++) {
         pthread_join(callThd[j], &status_thread);
      //printf("Thread number: %d joined\n",j);
    }
    
    for (i = 0; i < nbuf; i++){
        munmap(buffers[i].start, buffers[i].length);
    }
    free(buffers);
    
    return 1;
}
