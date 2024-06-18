#ifndef __DECODER_H__
#define __DECODER_H__

#include "frame_listener.h"

// Forward declarations
class DecoderImpl;

/**
 * A video decoder.
 */
class Decoder {
public:
	Decoder();
	~Decoder();

	bool Initialize(IFrameListener * listener);
	void Deinitialize();

	/**
	 * Loads video from provided URL. Doesn't start demuxing automatically.
	 * 
	 * @param url The URL of stream video or file path.
	 * @return True on success and false otherwise.
	 * @see Demux()
	 */
	bool Load(const char* url);
	void Free();

	void Demux();

	void Start();
	void Stop();

private:
	DecoderImpl * impl_;
};

#endif