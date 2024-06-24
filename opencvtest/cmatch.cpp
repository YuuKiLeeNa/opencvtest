#include "cmatch.h"
#include<Windows.h>
#include<fstream>
#include<functional>
#include<algorithm>
#include<numeric>
#include<cmath>
double THRESHOLD_VALUE = 0.45;
cmatch::~cmatch()
{
	/*if (m_hook)
	{
		UnhookWindowsHookEx(m_hook);
		m_hook = NULL;
	}*/
}
bool cmatch::init()
{
	//m_hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);  // ��װȫ�ּ��̹ҹ�
	m_vhist.reserve(3000);
	setFishIcon({ "yupiao.jpg" });
	if (m_yupiao.empty())
	{
		std::cout << "couldn't find yupiao.jpg" << std::endl;
		return false;
	}
	if (!m_dxgi.Init()) 
	{
		std::cout << "dxgi initialize failed" << std::endl;
		return false;
	}
	getCursorWH();
	loadCmpIcon();
	readCfg();
	m_audio.Init();

	return true;
}

void cmatch::begin()
{
	cv::Mat mat;
	while (true)
	{
		/*if (m_bStop)
		{
			Sleep(2000);
			continue;
		}*/
		if (m_dxgi.AquireFrame(mat) == 0)
			(*this)(mat);
	}
}

void cmatch::operator()(cv::Mat& mat)
{
	/*cv::Mat mat;
	m_dxgi.AquireFrame(mat);*/
	if (mat.empty())
		return;
	//try {
	long long diff = time(NULL) - m_time_begin;
	if (judgeFishingTimeOut() || diff < 2LL)
	{
		Sleep(2000);
			//if (cv::waitKey(2000) == 'b')
			//	exit(0);
		return;
	}
	//}
	/*catch (std::exception& e) 
	{
		std::cout << e.what() << std::endl;
	}*/

	if (m_rect == cv::Rect())
	{
		cv::Mat res;
		mat.resize(mat.rows / 2);
		m_matchMat.clear();
		for (auto& ele : m_yupiao)
		{
			try
			{
				cv::matchTemplate(mat, m_isTri ? m_triYupiao : ele, res, cv::TM_CCOEFF_NORMED);
			}
			catch (std::exception* p) 
			{
				std::cout<<p->what()<<std::endl;
				return;
			}
			m_matchMat.push_back(res);
		}
		std::map<float, std::tuple<cv::Rect, cv::Mat, int>, std::greater<float>>points;
		//std::map<float, cv::Rect>points;
		//	59	32
		//col:1862	row:1049
		std::pair<float, std::tuple<cv::Rect, cv::Mat, int>>max_value = { 0.0,{cv::Rect(),cv::Mat(), 0} };
		assert(res.type() == CV_32FC1);

#define ELE_RECT(x) std::get<0>(x->second)
#define ELE_MAT(x)	std::get<1>(x->second)
#define ELE_COUNT(x)	std::get<2>(x->second)

#define ELE_RECT_DOT(x) std::get<0>(x.second)
#define ELE_MAT_DOT(x)	std::get<1>(x.second)
#define ELE_COUNT_DOT(x)	std::get<2>(x.second)


		for (int i = 0; i < m_matchMat.size(); ++i)
		{
			auto& res = m_matchMat[i];

			for (int cols = 0; cols < res.cols; ++cols)
				for (int rows = 0; rows < res.rows; ++rows)
				{
					float v = *res.ptr<float>(rows, cols);
					if (max_value.first < v)
						max_value = { v ,{cv::Rect(cols,rows, m_yupiao[i].cols, m_yupiao[i].rows), (cv::Mat)m_yupiao[i], 0} };
					if (v > THRESHOLD_VALUE)
						points.emplace(std::move(v), std::tuple<cv::Rect, cv::Mat, int>{ cv::Rect(cols, rows, m_yupiao[i].cols, m_yupiao[i].rows), cv::Mat(m_yupiao[i]), 0 });
				}
		}


		int itimes = 0;
		if (!points.empty())
		{
			for (auto iter = points.begin(); iter != --points.end() && iter != points.end(); )
			{
				cv::Rect r = ELE_RECT(iter);
				bool bIsDelete = false;
				for (auto secondIter = std::next(iter); secondIter != points.end(); )
				{
					if (std::abs(r.x - ELE_RECT(secondIter).x) <= ELE_MAT(secondIter).cols  && std::abs(r.y - ELE_RECT(secondIter).y) < ELE_MAT(secondIter).rows )
					{
						secondIter = points.erase(secondIter);
						bIsDelete = true;
						++ELE_COUNT(iter);
					}
					else
						++secondIter;
				}
				++itimes;
				if (bIsDelete)
					iter = std::next(points.begin(), itimes);
				else
					++iter;
			}
		}
		else if(!m_isTri)
		{
			//m_time_begin -= 30;
			THRESHOLD_VALUE -= 0.5;
			if (THRESHOLD_VALUE <= 0.001)
				THRESHOLD_VALUE = 0.001;
		}
		/*if (points.empty())
			m_isTri = false;*/
		int ptsize = points.size();

		if (ptsize > 100) 
		{
			THRESHOLD_VALUE += 0.25;
			if (THRESHOLD_VALUE > 0.45)
				THRESHOLD_VALUE = 0.45;
		}
		if (ptsize > 10)
			points.erase(std::next(points.begin(), 10), points.end());
		/*else
		{
			if (points.empty() && max_value.first > MIN_VALUE)
				points.emplace(std::move(max_value));
		}*/

		//int times = 0;
		//auto tmp1 = tmp.clone();
		int times = 0;



		//void* pNormal = getIconNormal();

		cv::Rect ddrect;
		for (auto& ele : points)
		{
			if(time(NULL) - m_time_begin > MAX_DURING_TIME - 15)
			{
				m_time_begin -= 20;
				break;
			}
			//void* pRectCursorHandle = getIconPointer(ELE_RECT_DOT(ele));
			//if (pNormal != NULL && pRectCursorHandle != NULL && pNormal == pRectCursorHandle)
			//	continue;
			//getIcon(mat,ELE_RECT_DOT(ele).x-50, ELE_RECT_DOT(ele).y-50);
			///matched!////////////////////////////
			cv::Point2i pt(ELE_RECT_DOT(ele).x + ELE_RECT_DOT(ele).width / 2, ELE_RECT_DOT(ele).y + ELE_RECT_DOT(ele).height / 2);
			SetCursorPos(pt.x,pt.y);
			Sleep(80);
			cv::Mat tmpMat;
			bool matched = false;
			int tryTimes = 0;
			while (m_dxgi.AquireFrame(tmpMat) != 0) 
			{
				if (++tryTimes > 5)
					break;
				Sleep(20);
			}

			if (tmpMat.empty())
			{
				std::cout << "dxgi aquire frames failed\n" << std::endl;
				continue;
			}
			cv::Rect rect(pt.x - 6, pt.y -  6, m_iconx*1.3, m_icony*1.3);
			/*if (rect.x < 0)
				rect.x = 0;
			if (rect.y < 0)
				rect.y = 0;
			if (rect.x + rect.width > tmpMat.cols)
			{
				rect.width = tmpMat.cols - rect.x;
				if (rect.width < m_iconx)
				{
					std::cout << "rect width > screen width" << std::endl;
					continue;
				}
			}
			if (rect.y + rect.height > tmpMat.rows)
			{
				rect.height = tmpMat.rows - rect.y;
				if (rect.height < m_icony)
				{
					std::cout << "rect height > screen height" << std::endl;
					continue;
				}
			}*/
			//edges ignores for simple
			if(rect.x < 0 || rect.y < 0 || rect.width + rect.x > tmpMat.cols || rect.height + rect.y > tmpMat.rows)
				continue;
			cv::Mat iconEdgeMat = tmpMat(rect);
			matched = isMatched(iconEdgeMat, cv::Rect(6, 6, m_iconx, m_icony));

			if (!matched)
				continue;

			if (!m_isTri) 
			{
				for (int i = 100; i < 250; i += 50)
				{
					cv::Rect rect1(ELE_RECT_DOT(ele).x - i, ELE_RECT_DOT(ele).y - i, 2 * i, 2 * i);
					if (cmatch::triRect(mat, rect1))
					{
						//cv::rectangle(tmp1, rect1 , cv::Scalar(0, 0, 255));
						//cv::rectangle(tmp1, cv::Rect(ELE_RECT_DOT(ele).x - 80, ELE_RECT_DOT(ele).y - 80, 160, 160),cv::Scalar(0, 0, 255));
						cv::Mat ddst = mat(rect1);
						cv::Mat bin = cmatch::getForegroundBin(ddst, cv::Rect(20, 20, 2*i - 40, 2*i - 40));
						auto ppa = cmatch::getConnectedRect(bin);
						if (ppa.first != cv::Rect())
						{
							cv::Rect yupiaosize = ppa.first;
							if (yupiaosize.width < 6 || yupiaosize.height < 6)
								break;
							//cv::Rect mydstrect(rect1.x + yupiaosize.x, rect1.y + yupiaosize.y, yupiaosize.width, yupiaosize.height);
							cv::Rect mydstrect(rect1.x + yupiaosize.x, rect1.y + yupiaosize.y+ (int)(yupiaosize.height*0.2), (int)(yupiaosize.width*0.8), (int)(yupiaosize.height*0.8));
							m_triYupiao = mat(mydstrect);
							if (!m_triYupiao.empty())
							{
								//cv::imshow("fetchedyupiao", fetchedyupiao);
								//ppa.second.pt.x += rect1.x;
								//ppa.second.pt.y += rect1.y;
								//cv::drawKeypoints(tmp1, { ppa.second }, tmp1, cv::Scalar(0, 0, 255));
								//cv::rectangle(tmp1, rect1, cv::Scalar(0, 0, 255));
								cv::imwrite("triyupiao.jpg", m_triYupiao);
								m_isTri = true;
								break;
							}
						}
					}
				}
			}

			//m_rect = cv::Rect(ELE_RECT_DOT(ele).x - ELE_MAT_DOT(ele).cols , ELE_RECT_DOT(ele).y + ELE_MAT_DOT(ele).rows , ELE_MAT_DOT(ele).cols *2, ELE_MAT_DOT(ele).rows);
			
			m_rect = cv::Rect(ELE_RECT_DOT(ele).x - 2*ELE_MAT_DOT(ele).cols, ELE_RECT_DOT(ele).y, ELE_RECT_DOT(ele).width / 2 + 2*ELE_MAT_DOT(ele).cols, ELE_MAT_DOT(ele).rows*3);
			if (m_rect.x < 0)
				m_rect.x = 0;
			if(m_rect.y < 0)
				m_rect.y = 0;
			if (m_rect.x + m_rect.width > mat.cols)
				m_rect.width = mat.cols - m_rect.x;
			if (m_rect.y + m_rect.height > mat.rows)
				m_rect.height = mat.rows - m_rect.y;
			if (m_rect.width <= 0 || m_rect.height <= 0)
			{
				m_rect = cv::Rect();
				continue;
			}

			
			
			m_yumaoRect = ELE_RECT_DOT(ele);
			//break;

			//ddrect = cv::Rect(ELE_RECT_DOT(ele).x - 2 * ELE_MAT_DOT(ele).cols / 3, ELE_RECT_DOT(ele).y + ELE_MAT_DOT(ele).rows * 0.5, ELE_MAT_DOT(ele).cols * 4 / 3, ELE_MAT_DOT(ele).rows);
			
			//cv::rectangle(tmpMat, cv::Rect(ELE_RECT_DOT(ele).x - 2 * ELE_MAT_DOT(ele).cols / 3, ELE_RECT_DOT(ele).y + ELE_MAT_DOT(ele).rows * 0.5, ELE_MAT_DOT(ele).cols * 4 / 3, ELE_MAT_DOT(ele).rows), cv::Scalar(0, 0, 255));
			//cv::rectangle(tmpMat, m_rect, cv::Scalar(0, 0, 255));
			//cv::rectangle(tmpMat, rect, cv::Scalar(0, 0, 255));
			//cv::imwrite(getCurrentTime() + ".jpg", tmpMat);
			break;
			//cv::rectangle(mat, cv::Rect(ELE_RECT_DOT(ele).x - 2*ELE_MAT_DOT(ele).cols/3, ELE_RECT_DOT(ele).y+ ELE_MAT_DOT(ele).rows*0.5, ELE_MAT_DOT(ele).cols*4/3 , ELE_MAT_DOT(ele).rows), cv::Scalar(0, 0, 255));
			//std::stringstream ss;
			//ss << std::hex << pNormal << " " << pRectCursorHandle;
			//cv::putText(mat, cv::String(std::to_string(times)+":" + std::to_string(ELE_COUNT_DOT(ele))) + ":" + std::to_string(ele.first), cv::Point(ELE_RECT_DOT(ele).x - ELE_MAT_DOT(ele).cols, ELE_RECT_DOT(ele).y), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255));
			//cv::putText(mat, ss.str(), cv::Point(ELE_RECT_DOT(ele).x - ELE_MAT_DOT(ele).cols, ELE_RECT_DOT(ele).y), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255));

			//mat.
			//cv::imwrite(getCurrentTime() + ".jpg", mat);
			//getIconNormal();


			//break;

			//cv::Mat mm = cv::Mat::zeros(yupiao_gray.size(), yupiao_gray.type());
			//if (times == 0)
			//	break;
			/*times++;
			if (times > 50)
				break;*/
		}
		if (m_rect == cv::Rect())
			m_isTri = false;
		//if (!points.empty())
		//	cv::imwrite(getCurrentTime() + ".jpg", mat);
	}

	//cv::Mat langhuaMat = mat(ddrect);

	
	if (m_rect != cv::Rect())
	{
		cv::Mat langhuaMat = mat(m_rect);
		judgeOnFish();
		//judgeOnFish(langhuaMat);
		//judgeWaterFlower(langhuaMat);
	}



	

	//std::cout << maxValue << std::endl;
	//cv::rectangle(tmp1, cv::Rect(min.x, min.y, yupiao_gray.cols, yupiao_gray.rows), cv::Scalar(0, 0, 255));
	//cv::imshow("TM_CCOEFF_NORMED", mat);

	////}
	//cv::waitKey(1);

#undef ELE_RECT
#undef ELE_MAT
#undef ELE_COUNT	

#undef ELE_RECT_DOT
#undef ELE_MAT_DOT
#undef ELE_COUNT_DOT


}

void cmatch::setFishIcon(const std::vector<std::string>& iconPaths)
{
	m_yupiao.clear();
	for (auto& ele : iconPaths)
	{
		cv::Mat mat = cv::imread(ele);
		if(!mat.empty())
			m_yupiao.emplace_back(std::move(mat));
	}
}

//void cmatch::CursorPosition(int x,int y)
//{
//	SetCursorPos(x, y); //�ƶ���굽x, yλ��
//	//CURSORINFO
//	// Get your device contexts.
//	//HDC hdcScreen = GetDC(NULL);
//	//HDC hdcMem = CreateCompatibleDC(hdcScreen);
//	//// Create the bitmap to use as a canvas.
//	//HBITMAP hbmCanvas = CreateCompatibleBitmap(hdcScreen, 256, 256);
//	//// Select the bitmap into the device context.
//	//HGDIOBJ hbmOld = SelectObject(hdcMem, hbmCanvas);
//	// Get information about the global cursor.
//	CURSORINFO ci;
//	ci.cbSize = sizeof(ci);
//	if (!GetCursorInfo(&ci))
//		return;
//
//	//std::cout << std::hex << ci.hCursor<<std::endl;
//
//	++std::get<1>(m_cursor_count[ci.hCursor]);
//	std::get<0>(m_cursor_count[ci.hCursor]).emplace_back(x,y);
//
//}

void cmatch::LeftButtonClick(int x, int y)
{
	//if(!m_bStop)
		mouse_event(MOUSEEVENTF_LEFTDOWN, x, y, 0, 0); //�������
	//mouse_event(MOUSEEVENTF_RIGHTDOWN, x, y, 0, 0); //�Ҽ�����
}

void cmatch::RightButtonClick(int x, int y)
{
	//if (!m_bStop)
	//{
		mouse_event(MOUSEEVENTF_RIGHTDOWN, x, y, 0, 0); //�Ҽ�����
		Sleep(5);
		mouse_event(MOUSEEVENTF_RIGHTUP, x, y, 0, 0); //�Ҽ�����
	//}
}

//void cmatch::SrceenCursorInfo()
//{
//	int srcxsize = GetSystemMetrics(SM_CXSCREEN);//��ȡ��Ļ�豸�ߴ���Ϣ
//	int srcysize = GetSystemMetrics(SM_CYSCREEN);
//
//	for (int x = 0; x < srcxsize; x=x+10)
//		for (int y = 0; y < srcysize; y = y + 10)
//		{
//			//Sleep(500);
//			CursorPosition(x, y);
//		}
//
//
//}

void cmatch::setCursorPosition(int x, int y)
{
	//if(!m_bStop)
		SetCursorPos(x, y);
}

void* cmatch::curCursorPosition()
{
	CURSORINFO ci;
	ci.cbSize = sizeof(ci);

	if (GetCursorInfo(&ci))
	{
		std::cout <<std::hex <<ci.cbSize << " " << ci.flags <<" "<< ci.hCursor<<std::endl;
		return ci.hCursor;
	}
	return NULL;
}

std::map<void*, std::tuple<std::vector<cv::Point2i>, int>> cmatch::getMap()
{
	return m_cursor_count;
}

std::pair<int, int> cmatch::getScreenWH()
{
	int srcxsize = GetSystemMetrics(SM_CXSCREEN);//��ȡ��Ļ�豸�ߴ���Ϣ
	int srcysize = GetSystemMetrics(SM_CYSCREEN);
	return std::pair<int, int>(srcxsize,srcysize);
}

void* cmatch::getIconPointer(int x, int y)
{
	SetCursorPos(x, y); //�ƶ���굽x, yλ��
	//cv::waitKey(1000);
	/*CURSORINFO ci;
	ci.cbSize = sizeof(ci);
	if (!GetCursorInfo(&ci))
		return NULL;*/
	
	//getCursorIcon(x,y);
	//auto p = getCursorWH();
	return NULL;//ci.hCursor;
}

void* cmatch::getIconPointer(const cv::Rect& rect)
{
	return getIconPointer(rect.x + rect.width / 2, rect.y + rect.height / 2);
}

void* cmatch::getIconNormal()
{
	auto xy = getScreenWH();
	int xoff = rand() % 30;
	int yoff = rand() % 30;

	return getIconPointer(xy.first/2 + xoff, xy.second/2 + yoff);
}

std::pair<int, int> cmatch::getCursorWH()
{
	//std::pair<int, int> ret11(0, 0);
	HDC hdc = GetDC(NULL);
	ICONINFO cursorInfo;
	int w = 0;
	int h = 0;
	//std::pair<int, int> ret11{0,0};
	//std::pair<int, int> ret12{ 0,0 };
	//std::pair<int, int> ret13{ 0,0 };
	if (GetIconInfo(LoadCursor(NULL, IDC_ARROW), &cursorInfo)) {
		

		BITMAP bmpinfo = { 0 };
		if (::GetObject(cursorInfo.hbmMask, sizeof(BITMAP), &bmpinfo) != 0)
		{
			w = bmpinfo.bmWidth;
			h = bmpinfo.bmHeight;

			m_iconx = w;
			m_icony = h;
			//std::pair<int, int> ret1{ bmpinfo.bmWidth ,bmpinfo.bmHeight };
			//return ret1;
		}
		DeleteObject(cursorInfo.hbmMask);
		DeleteObject(cursorInfo.hbmColor);
	}
	return  {w,h};
}

//cv::Mat cmatch::getIcon(cv::Mat m, int x, int y)
//{
//	getCursorWH();
//	cv::Mat icon = m(cv::Rect(x, y, m_iconx+100, m_icony+100));
//	cv::imwrite(getCurrentTime() + ".jpg", icon);
//	Sleep(1000);
//	return icon;
//}

//cv::Mat cmatch::getCursorIcon(int x, int y)
//{
//	SetCursorPos(x,y);
//
//	CURSORINFO cursorInfo = { 0 };
//	cursorInfo.cbSize = sizeof(cursorInfo);
//
//	HDC memoryDC = (HDC)malloc(48*48*25);
//	memset(memoryDC, 0x00, 48 * 48 * 25);
//	cv::Mat m;
//	if (::GetCursorInfo(&cursorInfo)) {
//		ICONINFO ii = { 0 };
//		//GetIconInfo(cursorInfo.hCursor, &ii);
//		if (GetIconInfo(cursorInfo.hCursor, &ii)) {
//
//			BITMAP bm;
//			GetObject(ii.hbmMask, sizeof(BITMAP), &bm);
//
//			
//			::DrawIcon(memoryDC, cursorInfo.ptScreenPos.x - ii.xHotspot, cursorInfo.ptScreenPos.y - ii.yHotspot, cursorInfo.hCursor);
//
//			m = cv::Mat(bm.bmHeight, bm.bmWidth, CV_8UC4);
//
//			for (int i = 0; i < bm.bmWidth; i++) {
//				for (int j = 0; j < bm.bmHeight; j++) {
//					COLORREF c = GetPixel(memoryDC, i, j);
//					printf("%x\n", c);
//					*m.ptr<cv::Vec4b>(j, i) = c;
//				}
//			}
//			cv::Mat ddst;
//			cv::cvtColor(m, ddst, cv::COLOR_RGBA2BGR);
//			cv::imwrite(getCurrentTime() + ".jpg", ddst);
//
//			DeleteObject(ii.hbmColor);
//			DeleteObject(ii.hbmMask);
//		}
//	}
//
//	return cv::Mat();
//}

bool cmatch::judgeWaterFlower(cv::Mat& mat)
{
	cv::Mat gray,bin;
	cv::cvtColor(mat, gray, cv::COLOR_BGR2GRAY);
	cv::threshold(gray, bin, m_waterFlowerThreshold, 255, cv::THRESH_BINARY);

	int ch[] = { 0 };
	float rangeH[] = { 255,256 };
	//float rangeS[] = { 0.0f,255.0f };
	const float* range[] = { rangeH };
	cv::Mat hist;
	int histSize[] = {1};
	cv::calcHist(&bin, 1, ch, cv::noArray(), hist, 1, histSize, range);
	int ttttt= hist.type();
	assert(hist.type() == CV_32F);
	auto v = *hist.ptr<float>(0,0);
	//std::cout << v << std::endl;
	//std::cout<<hist.ptr<int>
	//cv::calcHist()
	m_vhist.emplace_back(v);

	auto pa = std::minmax_element(m_vhist.cbegin(), m_vhist.cend());
	//auto diff = *pa.second - pa.first;
	if (*pa.second - *pa.first > m_waterHis && --m_vhist.cend() == pa.second)
	{
		//cv::imshow("text", bin);
		//cv::waitKey(0);
		/*setCursorPosition(m_yumaoRect.x + m_yumaoRect.width / 2, m_yumaoRect.y + m_yumaoRect.height / 2);
		RightButtonClick(m_yumaoRect.x + m_yumaoRect.width / 2, m_yumaoRect.y + m_yumaoRect.height / 2);
		Sleep(2000);
		beginingFishing();*/
		fishing();
		return true;
	}
	return false;
}

void cmatch::resetDstRectAndHist()
{
	m_rect = cv::Rect();
	m_yumaoRect = cv::Rect();
	m_vhist.clear();
	m_ptSets.clear();
	//m_ptMaxYSets.clear();
}

std::string cmatch::getCurrentTime()
{
	time_t nowtime;
	time(&nowtime); //��ȡ1970��1��1��0��0��0�뵽���ھ���������
	tm p;
	localtime_s(&p, &nowtime); //������ת��Ϊ����ʱ��,���1900����,��Ҫ+1900,��Ϊ0-11,����Ҫ+1
	char sz[128];
	snprintf(sz,sizeof(sz), "%04d%02d%02d %02d%02d%02d", p.tm_year + 1900, p.tm_mon + 1, p.tm_mday, p.tm_hour, p.tm_min, p.tm_sec);
	return std::string(sz);
}

void cmatch::beginingFishing()
{
	//INPUT input[2];
	//memset(input, 0, sizeof(input));
	////input[0].ki.wVk = 0x31;
	////input[0].type = INPUT_KEYBOARD;
	////input[0].ki.dwFlags = 0;//KEYEVENTF_UNICODE;
	//input[0].ki.wVk = 0x31;
	//input[0].type = INPUT_KEYBOARD;
	//input[0].ki.dwFlags = KEYEVENTF_KEYUP;
	//SendInput(1, input, sizeof(INPUT));
	/*int tryTimes = 0;
	cv::Mat tmpMat;
	while (m_dxgi.AquireFrame(tmpMat) != 0)
	{
		if (++tryTimes > 100)
		{
			std::cout << "aquire frame errors\n"<<std::endl;
			break;
		}
		Sleep(20);
	}*/
	//m_frameBeginStartFishing = tmpMat;
	

	//auto xy = getScreenWH();
	//int xoff = rand() % 30;
	//int yoff = rand() % 30;
	//int x = xy.first / 2 + xoff;
	//int y = xy.second / 2 + yoff;
	//setCursorPosition(x,y);
	//RightButtonClick(x,y);
	//if (m_bStop)
	//	return;


	::keybd_event(0x31, 0, 0, 0);
	Sleep(6);
	::keybd_event(0x31, 0, KEYEVENTF_KEYUP, 0);

	m_time_begin = time(NULL);
	resetDstRectAndHist();
	getIconNormal();
}

bool cmatch::judgeFishingTimeOut()
{
	if (time(NULL) - m_time_begin > MAX_DURING_TIME)
	{
		beginingFishing();
		return true;
	}
	return false;
}

void cmatch::loadCmpIcon()
{
	cv::Mat tri = cv::imread("1.jpg");
	cv::Mat hand = cv::imread("2.jpg");
	cv::Mat dao = cv::imread("3.jpg");
	if (tri.empty() || hand.empty() || dao.empty())
	{
		std::cout << "ȱ��ͼ��" << std::endl;
		exit(0);
	}

	m_tri = getForegroundBin(tri, cv::Rect(5,5, tri.cols -10, tri.rows - 10));
	m_hand = getForegroundBin(hand, cv::Rect(5, 5, hand.cols - 10, hand.rows - 10));
	m_dao = getForegroundBin(dao, cv::Rect(5, 5, dao.cols - 10, dao.rows - 10));

	m_iconPointsSet.clear();
	m_iconPointsSet.reserve(3);

	m_iconPointsSet.emplace_back(getBinMatPoints(m_tri));
	m_iconPointsSet.emplace_back(getBinMatPoints(m_hand));
	m_iconPointsSet.emplace_back(getBinMatPoints(m_dao));
}

std::vector<cv::Point> cmatch::getBinMatPoints(const cv::Mat& mat)
{
	cv::Mat edge;
	cv::Canny(mat, edge, 200, 210);

	//static int i = 0;
	//++i;
	//cv::imshow(std::to_string(i), edge);

	std::vector<std::vector<cv::Point>>psets;
	cv::findContours(edge, psets, cv::RetrievalModes::RETR_LIST, cv::ContourApproximationModes::CHAIN_APPROX_NONE);
	std::vector<cv::Point>sets;

	for (int i = 0; i < psets.size(); ++i)
		sets.insert(sets.cend(), std::make_move_iterator(psets[i].begin()), std::make_move_iterator(psets[i].end()));
	return sets;
}

cv::Mat cmatch::getForeground(cv::Mat src, const cv::Rect&rect) 
{
	cv::Mat mask = cv::Mat::zeros(cv::Size(src.rows, src.cols), CV_8UC1);
	//cv::Rect rect(36,90, 209,211);
	cv::Mat s1;// = cv::Mat::zeros(1, 65, CV_64F), 
	cv::Mat s2;// = cv::Mat::zeros(1, 65, CV_64F);
	cv::grabCut(src, mask, rect, s1, s2, 5, cv::GC_INIT_WITH_RECT);
	cv::Mat maskB = mask.clone();
	maskB.forEach<unsigned char>([](unsigned char& c, const int* position) {if (c == cv::GC_BGD || c == cv::GC_PR_BGD) c = 0; else c = 255; });
	//cv::Mat maskBclone = maskB.clone();
	//cv::Mat diflateMat = cv::getStructuringElement(cv::MORPH_DILATE, cv::Size(5, 5));
	//cv::Mat erodeMat = cv::getStructuringElement(cv::MORPH_ERODE, cv::Size(5, 5));
	//cv::erode(maskB, maskB, erodeMat);
	//cv::dilate(maskB, maskB, diflateMat);
	cv::Mat maskMorphology, mask3c;
	cv::merge(std::vector<cv::Mat>{maskB, maskB, maskB}, maskMorphology);
	//cv::merge(std::vector<cv::Mat>{maskBclone, maskBclone, maskBclone}, mask3c);
	cv::Mat dst;// , dst2;
	cv::bitwise_and(src, maskMorphology, dst);
	return dst;
}

cv::Mat cmatch::getForegroundBin(cv::Mat src, const cv::Rect& rect)
{
	cv::Mat dst = getForeground(src, rect);
	cv::Mat gray,bin;
	cv::cvtColor(dst, gray, cv::COLOR_BGR2GRAY);
	cv::threshold(gray, bin, 1, 255, cv::THRESH_BINARY);
	//cv::bitwise_and(src, mask3c, dst2);

	return bin;
}

std::tuple<std::vector<cv::Rect>, std::vector<cv::KeyPoint>, std::vector<int>> cmatch::getConnectedRectV(cv::Mat src)
{
	//cv::Mat srcDilate;
	//cv::Mat dilateMat5x5 = cv::getStructuringElement(cv::MorphShapes::MORPH_RECT, cv::Size(5, 5));
	//cv::dilate(src, srcDilate, dilateMat5x5);

	cv::Mat labelMat, statMat, centroidsMat;
	int nccomps = cv::connectedComponentsWithStats(src, labelMat, statMat, centroidsMat, 8);
	/*std::vector<cv::Vec3b>colorSets;
	colorSets.reserve(nccomps);
	colorSets.emplace_back(decltype(colorSets)::value_type(0, 0, 255));
	std::cout << "areas:" << nccomps << std::endl;*/

	//for (int i = 1; i < nccomps; ++i)
	//	colorSets.emplace_back(rand() % 255, rand() % 255, rand() % 255);


	std::vector<cv::Rect>rectSets;
	std::vector<int>areaSets;
	rectSets.reserve(nccomps);
	for (int i = 0; i < nccomps; ++i)
	{
		int area = statMat.at<int>(i, cv::CC_STAT_AREA);
		areaSets.emplace_back(area);
		//std::cout << area << std::endl;
		cv::Rect rect;
		rect.x = statMat.at<int>(i, cv::ConnectedComponentsTypes::CC_STAT_LEFT);
		rect.y = statMat.at<int>(i, cv::ConnectedComponentsTypes::CC_STAT_TOP);
		rect.width = statMat.at<int>(i, cv::ConnectedComponentsTypes::CC_STAT_WIDTH);
		rect.height = statMat.at<int>(i, cv::ConnectedComponentsTypes::CC_STAT_HEIGHT);
		rectSets.emplace_back(std::move(rect));
	}

	//cv::Mat color_map = cv::Mat::zeros(cv::Size(src.cols, src.rows), CV_8UC3);
	//for (int row = 0; row < color_map.rows; ++row)
	//	for (int col = 0; col < color_map.cols; ++col)
	//	{
	//		int nc = labelMat.at<int>(row, col);
	//		assert(nc < nccomps);
	//		color_map.at<cv::Vec3b>(row, col) = colorSets[nc];
	//	}

	//cv::imshow("srcx", src);
	/*for (auto& ele : rectSets)
	{
		cv::rectangle(src, ele, cv::Scalar(0, 0, 255));
	}*/

	std::vector<cv::KeyPoint>psets;
	psets.reserve(centroidsMat.rows);
	for (int r = 0; r < centroidsMat.rows; ++r)
	{
		psets.emplace_back(cv::Point2f(centroidsMat.at<double>(r, 0), centroidsMat.at<double>(r, 1)), 1);
	}


	return std::make_tuple<std::vector<cv::Rect>, std::vector<cv::KeyPoint>, std::vector<int>>(std::move(rectSets), std::move(psets), std::move(areaSets));
}

std::pair<cv::Rect, cv::KeyPoint> cmatch::getConnectedRect(cv::Mat src)
{
	auto r = getConnectedRectV(src);
	if(std::get<0>(r).size() > 1)
		return std::pair<cv::Rect, cv::KeyPoint>(std::get<0>(r)[1], std::get<1>(r)[1]);
	return std::pair<cv::Rect, cv::KeyPoint>();
}

std::vector<cv::Point> cmatch::getForegroundPoints(const cv::Mat& src, const cv::Rect& rect)
{
	cv::Mat mat = getForegroundBin(src, rect);
	//static int i = 0;
	//cv::imwrite(std::to_string(++i)+"edges.jpg",mat);
	return getBinMatPoints(mat);
}

float cmatch::calcK(const cv::Point2f& p1, const cv::Point2f& p2)
{
	return (p1.y - p2.y) / (p1.x - p2.x);
}

float cmatch::calcK(const cv::Vec4f& pts)
{
	return calcK({ pts[0], pts[1] }, { pts[2], pts[3] });
}

bool cmatch::triRect(cv::Mat m, cv::Rect& rect)
{
	if (m.empty() || rect.x >= m.cols || rect.y >= m.rows)
		return false;
	cv::Rect tmp = rect;
	if (tmp.x < 0)
		tmp.x = 0;
	if (tmp.y < 0)
		tmp.y = 0;

	if (tmp.x + tmp.width > m.cols)
		tmp.width = m.cols - tmp.x;
	if (tmp.y + tmp.height > m.rows)
		tmp.height = m.rows - tmp.y;

	if (tmp.width <= 0 || tmp.height <= 0)
		return false;
	rect = tmp;
	return true;
}

//LRESULT CALLBACK cmatch::KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
//{
//	if (nCode == HC_ACTION)
//	{
//		KBDLLHOOKSTRUCT* pKeyboardHookStruct = (KBDLLHOOKSTRUCT*)lParam;
//		DWORD keyCode = pKeyboardHookStruct->vkCode;
//		DWORD scanCode = pKeyboardHookStruct->scanCode;
//
//		BYTE keyboardState[256];
//		GetKeyboardState(keyboardState);
//		WORD charCode;
//		if (ToAscii(keyCode, scanCode, keyboardState, &charCode, 0) == 1)
//		{
//			char character = static_cast<char>(charCode);
//			if (character == 't')
//			{
//				printf("stop\n");
//				m_bStop = true;
//			}
//			else if (character == 'y'/* && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)*/)
//			{
//				printf("start\n");
//				m_bStop = false;
//			}
//		}
//	}
//
//	return CallNextHookEx(NULL, nCode, wParam, lParam);
//}

int cmatch::matchShape(const std::vector<cv::Point>& sets)
{
	if (sets.empty())
		return -1;
	std::vector<double>v;
	v.reserve(m_iconPointsSet.size());

	for (int i = 0; i < m_iconPointsSet.size(); ++i)
		v.emplace_back(mysc->computeDistance(m_iconPointsSet[i], sets));//cv::matchShapes(m_iconPointsSet[i], sets, cv::ShapeMatchModes::CONTOURS_MATCH_I1, 0));
	
	auto iter = std::min_element(v.cbegin(), v.cend());

	if (iter == v.cend())
		return -1;

	return (int)(iter - v.cbegin());
}

int cmatch::matchShape(const cv::Mat& src, const cv::Rect &rect)
{
	return matchShape(getForegroundPoints(src, rect));
}

bool cmatch::isMatched(const cv::Mat& src, const cv::Rect &rect)
{
	return matchShape(src, rect) == 0;
}

void cmatch::readCfg()
{
	std::ifstream f("configure.cfg");
	std::string line;
	while (std::getline(f, line)) 
	{
		line.erase(std::remove(line.begin(), line.end(), ' '), line.end());
		std::replace(line.begin(), line.end(), '=', ' ');
		std::stringstream ss(line);
		std::string key, value;
		ss >> key >> value;
		if (key == "threshold")
		{
			m_waterFlowerThreshold = std::stoi(value);
			std::cout << "m_waterFlowerThreshold=" << m_waterFlowerThreshold << std::endl;
		}
		else if (key == "points")
		{
			m_waterHis = std::stoi(value);
			std::cout << "m_waterHis=" << m_waterHis << std::endl;
		}
	}

}

bool cmatch::judgeOnFish(cv::Mat& mat)
{
	cv::Mat gray,bin,edge;
	cv::cvtColor(mat, gray, cv::COLOR_BGR2GRAY);
	//cv::threshold(gray, bin, m_waterFlowerThreshold, 255, cv::THRESH_BINARY);
	cv::Canny(gray, edge, 170, 180);
	if (edge.empty()) 
	{
		fishing();
		return true;
	}

	//std::vector<std::vector<cv::Point>>psets;
	//cv::findContours(edge, psets, cv::RetrievalModes::RETR_LIST, cv::ContourApproximationModes::CHAIN_APPROX_NONE);
	//if (psets.empty()) 
	//{
	//	fishing();
	//	return true;
	//}
	std::vector<cv::Vec4f>sets;
	cv::HoughLinesP(edge, sets, 1, CV_PI / 180, 20, 0);
	cv::Scalar color(0, 0, 255);

	std::vector<cv::Vec4f>ptSets;

	for (int i = 0; i < sets.size(); ++i)
	{
		//decltype(sets)::value_type::value_type r = sets[i][0];
		//decltype(sets)::value_type::value_type theta = sets[i][1];
		//float cv = cosf(theta);
		//float sv = sinf(theta);
		//decltype(r) x = r * cv;
		//decltype(r) y = r * sv;
		//cv::Point2f C(x,y);
		//cvRound()
		//cv::Point2f P1(x + 1000*(-sv), y + 1000 * cv);
		//cv::Point2f P2(x - 1000 * -sv, y - 1000 * cv);
		cv::Point2f p1(sets[i][0], sets[i][1]);
		cv::Point2f p2(sets[i][2], sets[i][3]);
		float k = calcK(p1, p2);
		float tenDegrees = std::tanf(3*CV_PI / 36);
		if (k <= tenDegrees && k >= -tenDegrees)
		{
			ptSets.emplace_back(std::move(sets[i]));
			cv::line(mat,p1, p2, cv::Scalar(0,255,0));
		}
	}
	if (ptSets.empty())
	{
		//cv::imshow("edge", edge);
		//cv::imshow("gray", gray);
		//for (;;)
		//	cv::waitKey();
		fishing();
		return true;
		
	}
	//m_ptSets.emplace_back(std::move(ptSets));
	//beginingFishing();
	if (isOnFishing(std::move(ptSets))) 
	{
		fishing();
		return true;
	}

	//static int index = 0;
	//++index;
	//cv::imwrite(std::to_string(index) + "_line.jpg", m1at);

	return false;
}

bool cmatch::judgeOnFish()
{
	int ch = m_audio.GetChannels();
	int sampleRate = m_audio.GetSamplerate();
	int bytes = m_audio.GetBitsPerSample();
	assert(bytes == 16);

	//FILE*f = fopen((getCurrentTime() + ".pcm").c_str(), "wb+");




	while (!judgeFishingTimeOut())
	{
		int samples = m_audio.GetSamples();
		if (samples > 0)
		{
			uint8_t* pp = new uint8_t[bytes / 8 * ch * samples];
			samples = m_audio.Read(pp, samples);
			//fwrite(pp, 1, bytes / 8 * ch * samples, f);
			
			int s = 0;
			if (samples > sampleRate)
				s = samples - sampleRate;
			std::vector<int>sets;
			sets.reserve(sampleRate);
			bool b = false;
			for (; s < samples; ++s)
			{
				//pp[s * ch * 2];
				int avg = 0;
				for (int c = 0; c < ch; ++c)
				{
					avg += std::abs(*(int16_t*)&pp[s * ch * 2 + c * 2]);
				}
				sets.emplace_back(avg / ch);
				/*if (avg > 0)
				{
					b = true;
					break;
				}*/
			}
			delete[] pp;

			int nums = std::count_if(sets.cbegin(), sets.cend(), [](const int& c) {return std::abs(c) > 10; });
			std::cout << nums << " ";
			if (nums > samples /2)
			{
				b = true;
			}
			if (b)
			{
				fishing();
				break;
			}
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}
	//fclose(f);
	return false;
}

void cmatch::fishing()
{
	if (m_yumaoRect != cv::Rect())
	{
		setCursorPosition(m_yumaoRect.x + m_yumaoRect.width / 2, m_yumaoRect.y + m_yumaoRect.height / 2);
		RightButtonClick(m_yumaoRect.x + m_yumaoRect.width / 2, m_yumaoRect.y + m_yumaoRect.height / 2);
		Sleep(1000);
	}
	beginingFishing();
}

bool cmatch::isOnFishing(std::vector<cv::Vec4f>&& sets)
{
	if (m_ptSets.empty())
	{

		m_ptSets.emplace_back(std::move(sets));
		return false;
	}

	//std::vector<float>maxY;
	auto fun = [this](const decltype(m_ptSets)::value_type& v) ->bool
	{
		//int maxy = 0;
		//for (auto& ele : v) 
		//{
			/*if (ele[1] > maxy)
				maxy = ele[1];
			if (ele[3] > maxy)
				maxy = ele[3];*/
			/*maxy += ele[1] + ele[3];
		}
		return (float)maxy/(v.size()*2);*/
		/*if (time(NULL) - m_time_begin > 23) 
		{
			int yy = 1 + 1;
		}*/

		auto& first = m_ptSets[0];
		bool bfind = false;
		for (auto& ele : v) 
		{
			ele[1]; ele[3];
			for (auto& fele : first) 
			{
				if (std::abs(ele[1] - fele[1]) < 4
					|| std::abs(ele[1] - fele[3]) < 4
					|| std::abs(ele[3] - fele[1]) < 4
					|| std::abs(ele[3] - fele[3]) < 4
					)
				{
					bfind = true;
					break;
				}
			}
			if (bfind)
				break;
		}
		return bfind;
	};
	//float averageV = fun(sets);
	/*for (const auto& ele : m_ptMaxYSets) 
	{
		fun()
	}*/
	//m_ptMaxYSets.emplace_back(fun(sets));



	bool b = fun(sets);
	/*if (m_ptSets.empty()) 
	{
		for (auto& ele : sets) 
		{
			std::cout << ele[0] << ele[1]<<"      "<< ele[2] << ele[3] << std::endl;
		}
		std::cout << "*********************" << std::endl;
	}*/
	m_ptSets.emplace_back(std::move(sets));
	return !b;
}



//bool cmatch::m_bStop = false;