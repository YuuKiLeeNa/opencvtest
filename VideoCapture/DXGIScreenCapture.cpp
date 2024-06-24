#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "DXGIScreenCapture.h"
#include <fstream> 

//#ifdef __cplusplus
//extern "C" 
//{
//#endif
//#include "libavutil/frame.h"
//#include "libavutil/imgutils.h"
//#ifdef __cplusplus
//}
//#endif


#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")


DXGIScreenCapture::DXGIScreenCapture()
	: is_initialized_(false)
	//, is_started_(false)
	, thread_ptr_(nullptr)
	, texture_handle_(nullptr)
	//, image_ptr_(nullptr)
	//,yuv420p_(av_frame_alloc())
	//, tmp_(av_frame_alloc())
	, image_size_(0)
	, key_(0)
{
	memset(&monitor_, 0, sizeof(DX::Monitor));
	memset(&dxgi_desc_, 0, sizeof(dxgi_desc_));
}

DXGIScreenCapture::~DXGIScreenCapture()
{
	Destroy();
	/*if (yuv420p_) 
		av_frame_free(&yuv420p_);
	if(tmp_)
		av_frame_free(&tmp_);*/
}

bool DXGIScreenCapture::Init(int display_index)
{
	if (is_initialized_) {
		return true;
	}

	std::vector<DX::Monitor> monitors = DX::GetMonitors();
	if (monitors.size() < (size_t)(display_index + 1)) {
		return false;
	}

	monitor_ = monitors[display_index];

	HRESULT hr = S_OK;

	D3D_FEATURE_LEVEL feature_level;
	hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION,
		d3d11_device_.GetAddressOf(), &feature_level, d3d11_context_.GetAddressOf());
	if (FAILED(hr)) {
		printf("[DXGIScreenCapture] Failed to create d3d11 device.\n");
		return false;
	}

	Microsoft::WRL::ComPtr<IDXGIFactory> dxgi_factory;
	hr = CreateDXGIFactory1(__uuidof(IDXGIFactory), (void **)dxgi_factory.GetAddressOf());
	if (FAILED(hr)) {
		printf("[DXGIScreenCapture] Failed to create dxgi factory.\n");
		Destroy();
		return false;
	}

	Microsoft::WRL::ComPtr<IDXGIAdapter> dxgi_adapter;
	Microsoft::WRL::ComPtr<IDXGIOutput> dxgi_output;
	int index = 0;
	do
	{
		if (dxgi_factory->EnumAdapters(index, dxgi_adapter.GetAddressOf()) != DXGI_ERROR_NOT_FOUND) {
			if (dxgi_adapter->EnumOutputs(display_index, dxgi_output.GetAddressOf()) != DXGI_ERROR_NOT_FOUND) {
				if (dxgi_output.Get() != nullptr) {
					break;
				}
			}
		}
	} while (0);

	if (dxgi_adapter.Get() == nullptr) {
		printf("[DXGIScreenCapture] DXGI adapter not found.\n");
		Destroy();
		return false;
	}

	if (dxgi_output.Get() == nullptr) {		
		printf("[DXGIScreenCapture] DXGI output not found.\n");
		Destroy();
		return false;
	}

	Microsoft::WRL::ComPtr<IDXGIOutput1> dxgiOutput1;
	hr = dxgi_output.Get()->QueryInterface(__uuidof(IDXGIOutput1), reinterpret_cast<void **>(dxgiOutput1.GetAddressOf()));
	if (FAILED(hr)) {
		printf("[DXGIScreenCapture] Failed to query interface dxgiOutput1.\n");
		Destroy();
		return false;
	}

	hr = dxgiOutput1->DuplicateOutput(d3d11_device_.Get(), dxgi_output_duplication_.GetAddressOf());
	if (FAILED(hr)) {
		/* 0x887a0004: NVIDIA�������-->ȫ������--��ѡͼ�δ�����(�Զ�ѡ��) */
		printf("[DXGIScreenCapture] Failed to get duplicate output:%d.\n", hr);
		Destroy();
		return false;
	}

	dxgi_output_duplication_->GetDesc(&dxgi_desc_);

	if (CreateSharedTexture() < 0) {
		Destroy();
		return false;
	}

	is_initialized_ = true;
	//StartCapture();
	return true;
}

int DXGIScreenCapture::CreateSharedTexture()
{
	D3D11_TEXTURE2D_DESC desc = {0};
	desc.Width = dxgi_desc_.ModeDesc.Width;
	desc.Height = dxgi_desc_.ModeDesc.Height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	desc.BindFlags = 0;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX; //D3D11_RESOURCE_MISC_SHARED; 

	HRESULT hr = d3d11_device_->CreateTexture2D(&desc, nullptr, shared_texture_.GetAddressOf());
	if (FAILED(hr)) {
		printf("[DXGIScreenCapture] Failed to create texture.\n");
		return -1;
	}
	
	desc.Usage = D3D11_USAGE_STAGING;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.MiscFlags = 0;

	hr = d3d11_device_->CreateTexture2D(&desc, nullptr, rgba_texture_.GetAddressOf());
	if (FAILED(hr)) {
		printf("[DXGIScreenCapture] Failed to create texture.\n");
		return -1;
	}

	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET;
	desc.MiscFlags = D3D11_RESOURCE_MISC_GDI_COMPATIBLE;

	hr = d3d11_device_->CreateTexture2D(&desc, nullptr, gdi_texture_.GetAddressOf());
	if (FAILED(hr)) {
		printf("[DXGIScreenCapture] Failed to create texture.\n");
		return -1;
	}

	Microsoft::WRL::ComPtr<IDXGIResource> dxgiResource;
	hr = shared_texture_->QueryInterface(__uuidof(IDXGIResource), reinterpret_cast<void **>(dxgiResource.GetAddressOf()));
	if (FAILED(hr)) {
		printf("[DXGIScreenCapture] Failed to query IDXGIResource interface from texture.\n");
		return -1;
	}

	hr = dxgiResource->GetSharedHandle(&texture_handle_);
	if (FAILED(hr)) {
		printf("[DXGIScreenCapture] Failed to get shared handle.\n");
		return -1;
	}

	hr = shared_texture_->QueryInterface(_uuidof(IDXGIKeyedMutex), reinterpret_cast<void **>(keyed_mutex_.GetAddressOf()));
	if (FAILED(hr)) {
		printf("[DXGIScreenCapture] Failed to create key mutex.\n");
		return -1;
	}

	return 0;
}

bool DXGIScreenCapture::Destroy()
{
	if (is_initialized_) {
		//StopCapture();
		rgba_texture_.Reset();
		gdi_texture_.Reset();
		keyed_mutex_.Reset();
		shared_texture_.Reset();
		dxgi_output_duplication_.Reset();
		d3d11_device_.Reset();
		d3d11_context_.Reset();
		memset(&dxgi_desc_, 0, sizeof(dxgi_desc_));
		is_initialized_ = false;
	}
	return true;
}

//bool DXGIScreenCapture::CaptureFrame(AVFrame* f) 
//{
//	av_frame_unref(f);
//	int ret;
//	std::lock_guard<std::mutex>lock(mutex_);
//	if ((ret = av_frame_ref(f, yuv420p_)) != 0)
//	{
//		av_frame_unref(f);
//		return false;
//	}
//	return true;
//}

//int DXGIScreenCapture::StartCapture()
//{
//	if (!is_initialized_) {
//		return -1;
//	}
//
//	/*if (is_started_) {
//		return 0;
//	}*/
//
//	//is_started_ = true;
//	AquireFrame();
//	thread_ptr_.reset(new std::thread([this] {
//		while (is_started_) {
//			std::this_thread::sleep_for(std::chrono::milliseconds(10));
//			AquireFrame();
//		}
//	}));
//
//	return 0;
//}

//int DXGIScreenCapture::StopCapture()
//{
//	is_started_ = false;
//
//	if (thread_ptr_ != nullptr) {
//		thread_ptr_->join();
//		thread_ptr_ = nullptr;
//	}
//
//	return 0;
//}

int DXGIScreenCapture::AquireFrame(cv::Mat& mat)
{
	Microsoft::WRL::ComPtr<IDXGIResource> dxgi_resource;
	DXGI_OUTDUPL_FRAME_INFO frame_info;
	memset(&frame_info, 0, sizeof(DXGI_OUTDUPL_FRAME_INFO));
		
	dxgi_output_duplication_->ReleaseFrame();
	HRESULT hr = dxgi_output_duplication_->AcquireNextFrame(0, &frame_info, dxgi_resource.GetAddressOf());
	
	if (FAILED(hr)) {
		if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
			return -1;
		}
		if (hr == DXGI_ERROR_INVALID_CALL) {
			return -1;
		}
		if (hr == DXGI_ERROR_ACCESS_LOST) {
			return -1;
		}
		
		return -1;
	}

	if (frame_info.AccumulatedFrames == 0 || 
		frame_info.LastPresentTime.QuadPart == 0) {
		// No image update, only cursor moved.
	}

	if (!dxgi_resource.Get()) {
		return -1;
	}

	Microsoft::WRL::ComPtr<ID3D11Texture2D> outputTexture;
	hr = dxgi_resource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void **>(outputTexture.GetAddressOf()));
	if (FAILED(hr)) {
		return -1;
	}
	image_size_ = dxgi_desc_.ModeDesc.Width * dxgi_desc_.ModeDesc.Height * 4;

	D3D11_MAPPED_SUBRESOURCE dsec = { 0 };
	d3d11_context_->CopyResource(gdi_texture_.Get(), outputTexture.Get());
	
	Microsoft::WRL::ComPtr<IDXGISurface1> surface1;
	hr = gdi_texture_->QueryInterface(__uuidof(IDXGISurface1), reinterpret_cast<void **>(surface1.GetAddressOf()));
	if (FAILED(hr)) {
		return -1;
	}

	CURSORINFO cursorInfo = { 0 };
	cursorInfo.cbSize = sizeof(CURSORINFO);
	if (GetCursorInfo(&cursorInfo) == TRUE) {
		if (cursorInfo.flags == CURSOR_SHOWING) {
			auto cursorPosition = cursorInfo.ptScreenPos;
			auto lCursorSize = cursorInfo.cbSize;
			HDC  hdc;
			surface1->GetDC(FALSE, &hdc);
			DrawIconEx(hdc, cursorPosition.x - monitor_.left, cursorPosition.y - monitor_.top, 
				cursorInfo.hCursor, 0, 0, 0, 0, DI_NORMAL | DI_DEFAULTSIZE);
			surface1->ReleaseDC(nullptr);
		}
	}

	d3d11_context_->CopyResource(rgba_texture_.Get(), gdi_texture_.Get());
	hr = d3d11_context_->Map(rgba_texture_.Get(), 0, D3D11_MAP_READ, 0, &dsec);
	if (!FAILED(hr)) {
		if (dsec.pData != NULL) {
			//int ret;
			uint32_t image_width = GetWidth();
			uint32_t image_height = GetHeight();
			
			cv::Mat bgra(image_height, image_width, CV_8UC4, dsec.pData);

			cv::cvtColor(bgra, mat, cv::COLOR_BGRA2BGR);

			//if (tmp_->width != image_width || tmp_->height != image_height)
			//{
			//	av_frame_unref(tmp_);
			//	tmp_->width = image_width;
			//	tmp_->height = image_height;
			//	tmp_->format = AV_PIX_FMT_YUV420P;
			//	tmp_->sample_aspect_ratio = { 1,1 };
			//	if ((ret = av_frame_get_buffer(tmp_, 0)) < 0)
			//		return ret;
			//}
			////int yuv420size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, image_width, image_height, 1);
			////image_ptr_.reset(new uint8_t[yuv420size], std::default_delete<uint8_t[]>());
			//int in_line[4];// = { image_width ,image_width ,image_width,image_width ,0,0,0,0 };
			//uint8_t* in_data[4];
			////int out_line[4];// = { image_width ,image_width / 2 ,image_width / 2,0,0,0,0,0 };
			////uint8_t* out_data[4];
			//int size1 = av_image_fill_arrays(in_data, in_line, (uint8_t*)dsec.pData, AV_PIX_FMT_BGRA, image_width, image_height, 1);
			////size1 =av_image_fill_arrays(out_data, out_line, image_ptr_.get(), AV_PIX_FMT_YUV420P, image_width, image_height, 1);
			//if ((ret = av_frame_make_writable(tmp_)) != 0)
			//	return ret;

			////std::lock_guard<std::mutex> locker(mutex_);
			//if (m_con.init(AV_PIX_FMT_BGRA, image_width, image_height, AV_PIX_FMT_YUV420P, image_width, image_height))
			//{
			//	if (m_con(in_data, in_line, tmp_->data, tmp_->linesize))
			//	{
			//		AVFrame* tmp = nullptr;
			//		{
			//			std::lock_guard<std::mutex>lock(mutex_);
			//			av_frame_unref(yuv420p_);
			//			av_frame_ref(yuv420p_, tmp_);
			//		}
			//		tmp = av_frame_clone(yuv420p_);
			//		{
			//			std::lock_guard<std::mutex>lock(mutex_fun_);
			//			if (m_captureFrameCallBack)
			//				(*m_captureFrameCallBack)(tmp);
			//		}
			//	}
			//}
		}
		d3d11_context_->Unmap(rgba_texture_.Get(), 0);
	}

	hr = keyed_mutex_->AcquireSync(0, 5);
	if (hr != S_OK) {
		return 0;
	}
	d3d11_context_->CopyResource(shared_texture_.Get(), rgba_texture_.Get());
	keyed_mutex_->ReleaseSync(key_);
	return 0;
}

//bool DXGIScreenCapture::CaptureFrame(std::vector<uint8_t>& yuv420p_image, uint32_t& width, uint32_t& height)
//{
//	std::lock_guard<std::mutex> locker(mutex_);
//
//	if (!is_started_) {
//		yuv420p_image.clear();
//		return false;
//	}
//
//	if (image_ptr_ == nullptr || image_size_ == 0) {
//		yuv420p_image.clear();
//		return false;
//	}
//
//	if (yuv420p_image.capacity() < image_size_) {
//		yuv420p_image.reserve(image_size_);
//	}
//
//	yuv420p_image.assign(image_ptr_.get(), image_ptr_.get() + image_size_);
//	width = dxgi_desc_.ModeDesc.Width;
//	height = dxgi_desc_.ModeDesc.Height;
//	return true;
//}
//
//bool DXGIScreenCapture::CaptureFrame(std::shared_ptr<uint8_t>& yuv420p_image, uint32_t& width, uint32_t& height)
//{
//	std::lock_guard<std::mutex> locker(mutex_);
//
//	if (!is_started_) {
//		yuv420p_image.reset();
//		return false;
//	}
//
//	if (image_ptr_ == nullptr || image_size_ == 0) {
//		yuv420p_image.reset();
//		return false;
//	}
//
//	yuv420p_image = image_ptr_;
//	width = dxgi_desc_.ModeDesc.Width;
//	height = dxgi_desc_.ModeDesc.Height;
//	return true;
//}

//bool DXGIScreenCapture::CaptureFrame_withOutLock(std::shared_ptr<uint8_t>& bgra_image, uint32_t& width, uint32_t& height)
//{
//	if (!is_started_) {
//		return false;
//	}
//
//	if (image_ptr_ == nullptr || image_size_ == 0) {
//		return false;
//	}
//	bgra_image = image_ptr_;
//	width = dxgi_desc_.ModeDesc.Width;
//	height = dxgi_desc_.ModeDesc.Height;
//	return true;
//}

//void DXGIScreenCapture::setCaptureFrameCallBack(const std::function<void(AVFrame*)>&fun)
//{
//	std::lock_guard<std::mutex>lock(mutex_fun_);
//	m_captureFrameCallBack = std::make_unique<std::remove_cv<std::remove_reference<decltype(fun)>::type>::type>(fun);
//}
//bool DXGIScreenCapture::GetTextureHandle(HANDLE* handle, int* lock_key, int* unlock_key)
//{
//	if (texture_handle_ == nullptr) {
//		return false;
//	}
//
//	*handle = texture_handle_;
//	key_ = *lock_key = 1;
//	unlock_key = 0;
//	return true;
//}

//bool DXGIScreenCapture::CaptureImage(std::string pathname)
//{
//	std::lock_guard<std::mutex> locker(mutex_);
//
//	if (!is_started_) {
//		return false;
//	}
//
//	if (image_ptr_ == nullptr || image_size_ == 0) {
//		return false;
//	}
//	
//	std::ofstream fpOut(pathname.c_str(), std::ios::out | std::ios::binary);
//	if (!fpOut) {
//		printf("[DXGIScreenCapture] capture image failed, open %s failed.\n", pathname.c_str());
//		return false;
//	}
//
//	unsigned char fileHeader[54] = {  
//		0x42, 0x4d, 0, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0,  /*file header*/
//		40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 32, 0, /*info header*/
//		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0, 0, 0, 0
//	};
//
//	uint32_t imageWidth  = dxgi_desc_.ModeDesc.Width;
//	uint32_t imageHeight = dxgi_desc_.ModeDesc.Height;
//	uint32_t imageSize  = image_size_;
//	uint32_t fileSize = sizeof(fileHeader) + imageSize;
//
//	fileHeader[2] = (uint8_t)fileSize;
//	fileHeader[3] = fileSize >> 8;
//	fileHeader[4] = fileSize >> 16;
//	fileHeader[5] = fileSize >> 24;
//
//	fileHeader[18] = (uint8_t)imageWidth;
//	fileHeader[19] = imageWidth >> 8;
//	fileHeader[20] = imageWidth >> 16;
//	fileHeader[21] = imageWidth >> 24;
//
//	fileHeader[22] = (uint8_t)imageHeight;
//	fileHeader[23] = imageHeight >> 8;
//	fileHeader[24] = imageHeight >> 16;
//	fileHeader[25] = imageHeight >> 24;
//
//	fpOut.write((char*)fileHeader, 54);
//
//	char *imageData = (char *)image_ptr_.get();
//	for (int h = imageHeight-1; h >= 0; h--) {
//		fpOut.write(imageData+h*imageWidth*4, imageWidth *4);
//	}
//
//	fpOut.close();
//	return true;
//}