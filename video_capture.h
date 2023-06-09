/*
 *  Header file for the video capturer
 */

#ifndef video_capture_h__
#define video_capture_h__

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Functions */
    /* Returning fd, it will exit otherwise  */
    int camera_init(const char *camera, unsigned width, unsigned height, unsigned nbufs, unsigned *size);

    /* Returning 0 when OK, it will exit otherwise */
    int camera_stream_on(int fd, char *frame);
    
    /* Returning 0 when OK, it will exit otherwise */
    int camera_stream_off(int fd, unsigned nbufs);

#ifdef __cplusplus
}
#endif
 
#endif  // video_capture_h__
