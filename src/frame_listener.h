#ifndef __FRAME_LISTENER_H__
#define __FRAME_LISTENER_H__

#include <cstdint>

/**
 * Interface class for frame listener
 */
class IFrameListener {
public:

    /**
     * Callback on video information is ready
     * 
     * @param width  The frame width
     * @param height The frame height
     * @param format The pixel format
     */
    virtual void OnVideoInfoReady(int width, int height, int format) = 0;

    /**
     * Callback on frame data is ready
     * 
     * @param data     The data array
     * @param linesize The line size array (stride)
     */
    virtual void OnFrameReady(uint8_t **data, int* linesize, int64_t frame_number) = 0;
};

#endif