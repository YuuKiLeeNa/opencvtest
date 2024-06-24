#pragma once
#include<vector>
#include<opencv2/opencv.hpp>
#include<map>
#include "DXGIScreenCapture.h"
#include<vector>
#include<chrono>
#include "opencv2/shape.hpp"
#include "AudioCapture/AudioCapture.h"

//extern double THRESHOLD_VALUE = 0.45;
#define MIN_VALUE 0.4

class cmatch
{
#define MAX_DURING_TIME 31LL
public:

	virtual ~cmatch();
	bool init();
	void begin();

//protected:
	void operator()(cv::Mat&mat);
	void setFishIcon(const std::vector<std::string>&iconPaths);
	//void CursorPosition(int x,int y);
	void LeftButtonClick(int x, int y);
	void RightButtonClick(int x, int y);
	void SrceenCursorInfo();
	void setCursorPosition(int x, int y);
	void* curCursorPosition();
	std::map<void*, std::tuple<std::vector<cv::Point2i>, int>> getMap();
	std::pair<int, int>getScreenWH();

	void* getIconPointer(int x, int y);
	void* getIconPointer(const cv::Rect& rect);
	void* getIconNormal();

	std::pair<int, int>getCursorWH();
	//cv::Mat getIcon(cv::Mat m, int x, int y);
	//cv::Mat getCursorIcon(int x, int y);
	bool judgeWaterFlower(cv::Mat& mat);
	void resetDstRectAndHist();
	static std::string getCurrentTime();
	void beginingFishing();
	bool judgeFishingTimeOut();
	void loadCmpIcon();



	static std::vector<cv::Point>getBinMatPoints(const cv::Mat& mat);
	static cv::Mat getForeground(cv::Mat src, const cv::Rect&rect);
	static cv::Mat getForegroundBin(cv::Mat src, const cv::Rect&rect);


	static std::tuple<std::vector<cv::Rect>, std::vector<cv::KeyPoint>, std::vector<int>>getConnectedRectV(cv::Mat src);
	static std::pair<cv::Rect, cv::KeyPoint>getConnectedRect(cv::Mat src);

	static std::vector<cv::Point> getForegroundPoints(const cv::Mat&src, const cv::Rect&rect);
	static float calcK(const cv::Point2f& p1, const cv::Point2f& p2);
	static float calcK(const cv::Vec4f&pts);

	static bool triRect(cv::Mat m, cv::Rect& rect);

	//static LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

	int matchShape(const std::vector<cv::Point>& sets);
	int matchShape(const cv::Mat& src, const cv::Rect&rect);
	bool isMatched(const cv::Mat& src, const cv::Rect&rect);
	cv::Mat getTri() {return m_tri;}
	cv::Mat getHand() {return m_hand;}
	cv::Mat getDao() {return m_dao;}
	void readCfg();
	bool judgeOnFish(cv::Mat&mat);
	bool judgeOnFish();
	void fishing();

	bool isOnFishing(std::vector<cv::Vec4f>&&sets);

protected:
	std::vector<cv::Mat>m_yupiao;
	std::vector<cv::Mat>m_matchMat;
	std::map<void*, std::tuple<std::vector<cv::Point2i>, int>>m_cursor_count;
	DXGIScreenCapture m_dxgi;
	cv::Rect m_rect;
	cv::Rect m_yumaoRect;
	std::vector<float>m_vhist;
	time_t m_time_begin = 0;

	cv::Mat m_tri;
	cv::Mat m_hand;
	cv::Mat m_dao;

	std::vector<std::vector<cv::Point>>m_iconPointsSet;

	int m_iconx = 0;
	int m_icony = 0;

	int m_waterFlowerThreshold = 200;
	int m_waterHis = 100;

	cv::Ptr <cv::ShapeContextDistanceExtractor> mysc = cv::createShapeContextDistanceExtractor();
	std::vector<std::vector<cv::Vec4f>>m_ptSets;
	//std::vector<float>m_ptMaxYSets;
	//cv::Mat m_frameBeginStartFishing;
	AudioCapture m_audio;
	bool m_isTri = false;
	cv::Mat m_triYupiao;
	cv::Mat z_noise;
	//HHOOK m_hook = NULL;
	//static bool m_bStop;
};

