#ifndef __FRAME_CONVERTER_H__
#define __FRAME_CONVERTER_H__

extern "C" {
    #include <libswscale/swscale.h>
}

#include <cstdint>

class FrameConverter {
    static constexpr uint32_t kMax = AV_NUM_DATA_POINTERS;
public:

    /**
     * Destructor.
     */
    ~FrameConverter();

    /**
     * Auxilary strycture containing image information
     */
    struct ImageInfo {
        uint8_t *data[kMax];
        int linesize[kMax];
        int width;
        int height;
        int align;
        enum AVPixelFormat format;
    };

    /**
     * Sets source buffer data for conversion
     */
    void SetData(uint8_t * data[], int linesize[]);

    /**
     * Gets destination buffer data
     */
    uint8_t ** get_data() { return dst_.data; }
    /**
     * Gets destination buffer linesize (stride)
     */
    int* get_linesize() { return dst_.linesize; }

    /**
     * Sets source and destionation image properties.
     * 
     * NOTE: should be called once before Initialize()
     * 
     * @param src_width  Source width
     * @param src_height Source height
     * @param src_align  Source memory alignment
     * @param src_format Source pixel format
     * @param dst_width  Destination width
     * @param dst_height Destination height
     * @param dst_align  Destination memory alignment
     * @param dst_format Destination pixel format
     * 
     * @see Initialize()
     */
    void Set(int src_width, int src_height, int src_align, enum AVPixelFormat src_format,
             int dst_width, int dst_height, int dst_align, enum AVPixelFormat dst_format);

    /**
     * Initializes converter
     * 
     * NOTE: src and dst formats should be set before initialization.
     * 
     * @return True on success and false otherwise.
     * @see Set()
     */
    bool Initialize();

    /**
     * Deinitializes converter (frees allocated data)
     */
    void Deinitialize();

    /**
     * Converts image data
     */
    void Convert();

private:
    ImageInfo src_;
    ImageInfo dst_;
    struct SwsContext * context_ = nullptr;
    bool allocated_ = false;
};

#endif