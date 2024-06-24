// PHZ
// 2020-11-20

#ifndef DXGI_SCREEN_CAPTURE_H
#define DXGI_SCREEN_CAPTURE_H

#include "VideoCapture.h"
#include "WindowHelper.h"
#include <cstdio>
#include <cstdint>
#include <string>
#include <mutex>
#include <thread>
#include <memory>
#include <vector>
#include <wrl.h>
#include <dxgi.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include<functional>
#include<memory>
#include<opencv2/opencv.hpp>
#include "DXGIScreenCapture.h"
//#include "imageConvert.h"

//struct AVFrame;


class DXGIScreenCapture //: public VideoCapture
{
public:
	DXGIScreenCapture();
	virtual ~DXGIScreenCapture();

	bool Init(int display_index = 0);
	bool Destroy();

	uint32_t GetWidth()  const { return dxgi_desc_.ModeDesc.Width; }
	uint32_t GetHeight() const { return dxgi_desc_.ModeDesc.Height; }

	//bool CaptureFrame(std::vector<uint8_t>& yuv420p_image, uint32_t& width, uint32_t& height);
	//bool CaptureFrame(std::shared_ptr<uint8_t>& yuv420p_image, uint32_t& width, uint32_t& height);
	//bool CaptureFrame(AVFrame*f)override;
	//bool CaptureFrame_withOutLock(std::shared_ptr<uint8_t>& bgra_image, uint32_t& width, uint32_t& height);
	//void setCaptureFrameCallBack(const std::function<void(std::shared_ptr<uint8_t>& bgra_image, const uint32_t& width, const uint32_t& height)>&fun);
	//void setCaptureFrameCallBack(const std::function<void(AVFrame*)>&fun);
	//bool GetTextureHandle(HANDLE* handle, int* lockKey, int* unlockKey);
	//bool CaptureImage(std::string pathname);
	//ID3D11Device* GetD3D11Device() { return d3d11_device_.Get(); }
	//ID3D11DeviceContext* GetD3D11DeviceContext() { return d3d11_context_.Get(); }

	/*bool CaptureStarted() const
	{
		return is_started_;
	}*/
	int AquireFrame(cv::Mat&mat);
private:
	//int StartCapture();
	//int StopCapture();
	int CreateSharedTexture();

	DX::Monitor monitor_;

	bool is_initialized_;
	//bool is_started_;
	std::unique_ptr<std::thread> thread_ptr_;

	std::mutex mutex_;
	//AVFrame* yuv420p_ = nullptr; // bgra
	//AVFrame* tmp_ = nullptr;
	uint32_t image_size_;

	// d3d resource
	DXGI_OUTDUPL_DESC dxgi_desc_;
	HANDLE texture_handle_;
	int key_;
	Microsoft::WRL::ComPtr<ID3D11Device> d3d11_device_;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3d11_context_;
	Microsoft::WRL::ComPtr<IDXGIOutputDuplication> dxgi_output_duplication_;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> shared_texture_;
	Microsoft::WRL::ComPtr<IDXGIKeyedMutex> keyed_mutex_;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> rgba_texture_;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> gdi_texture_;
	//std::unique_ptr<std::function<void(std::shared_ptr<uint8_t>&, const uint32_t&, const uint32_t&)>>m_captureFrameCallBack;
	std::mutex mutex_fun_;
	//std::unique_ptr<std::function<void(AVFrame*)>>m_captureFrameCallBack;
	//imageConvert m_con;
};

#endif