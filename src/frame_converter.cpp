#include "frame_converter.h"

extern "C" {
    #include <libavutil/imgutils.h>
}

FrameConverter::~FrameConverter()
{
    Deinitialize();
}
void FrameConverter::SetData(uint8_t * data[], int linesize[])
{
    for (int i = 0; i < kMax; ++i)
    {
        src_.data[i] = data[i];
        src_.linesize[i] = linesize[i];
    }
}
void FrameConverter::Set(int src_width, int src_height, int src_align, enum AVPixelFormat src_format,
                         int dst_width, int dst_height, int dst_align, enum AVPixelFormat dst_format)
{
    src_.width = src_width;
    src_.height = src_height;
    src_.align = src_align;
    src_.format = src_format;

    dst_.width = dst_width;
    dst_.height = dst_height;
    dst_.align = dst_align;
    dst_.format = dst_format;

    for (int i = 0; i < kMax; ++i)
    {
        src_.data[i] = nullptr;
        src_.linesize[i] = 0;

        dst_.data[i] = nullptr;
        dst_.linesize[i] = 0;
    }
}
bool FrameConverter::Initialize()
{
    context_ = sws_getContext(src_.width, src_.height, src_.format,
                             dst_.width, dst_.height, dst_.format,
                             SWS_BILINEAR, NULL, NULL, NULL);
    if (!context_) {
        fprintf(stderr,
                "Impossible to create scale context for the conversion "
                "fmt:%s s:%dx%d -> fmt:%s s:%dx%d\n",
                av_get_pix_fmt_name(src_.format), src_.width, src_.height,
                av_get_pix_fmt_name(dst_.format), dst_.width, dst_.height);
        return false;
    }
    // Allocate source and destination image buffers
    if (av_image_alloc(src_.data, src_.linesize,
                       src_.width, src_.height, src_.format, src_.align) < 0) {
        fprintf(stderr, "Could not allocate source image\n");
        return false;
    }
    if (av_image_alloc(dst_.data, dst_.linesize,
                       dst_.width, dst_.height, dst_.format, dst_.align) < 0) {
        fprintf(stderr, "Could not allocate destination image\n");
        return false;
    }
    allocated_ = true;
    return true;
}
void FrameConverter::Deinitialize()
{
    if (allocated_)
    {
        av_freep(&src_.data[0]);
        av_freep(&dst_.data[0]);
        allocated_ = false;
    }
    if (context_ != nullptr)
    {
        sws_freeContext(context_);
        context_ = nullptr;
    }
}
void FrameConverter::Convert()
{
    /* convert to destination format */
    sws_scale(context_, (const uint8_t * const*)src_.data, src_.linesize, 0, src_.height, 
              dst_.data, dst_.linesize);
}