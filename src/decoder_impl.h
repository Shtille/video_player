#ifndef __DECODER_IMPL_H__
#define __DECODER_IMPL_H__

#include "frame_listener.h"
#include "frame_converter.h"

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
}

#include <mutex>
#include <condition_variable>
#include <thread>

class DecoderImpl {
	static const AVPixelFormat kTargetFormat = AV_PIX_FMT_RGB24;
public:
	DecoderImpl(IFrameListener * listener = nullptr);
	~DecoderImpl();

	bool Initialize();
	void Deinitialize();

	bool Load(const char* url);
	void Free();

	void Start();
	void Stop();

	// Main routine
	void Demux();
	void DemuxRoutine();

protected:
	void DelayFrame();
	void DecodeFrame();
	void StartService();
	void StopService();

private:
	IFrameListener * listener_;
	FrameConverter converter_;
	AVFormatContext * format_context_ = nullptr;
	AVCodecContext * codec_context_ = nullptr;
	AVFrame * frame_ = nullptr;
	AVPacket * packet_ = nullptr;
	AVPixelFormat pixel_format_;

	int64_t last_pts_ = AV_NOPTS_VALUE;
	int video_stream_index_ = 0;
	int width_, height_;
	const bool use_converter_ = true;
	bool paused_ = false;
	bool finishing_ = false;

	std::mutex mutex_;
	std::condition_variable condition_variable_;
	std::thread thread_;
};

#endif