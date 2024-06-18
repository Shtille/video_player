#include "decoder_impl.h"

#include <cstdio>
#include <chrono>

static void LogError(const char* str)
{
	fprintf(stderr, "%s\n", str);
}
static void LogErrorDetails(int error_code)
{
	char buf[1024];
	av_strerror(error_code, buf, sizeof(buf));
	fprintf(stderr, "FFMPEG ERROR: %s\n", buf);
}

DecoderImpl::DecoderImpl(IFrameListener * listener)
: listener_(listener)
{

}
DecoderImpl::~DecoderImpl()
{
	Free();
}
bool DecoderImpl::Initialize()
{
	avformat_network_init();
	return true;
}
void DecoderImpl::Deinitialize()
{
	avformat_network_deinit();
}
bool DecoderImpl::Load(const char* url)
{
	const enum AVMediaType kMediaType = AVMEDIA_TYPE_VIDEO;
	const AVCodec * codec = nullptr;
	AVStream * stream = nullptr;
	int ret, stream_index;

	ret = avformat_open_input(&format_context_, url, nullptr, nullptr);
	if (ret < 0)
	{
		LogError("open input failed");
		LogErrorDetails(ret);
		return false;
	}
	ret = avformat_find_stream_info(format_context_, nullptr);
	if (ret < 0)
	{
		LogError("find stream failed");
		LogErrorDetails(ret);
		return false;
	}
	ret = av_find_best_stream(format_context_, kMediaType, -1, -1, &codec, 0);
	if (ret < 0)
	{
		LogError("can't find stream");
		LogErrorDetails(ret);
		return false;
	}
	stream_index = ret;
	stream = format_context_->streams[stream_index];
	if (!stream)
	{
		LogError("stream is invalid");
		return false;
	}

	codec_context_ = avcodec_alloc_context3(codec);
	if (!codec_context_)
	{
		ret = AVERROR(ENOMEM);
		LogError("can't alloc context");
		return false;
	}
	ret = avcodec_parameters_to_context(codec_context_, stream->codecpar);
	if (ret < 0)
	{
		LogError("failed to copy stream codec parameters");
		LogErrorDetails(ret);
		return false;
	}
	ret = avcodec_open2(codec_context_, codec, nullptr);
	if (ret < 0)
	{
		LogError("failed to open codec");
		LogErrorDetails(ret);
		return false;
	}

	// Video information summary
	video_stream_index_ = stream_index;
	width_ = codec_context_->width;
	height_ = codec_context_->height;
	pixel_format_ = codec_context_->pix_fmt;

	if (listener_ != nullptr)
		listener_->OnVideoInfoReady(width_, height_, (int)pixel_format_);

	av_dump_format(format_context_, stream_index, url, 0);

	frame_ = av_frame_alloc();
	if (!frame_)
	{
		LogError("failed to alloc frame");
		return false;
	}

	packet_ = av_packet_alloc();
	if (!packet_)
	{
		LogError("failed to alloc packet");
		return false;
	}

	// Check if we need conversion
	if (pixel_format_ != kTargetFormat)
	{
		// Highly likely we will convert YUV to RGB
		converter_.Set(width_, height_, 16, pixel_format_,
					   width_, height_,  1, kTargetFormat);
		if (!converter_.Initialize())
		{
			LogError("failed to init converter");
			return false;
		}
	}
	return true;
}
void DecoderImpl::Free()
{
	StopService();
	converter_.Deinitialize();
	if (codec_context_ != nullptr)
	{
		avcodec_free_context(&codec_context_);
	}
	if (format_context_ != nullptr)
	{
		avformat_close_input(&format_context_);
	}
	if (frame_ != nullptr)
	{
		av_frame_free(&frame_);
	}
	if (packet_ != nullptr)
	{
		av_packet_free(&packet_);
	}
}
void DecoderImpl::Start()
{
	paused_ = false;
	condition_variable_.notify_one();
}
void DecoderImpl::Stop()
{
	paused_ = true;
	condition_variable_.notify_one();
}
void DecoderImpl::Demux()
{
	StartService();
}
void DecoderImpl::DemuxRoutine()
{
	volatile bool finishing = false;
	volatile bool paused = false;

	while (av_read_frame(format_context_, packet_) >= 0)
	{
		{//---
			std::lock_guard<std::mutex> guard(mutex_);
			finishing = finishing_;
			paused = paused_;
		}//---
		if (packet_->stream_index == video_stream_index_)
		{
			if (avcodec_send_packet(codec_context_, packet_) == 0)
			{
				while (avcodec_receive_frame(codec_context_, frame_) == 0)
				{
					DecodeFrame();
				}
			}
		}
		if (paused)
		{
			std::unique_lock<std::mutex> guard(mutex_);
			while (!finishing_ && paused_)
				condition_variable_.wait(guard);
		}
		if (finishing)
			break;
	}
	if (avcodec_send_packet(codec_context_, nullptr) == 0)
	{
		while (avcodec_receive_frame(codec_context_, frame_) == 0)
		{
			DecodeFrame();
		}
	}
}
void DecoderImpl::DelayFrame()
{
	// https://github.com/FFmpeg/FFmpeg/blob/master/doc/examples/decode_filter_video.c#L180
	int64_t delay;
	AVRational time_base = format_context_->streams[video_stream_index_]->time_base;

	frame_->pts = frame_->best_effort_timestamp;

	if (frame_->pts != AV_NOPTS_VALUE)
	{
		if (last_pts_ != AV_NOPTS_VALUE)
		{
			// Sleep roughly right amount of time
			delay = av_rescale_q(frame_->pts - last_pts_, time_base, AV_TIME_BASE_Q);
			if (delay > 0 && delay < 1000000)
			{
				std::this_thread::sleep_for(std::chrono::microseconds(delay));
			}
		}
		last_pts_ = frame_->pts;
	}
}
void DecoderImpl::DecodeFrame()
{
	uint8_t **data = frame_->data;
	int *linesize = frame_->linesize;

	if (frame_->format != kTargetFormat) // like AV_PIX_FMT_YUV420P
	{
		// Need to convert data if necessary
		if (use_converter_)
		{
			// Fill src data and linesize
			converter_.SetData(frame_->data, frame_->linesize);
			converter_.Convert();
			// Finally
			data = converter_.get_data();
			linesize = converter_.get_linesize();
		}
	}
	DelayFrame();
	// Call frame callback with decoded data
	if (listener_ != nullptr)
		listener_->OnFrameReady(data, linesize, codec_context_->frame_num);
}
void DecoderImpl::StartService()
{
	finishing_ = false;
	paused_ = false;
	thread_ = std::thread(&DecoderImpl::DemuxRoutine, this);
}
void DecoderImpl::StopService()
{
	finishing_ = true;
	paused_ = false;
	condition_variable_.notify_one();
	thread_.join();
}