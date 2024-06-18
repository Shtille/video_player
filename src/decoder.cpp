#include "decoder.h"
#include "decoder_impl.h"

Decoder::Decoder()
: impl_(nullptr)
{
}
Decoder::~Decoder()
{
	if (impl_ != nullptr)
	{
		delete impl_;
		impl_ = nullptr;
	}
}
bool Decoder::Initialize(IFrameListener * listener)
{
	impl_ = new DecoderImpl(listener);
	if (!impl_)
		return false;

	return impl_->Initialize();
}
void Decoder::Deinitialize()
{
	if (impl_)
		impl_->Deinitialize();
}
bool Decoder::Load(const char* url)
{
	return impl_->Load(url);
}
void Decoder::Free()
{
	impl_->Free();
}
void Decoder::Demux()
{
	impl_->Demux();
}
void Decoder::Start()
{
	impl_->Start();
}
void Decoder::Stop()
{
	impl_->Stop();
}