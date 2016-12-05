// GetImageThread.cpp : 实现文件
//
#include "stdafx.h"
#include <string>
#include <iostream>
#include <fstream>
#include <queue>
#include <tchar.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include "Dialog.h"
#include "DialogDlg.h"
#include "GetImageThread.h"
#include "GetVoxelThread.h"


IMPLEMENT_DYNCREATE(GetImageThread, CWinThread)

GetImageThread::GetImageThread()
{
}

GetImageThread::~GetImageThread()
{
}

BOOL GetImageThread::InitInstance()
{
	// TODO:    在此执行任意逐线程初始化
	return TRUE;
}

int GetImageThread::ExitInstance()
{
	// TODO:    在此执行任意逐线程清理
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(GetImageThread, CWinThread)
	ON_THREAD_MESSAGE(WM_GETIMAGE_BEGIN, GetImage)
END_MESSAGE_MAP()


// GetImageThread 消息处理程序

using namespace cv;
using namespace std;

extern SOCKET sockConn, sockRrv;

extern CCriticalSection critical_rawdata;
extern vector<Mat> vec_depth;
extern vector<Mat> vec_left;
extern vector<Position> vec_position;
extern int rece_count;

extern volatile ProgressStatus progress_status;//标志进程的运行状态，0是暂停，1是进行
volatile get_image_ret_code get_image_status = get_image_is_running; //标志这一GetImage函数是否已经结束

string itos(double i)
{
	stringstream ss;

	ss << i;

	return ss.str();
}
//string转化成LPCWSTR类型
LPCWSTR stringToLPCWSTR(std::string orig)
{
	size_t origsize = orig.length() + 1;
	const size_t newsize = 100;
	size_t convertedChars = 0;
	wchar_t *wcstring = (wchar_t *)malloc(sizeof(wchar_t) *(orig.length() - 1));
	mbstowcs_s(&convertedChars, wcstring, origsize, orig.c_str(), _TRUNCATE);
	return wcstring;
}
int RecvData(SOCKET sock, void *buf, int size);

void GetImageThread::GetImage(UINT wParam, LONG lParam)
{
	get_image_status = get_image_is_running;//每次调用此函数先得置标志位

	const int length = sizeof(MulDataStream);
	char data_stream[length]; //接收传输的char *
	memset(data_stream, '0', length);
	MulDataStream data; //将data_stream的数据排列为MulDataStream
	memset(&data, 0, length);

	//程序临时变量
	cv::Mat depth_image(HEIGHT, WIDTH, CV_16SC1);
	cv::Mat left_image(HEIGHT, WIDTH, CV_8U);
	Position position;

	while(1)
	{
		if ((progress_status == is_stopped) || (progress_status == complete))
		{
			closesocket(sockConn);
			closesocket(sockRrv);
			get_image_status = get_image_is_stopped;
			::PostMessage((HWND)(GetMainWnd()->GetSafeHwnd()), WM_UPDATE_STATUS, get_image_status, NULL);
			return;
		}

		memset(data_stream, 0, length);
		int ret = RecvData(sockConn, data_stream, length);
		if (ret < 0)
		{
			closesocket(sockConn);
			closesocket(sockRrv);
			get_image_status = receive_error;
			::PostMessage((HWND)(GetMainWnd()->GetSafeHwnd()), WM_UPDATE_STATUS, get_image_status, NULL);
			return;
		}
		if (!strncmp(data_stream, "send error!", 11))
		{
			closesocket(sockConn);
			closesocket(sockRrv);
			get_image_status = send_error;
			::PostMessage((HWND)(GetMainWnd()->GetSafeHwnd()), WM_UPDATE_STATUS, get_image_status, NULL);
			return;
		}
		std::memcpy(&data, data_stream, length);
		if (strncmp(data.head, "head", 5) || strncmp(data.tail, "tail", 5)) //检查首尾的校验码是否正确
		{
			get_image_status = data_error;
			::PostMessage((HWND)(GetMainWnd()->GetSafeHwnd()), WM_UPDATE_STATUS, get_image_status, NULL);
			continue;
		}
		rece_count++;
		std::memcpy(depth_image.data, data.depth, WIDTH*HEIGHT * 2);
		std::memcpy(left_image.data, data.left, WIDTH*HEIGHT);
		position.pitch = data.attitude.pitch;
		position.roll = data.attitude.roll;
		position.yaw = data.attitude.yaw;
		position.x = data.posi.gps_x;
		position.y = data.posi.gps_y;
		position.z = data.posi.gps_z;

		//将读取的数据存到容器中
		critical_rawdata.Lock();
		vec_depth.push_back(depth_image.clone());
		vec_left.push_back(left_image.clone());
		vec_position.push_back(position);
		critical_rawdata.Unlock();
	}
}
//读取size个Byte的数据
int RecvData(SOCKET sock, void *buf, int size)
{
	int sum = size;
	int err;
	int index = 0;
	while (size != 0)
	{
		err = recv(sock, (char*)buf + index, size, 0);
		if (err == SOCKET_ERROR) return -1;
		else if (err == 0) return -1;
		size -= err;
		index += err;
	}
	return sum - size;
}