#include<iostream>
#include<opencv2/opencv.hpp>
#include<memory>
#include<type_traits>
#include<vector>
#include<map>
#include "cmatch.h"
#include<chrono>
//#include<windows.h>


//int g_d = 0;
//int g_sigmaColor = 0;
//int g_sigmaSpace = 0;
//int g_times = 0;
//
//cv::Mat g_src;
//cv::Mat g_dst;
//
//#define X 308
//#define Y 385
//#define B_WIDTH 448
//#define B_HEIGHT 448
//#define ims(X) cv::imshow(#X, X)
//
//std::string path[] = {"0.jpg","1.jpg","2.jpg"};
//#define P_SIZE (sizeof(path)/sizeof(std::string)+1)
//
//cv::Mat g_averageMat;
//cv::Mat g_absDiffMat;
//cv::Mat g_lowMat;
//cv::Mat g_hiMat;
//cv::Mat g_tmp;
//cv::Mat g_tmp2;
//long long g_count = 0;
//
//bool b_init = false;
//


int main(int argc, char* argv[])
{
	Sleep(2000);
	cmatch m;
	if (m.init())
		m.begin();

	/*m.loadCmpIcon();
	cv::imshow("tri", m.getTri());
	cv::imshow("hand", m.getHand());
	cv::imshow("dao", m.getDao());
	cv::waitKey();*/
	std::cout << "failed" << std::endl;
	Sleep(200000);
	return 0;
}
