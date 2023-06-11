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
    /* Returning 1 when OK */
    int camera_init(const char *camera, unsigned width, unsigned height, unsigned nbufs, char *frame);

    /* Returning 1 when OK */
    int camera_stream_on(const char *camera, char * frame);
    
    /* Returning 1 when OK */
    int camera_stream_off(const char *camera, unsigned nbufs);

#ifdef __cplusplus
}
#endif
 
#endif  // video_capture_h__
