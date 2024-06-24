#ifndef CORE_AUDIO_CAPTURE_H
#define CORE_AUDIO_CAPTURE_H

#include <thread>
#include <cstdint>
#include <memory>
#include "AudioCapture/WASAPICapture.h"
#include "AudioCapture/WASAPIPlayer.h"
#include "AudioBuffer.h"

class AudioCapture
{
public:
	AudioCapture();
	virtual ~AudioCapture();

	bool Init(WASAPICapture::CAPTURE_TYPE t = WASAPICapture::CAPTURE_TYPE::DESKTOP, uint32_t buffer_size = 20480);
	void Destroy();
	
	int Read(uint8_t*data,uint32_t samples);
	int GetSamples();

	uint32_t GetSamplerate() const
	{ return samplerate_; }

	uint32_t GetChannels()const
	{ return channels_; }

	uint32_t GetBitsPerSample()const
	{ return bits_per_sample_; }

	bool CaptureStarted() const
	{ return is_started_; }



	long long read_samples = 0;
	long long read_times = 0;

private:
	int StartCapture();
	int StopCapture();
	
	bool is_initialized_ = false;
	bool is_started_ = false;

	/*uint32_t channels_ = 2;
	uint32_t samplerate_ = 48000;
	uint32_t bits_per_sample_ = 16;*/
	uint32_t channels_ = 0;
	uint32_t samplerate_ = 0;
	uint32_t bits_per_sample_ = 0;
	//WASAPIPlayer player_;
	WASAPICapture capture_;
	std::unique_ptr<AudioBuffer> audio_buffer_;
};

#endif