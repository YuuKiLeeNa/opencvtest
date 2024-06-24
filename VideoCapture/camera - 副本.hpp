#pragma once
#include <vector>
#include <functional>
#include <dshow.h>
#include <windows.h>
#include "qedit.h"
#include<string>

class Camera
{
public:
	Camera();
	Camera(const Camera &) = delete;
	Camera& operator  =(const Camera&) = delete;
	virtual ~Camera();
public:
	/*static Camera *GetInstance(void)
	{
		static Camera instance;
		return &instance;
	}*/

	std::vector<std::wstring> EnumAllCamera(void);

	bool Open(const std::wstring &camera_name);
	bool Close(void);

	/*!
	* @param time : Starting time of the sample, in seconds.
	* @param buff : Pointer to a buffer that contains the sample data.
	* @param len  : Length of the buffer pointed to by pBuffer, in bytes.
	*/
	void SetCallBack(const std::function<void(double time, BYTE *buff, LONG len)>& fun);

	int GetHeight()const { return mVideoHeight; }
	int GetWidth()const { return mVideoWidth; }
	int GetBitDepth()const { return mBitDepth; }
	
private:
	bool mInitOK;
	bool mIsVideoOpened;

	int mVideoWidth, mVideoHeight, mBitDepth;
	std::function<void(double, BYTE*, LONG)> mFrameCallBack;

private:
	class SampleGrabberCallback : public ISampleGrabberCB
	{
	public:
		ULONG STDMETHODCALLTYPE AddRef();
		ULONG STDMETHODCALLTYPE Release();
		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);
		HRESULT STDMETHODCALLTYPE SampleCB(double Time, IMediaSample* pSample);
		HRESULT STDMETHODCALLTYPE BufferCB(double Time, BYTE* pBuffer, long BufferLen);
		std::function<void(double, BYTE*, LONG)> mNewDataCallBack;
	};
	IGraphBuilder* mGraphBuilder;
	ICaptureGraphBuilder2* mCaptureGB;
	IMediaControl* mMediaControl;
	IBaseFilter* mDevFilter;
	ISampleGrabber* mSampGrabber;
	IMediaEventEx* mMediaEvent;

	SampleGrabberCallback mSampleGrabberCB;
	HRESULT InitializeEnv();
	HRESULT BindFilter(int deviceID, IBaseFilter** pBaseFilter);

};