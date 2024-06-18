#include "decoder.h"

#include "declare_main.h"
#include "graphics/renderer.h"
#include "model/mesh.h"
#include "common/sc_assert.h"

#include <cstring>
#include <mutex>
#include <atomic>

static const int kFrameTextureSize = 1024;

class VideoPlayerApp
: public scythe::OpenGlApplication
, public scythe::DesktopInputListener
, public IFrameListener
{
public:
	VideoPlayerApp()
	: quad_(nullptr)
	, update_frame_(false)
	, buffer_(nullptr)
	, video_width_(0)
	, video_height_(0)
	, video_ready_(false)
	, paused_(false)
	{
		SetInputListener(this);
	}
	const char* GetTitle() final
	{
		return "Video Player";
	}
	const bool IsMultisample() final
	{
		return false;
	}
	bool Load() final
	{
		const char* kVideoUrl = "data/video/sample.mpeg";

		// Initialize decoder first
		if (!decoder_.Initialize(this))
			return false;
		if (!decoder_.Load(kVideoUrl))
			return false;

		if (!video_ready_)
			return false;
		buffer_ = new uint8_t[video_width_ * video_height_ * 3];
		if (buffer_ == nullptr)
			return false;

		// Vertex formats
		scythe::VertexFormat * quad_vertex_format;
		{
			scythe::VertexAttribute attributes[] = {
				scythe::VertexAttribute(scythe::VertexAttribute::kVertex, 3)
			};
			renderer_->AddVertexFormat(quad_vertex_format, attributes, _countof(attributes));
		}

		// Quad model
		quad_ = new scythe::Mesh(renderer_);
		quad_->CreateQuadFullscreen();
		if (!quad_->MakeRenderable(quad_vertex_format))
			return false;
		
		// Load shaders
		if (!renderer_->AddShader(quad_shader_, "data/shaders/quad")) return false;

		// Create frame texture
		renderer_->CreateTextureFromData(frame_texture_, video_width_, video_height_, scythe::Image::Format::kRGB8, nullptr);

		// No need for depth test in quad rendering
		renderer_->DisableDepthTest();

		decoder_.Demux();
		
		return true;
	}
	void Unload() final
	{
		if (buffer_)
			delete buffer_;
		if (quad_)
			delete quad_;
		decoder_.Free();
		decoder_.Deinitialize();
	}
	void OnVideoInfoReady(int width, int height, int format) final
	{
		video_width_ = width;
		video_height_ = height;
		video_ready_ = width > 0 && height > 0;
	}
	void OnFrameReady(uint8_t **data, int* linesize, int64_t frame_number) final
	{
		SC_ASSERT(linesize[0] == video_width_ * 3);
		update_frame_ = true;
		mutex_.lock();
		size_t size = video_width_ * video_height_ * 3;
		std::memcpy(buffer_, data[0], size);
		mutex_.unlock();
	}
	void Update() final
	{
		// const float kFrameTime = GetFrameTime();
		if (update_frame_)
		{
			update_frame_ = false;
			mutex_.lock();
			const void* pixels = reinterpret_cast<const void*>(buffer_);
			renderer_->SetTextureData(frame_texture_, 0, 0, video_width_, video_height_, pixels);
			mutex_.unlock();
		}
	}
	void Render() final
	{
		renderer_->SetViewport(width_, height_);

		renderer_->ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		renderer_->ClearColorBuffer();
		// renderer_->ClearColorAndDepthBuffers();

		// Render fullscreen quad with frame data texture
		quad_shader_->Bind();
		quad_shader_->Uniform1i("u_texture", 0);
		renderer_->ChangeTexture(frame_texture_, 0);
		quad_->Render();
		renderer_->ChangeTexture(nullptr, 0);
		quad_shader_->Unbind();
	}
	void OnChar(unsigned short code) final
	{
	}
	void OnKeyDown(scythe::PublicKey key, int mods) final
	{
		if (key == scythe::PublicKey::kF)
		{
			DesktopApplication::ToggleFullscreen();
		}
		else if (key == scythe::PublicKey::kEscape)
		{
			DesktopApplication::Terminate();
		}
		else if (key == scythe::PublicKey::kF5)
		{
			renderer_->TakeScreenshot("screenshots");
		}
	}
	void OnKeyUp(scythe::PublicKey key, int modifiers) final
	{
	}
	void OnMouseDown(scythe::MouseButton button, int modifiers) final
	{
	}
	void OnMouseUp(scythe::MouseButton button, int modifiers) final
	{
	}
	void OnMouseMove() final
	{
	}
	void OnScroll(float delta_x, float delta_y) final
	{
	}
	void OnSize(int w, int h) final
	{
		DesktopApplication::OnSize(w, h);
	}
	
private:
	scythe::Mesh * quad_;

	scythe::Shader * quad_shader_;

	scythe::Texture * frame_texture_;

	Decoder decoder_;

	std::mutex mutex_;
	std::atomic<bool> update_frame_;

	uint8_t *buffer_;
	int video_width_, video_height_;
	bool video_ready_;
	bool paused_;
};

DECLARE_MAIN(VideoPlayerApp);