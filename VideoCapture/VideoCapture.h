#ifndef SCREEN_CAPTURE_H
#define SCREEN_CAPTURE_H
#include <cstdint>
#include <vector>
#include<memory>
#include<functional>
struct AVFrame;
class VideoCapture
{
public:
	VideoCapture& operator=(const VideoCapture&) = delete;
	VideoCapture(const VideoCapture&) = delete;
	VideoCapture() {}
	virtual ~VideoCapture() {}
	virtual bool Init(int display_index = 0) = 0;
	virtual bool Destroy() = 0;
	virtual bool  CaptureFrame(AVFrame* f) = 0;
	virtual void setCaptureFrameCallBack(const std::function<void(AVFrame*)>& fun) = 0;
	virtual uint32_t GetWidth()  const = 0;
	virtual uint32_t GetHeight() const = 0;
	virtual bool CaptureStarted() const = 0;
protected:
};

#endif