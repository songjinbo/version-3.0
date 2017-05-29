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
#include <cerrno>

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

extern SOCKET sockConn;

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

//udp接收数据函数的返回值
enum ERRCODE
{
	RCVERROR_IP = 1,
	RCVERROR_LEN,
	RCVERROR_BLOCK,
	SNDERROR,
	SUCCESS,
};

ERRCODE RecvData(SOCKET sock, char *buf, int size);

void GetImageThread::GetImage(UINT wParam, LONG lParam)
{
	get_image_status = get_image_is_running;//每次调用此函数先得置标志位

	const int length = sizeof(MulDataStream);
	MulDataStream data; //接收socket数据
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
			get_image_status = get_image_is_stopped;
			::PostMessage((HWND)(GetMainWnd()->GetSafeHwnd()), WM_UPDATE_STATUS, get_image_status, NULL);
			return;
		}
		ERRCODE ret = RecvData(sockConn, (char *)&data, length);
		if (ret == RCVERROR_IP)
		{
			closesocket(sockConn);
			get_image_status = wrong_IP;
			::PostMessage((HWND)(GetMainWnd()->GetSafeHwnd()), WM_UPDATE_STATUS, get_image_status, NULL);
			return;
		}
		else if (ret == RCVERROR_LEN)
		{
			closesocket(sockConn);
			get_image_status = wrong_length;
			::PostMessage((HWND)(GetMainWnd()->GetSafeHwnd()), WM_UPDATE_STATUS, get_image_status, NULL);
			return;
		}
		else if (ret == RCVERROR_BLOCK)
		{
			closesocket(sockConn);
			get_image_status = wrong_frame;
			::PostMessage((HWND)(GetMainWnd()->GetSafeHwnd()), WM_UPDATE_STATUS, get_image_status, NULL);
			return;
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


#define MAXLEN 1200
struct RCVUNIT
{
	int count;
	int index;
	char data[MAXLEN];
};
struct SNDUNIT
{
	int count;
	char data[6];
};
char client_IP[] = { "192.168.3.33" }; //客户端IP地址

extern int abandon_count; //计数漏掉的数据报数目
//读取size个Byte的数据
ERRCODE RecvData(SOCKET sock, char *buf, int size)
{
	int err;
	RCVUNIT data;

	struct sockaddr_in client_addr; //接收客户端地址
	int sin_size = sizeof(struct sockaddr);
	
	int BlockNum = (size % MAXLEN)? size/MAXLEN+1:size/MAXLEN;//分片的数目
	
	bool *isRcv = new bool[BlockNum];  //每一片是否到来的标志位，防止丢帧；
	for (int j = 0; j < BlockNum; j++)
		isRcv[j] = 0;

	int count = -1;//第count帧;
	for (int i = 0; i < BlockNum; i++)
	{
		//接收一片
		err = recvfrom(sock, (char *)&data, sizeof(RCVUNIT), 0, (struct sockaddr *)&client_addr, &sin_size);
		if (err != sizeof(RCVUNIT))
			return RCVERROR_LEN;
		if (strncmp(inet_ntoa(client_addr.sin_addr), client_IP, sizeof(client_IP))) //验证源IP地址是否正确
			return RCVERROR_IP;
		if (i == 0)
			count = data.count;
		else if (data.count > count) //后帧传送过来，直接丢弃本帧
		{
			abandon_count++;
			i = 0;
			count = data.count;
			for (int j = 0; j < BlockNum; j++)
				isRcv[j] = 0;
		}
		else if (data.count < count) //前帧的数据报延时传送至目的地，直接忽略；
		{
			i--;
			continue;
		}
		//对接收片登记
		isRcv[data.index] = 1;
		//将接收数据按照片的实际顺序拷贝到缓冲区中
		if (data.index < size / MAXLEN)
			memcpy(buf + data.index*MAXLEN, data.data, MAXLEN);
		else
			memcpy(buf + data.index*MAXLEN, data.data, size - data.index*MAXLEN);
	}
	return SUCCESS;
}