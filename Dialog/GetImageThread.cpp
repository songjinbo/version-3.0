// GetImageThread.cpp : ʵ���ļ�
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
	// TODO:    �ڴ�ִ���������̳߳�ʼ��
	return TRUE;
}

int GetImageThread::ExitInstance()
{
	// TODO:    �ڴ�ִ���������߳�����
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(GetImageThread, CWinThread)
	ON_THREAD_MESSAGE(WM_GETIMAGE_BEGIN, GetImage)
END_MESSAGE_MAP()


// GetImageThread ��Ϣ�������

using namespace cv;
using namespace std;

extern SOCKET sockConn;

extern CCriticalSection critical_rawdata;
extern vector<Mat> vec_depth;
extern vector<Mat> vec_left;
extern vector<Position> vec_position;
extern int rece_count;

extern volatile ProgressStatus progress_status;//��־���̵�����״̬��0����ͣ��1�ǽ���
volatile get_image_ret_code get_image_status = get_image_is_running; //��־��һGetImage�����Ƿ��Ѿ�����



string itos(double i)
{
	stringstream ss;

	ss << i;

	return ss.str();
}
//stringת����LPCWSTR����
LPCWSTR stringToLPCWSTR(std::string orig)
{
	size_t origsize = orig.length() + 1;
	const size_t newsize = 100;
	size_t convertedChars = 0;
	wchar_t *wcstring = (wchar_t *)malloc(sizeof(wchar_t) *(orig.length() - 1));
	mbstowcs_s(&convertedChars, wcstring, origsize, orig.c_str(), _TRUNCATE);
	return wcstring;
}

//udp�������ݺ����ķ���ֵ
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
	get_image_status = get_image_is_running;//ÿ�ε��ô˺����ȵ��ñ�־λ

	const int length = sizeof(MulDataStream);
	MulDataStream data; //����socket����
	memset(&data, 0, length);

	//������ʱ����
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

		//����ȡ�����ݴ浽������
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
char client_IP[] = { "192.168.3.33" }; //�ͻ���IP��ַ

extern int abandon_count; //����©�������ݱ���Ŀ
//��ȡsize��Byte������
ERRCODE RecvData(SOCKET sock, char *buf, int size)
{
	int err;
	RCVUNIT data;

	struct sockaddr_in client_addr; //���տͻ��˵�ַ
	int sin_size = sizeof(struct sockaddr);
	
	int BlockNum = (size % MAXLEN)? size/MAXLEN+1:size/MAXLEN;//��Ƭ����Ŀ
	
	bool *isRcv = new bool[BlockNum];  //ÿһƬ�Ƿ����ı�־λ����ֹ��֡��
	for (int j = 0; j < BlockNum; j++)
		isRcv[j] = 0;

	int count = -1;//��count֡;
	for (int i = 0; i < BlockNum; i++)
	{
		//����һƬ
		err = recvfrom(sock, (char *)&data, sizeof(RCVUNIT), 0, (struct sockaddr *)&client_addr, &sin_size);
		if (err != sizeof(RCVUNIT))
			return RCVERROR_LEN;
		if (strncmp(inet_ntoa(client_addr.sin_addr), client_IP, sizeof(client_IP))) //��֤ԴIP��ַ�Ƿ���ȷ
			return RCVERROR_IP;
		if (i == 0)
			count = data.count;
		else if (data.count > count) //��֡���͹�����ֱ�Ӷ�����֡
		{
			abandon_count++;
			i = 0;
			count = data.count;
			for (int j = 0; j < BlockNum; j++)
				isRcv[j] = 0;
		}
		else if (data.count < count) //ǰ֡�����ݱ���ʱ������Ŀ�ĵأ�ֱ�Ӻ��ԣ�
		{
			i--;
			continue;
		}
		//�Խ���Ƭ�Ǽ�
		isRcv[data.index] = 1;
		//���������ݰ���Ƭ��ʵ��˳�򿽱�����������
		if (data.index < size / MAXLEN)
			memcpy(buf + data.index*MAXLEN, data.data, MAXLEN);
		else
			memcpy(buf + data.index*MAXLEN, data.data, size - data.index*MAXLEN);
	}
	return SUCCESS;
}