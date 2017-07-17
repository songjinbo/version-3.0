
// DialogDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "afxmt.h"
#include <stdlib.h>
#include <queue>
#include <fstream>
#include <fcntl.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "Dialog.h"
#include "afxdialogex.h"
#include "DialogDlg.h"
#include "GetImageThread.h"
#include "GetVoxelThread.h"
#include "..\\SkinSharp\\SkinH.h"

#include "base.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// CDialogDlg �Ի���

CDialogDlg::CDialogDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDialogDlg::IDD, pParent)
	, m_dx(0)
	, m_dy(0)
	, m_dz(0)
	, m_dyaw(0)
	, m_dpitch(0)
	, m_droll(0)
	, m_dstartx(0)
	, m_dendx(0)
	, m_dstarty(0)
	, m_dendy(0)
	, m_dendz(0)
	, m_dstartz(0)
	, m_drunning_time(0)
	, m_dfinish_frames(0)
	, m_drece_frames(0)
	, m_dabandon_frames(0)
	, m_dtransfer_speed(0)
	, m_dresultx(0)
	, m_dresulty(0)
	, m_dresultz(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDialogDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_X, m_dx);
	DDX_Text(pDX, IDC_Y, m_dy);
	DDX_Text(pDX, IDC_Z, m_dz);
	DDX_Text(pDX, IDC_YAW, m_dyaw);
	DDX_Text(pDX, IDC_PITCH, m_dpitch);
	DDX_Text(pDX, IDC_ROLL, m_droll);
	DDX_Control(pDX, IDC_DISPLAYLEFT, m_DisplayLeft);
	DDX_Control(pDX, IDC_DISPLAYDEPTH, m_DisplayDepth);
	DDX_Text(pDX, IDC_ENDX, m_dendx);
	DDX_Text(pDX, IDC_ENDY, m_dendy);
	DDX_Text(pDX, IDC_ENDZ, m_dendz);
	DDX_Text(pDX, IDC_STARTX, m_dstartx);
	DDX_Text(pDX, IDC_STARTY, m_dstarty);
	DDX_Text(pDX, IDC_STARTZ, m_dstartz);
	//  DDX_Control(pDX, IDC_DISPLAYMAP, m_DisplayMap);
	//  DDX_Text(pDX, IDC_FINISH_FRAMES, m_drunning_time);
	DDX_Text(pDX, IDC_FINISH_FRAMES, m_dfinish_frames);
	DDX_Text(pDX, IDC_RUNNING_TIME, m_drunning_time);
	DDX_Text(pDX, IDC_RECE_FRAMES, m_drece_frames);
	DDX_Text(pDX, IDC_ABANDON_FRAMES, m_dabandon_frames);
	DDX_Text(pDX, IDC_TRANSFER_SPEED, m_dtransfer_speed);
	DDX_Text(pDX, IDC_RESULTX, m_dresultx);
	DDX_Text(pDX, IDC_RESULTY, m_dresulty);
	DDX_Text(pDX, IDC_RESULTZ, m_dresultz);
}

BEGIN_MESSAGE_MAP(CDialogDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_START, &CDialogDlg::OnBnClickedStart)
	ON_MESSAGE(WM_UPDATE_STATUS, UpdateStatus)
	ON_BN_CLICKED(IDC_STOP, &CDialogDlg::OnBnClickedStop)
END_MESSAGE_MAP()


// CDialogDlg ��Ϣ�������
BOOL CDialogDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��
	// TODO:  �ڴ���Ӷ���ĳ�ʼ������
	SkinH_AttachEx(L"../SkinSharp/Skins/TVB.she", NULL);//���Ƥ��
	InitWindow(&m_DisplayLeft, &m_DisplayDepth);
	InitThread();
	system("md data"); //����һ���ļ��У���Ų���������

	return false;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CDialogDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CDialogDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CDialogDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

/**********************************************************/

using namespace cv;
using namespace std;

#define MAXA 1600
#define PI 3.1415926

//���߳����������̵߳Ľӿ�
volatile ProgressStatus progress_status = is_stopped; //�г�ͻ����

//----------getimage�߳������̵߳Ľӿڱ���----------------//
SOCKET sockConn; //����ͼ���׽���
CCriticalSection critical_single_rawdata;
cv::Mat depth_image(HEIGHT, WIDTH, CV_16SC1);
cv::Mat left_image(HEIGHT, WIDTH, CV_8U);
Position position;

int rece_count = 0;
int abandon_count = 0;
static int receive_frames = 0; //���յ���֡��

//-----------getvoxel�߳������̵߳Ľӿڱ���---------------//
int count_voxel_file = 1;//���ڶ����ػ������ݽ��м���

//--------------pathplan�߳������̵߳Ľӿ�----------------//
double start_and_end[6]; //����·���滮ģ��,�г�ͻ����
bool is_first_frame=1;//�Ƿ��ǵ�һ֡
double subEndx, subEndy, subEndz; //��������ע������
SOCKET sockCommand;//����ָ���׽���
SOCKADDR_IN  addrServ;

//----------���̡߳�getimage�̡߳�getvoxel�̵߳Ľӿڱ���----------//
CCriticalSection critical_rawdata;//����vec_depth��vec_left��vec_position�ķ���
vector<Mat> vec_depth;
vector<Mat> vec_left;
vector<Position> vec_position;

//----------getvoxel�߳���pathplan�̵߳Ľӿڱ���----------//
vector<double> voxel_x; //GetVoxelThread�����,PathPlanThread������
vector<double> voxel_y;
vector<double> voxel_z;

//------------���̡߳�getvoxel��pathplan�Ľӿ�------------//
double currentX;
double currentY;
double currentZ;

clock_t start_time, finish_time,last_time; //�����������ֱ�洢���п�ʼʱ��ͽ���ʱ��
void CDialogDlg::OnBnClickedStart()
{
	GetDlgItem(IDC_STATUS_GETIMAGE)->SetWindowTextW(_T(""));
	GetDlgItem(IDC_STATUS_GETVOXEL)->SetWindowTextW(_T(""));
	GetDlgItem(IDC_STATUS_PATHPLAN)->SetWindowTextW(_T(""));
	GetDlgItem(IDC_STATUS)->SetWindowTextW(_T(""));//���״̬��
	GetDlgItem(IDC_START)->EnableWindow(FALSE);
	//��ʼ�����̣����Զ�ε��չʾ
	InitVariable();
	//UDP�׽��֣�����ͼ��
	bool succe = BuildConnection(sockConn);
	if (!succe)
	{
		GetDlgItem(IDC_START)->EnableWindow(TRUE);
		GetDlgItem(IDC_STATUS_GETIMAGE)->SetWindowTextW(_T("UDP���Ӳ��ɹ�"));
		GetDlgItem(IDC_STATUS_GETVOXEL)->SetWindowTextW(_T("UDP���Ӳ��ɹ�"));
		GetDlgItem(IDC_STATUS_PATHPLAN)->SetWindowTextW(_T("UDP���Ӳ��ɹ�"));
		GetDlgItem(IDC_STATUS)->SetWindowTextW(_T("UDP���Ӳ��ɹ�"));//���״̬��
		return;
	}
	//TCP�׽��֣�����ָ��
	BuildConnectionCommand(sockCommand);//����TCP��ֱ���ɹ�����Ϊֹ

	progress_status = is_ruuning;
	GetDlgItem(IDC_STOP)->EnableWindow(TRUE);

	m_pget_image_thread->PostThreadMessage(WM_GETIMAGE_BEGIN, NULL, NULL);
	GetDlgItem(IDC_STATUS_GETIMAGE)->SetWindowTextW(_T("GetImage������������"));
	m_pget_voxel_thread->PostThreadMessage(WM_GETVOXEL_BEGIN, NULL, NULL);
	GetDlgItem(IDC_STATUS_GETVOXEL)->SetWindowTextW(_T("GetVoxel������������"));
	GetDlgItem(IDC_STATUS_PATHPLAN)->SetWindowTextW(_T("PathPlan������������"));
	GetDlgItem(IDC_STATUS)->SetWindowTextW(_T("��������"));//���״̬��
	
	start_time = clock();
	last_time = start_time;
}
void CDialogDlg::OnBnClickedStop()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	progress_status = is_stopped;
	GetDlgItem(IDC_STOP)->EnableWindow(FALSE);
	GetDlgItem(IDC_START)->EnableWindow(TRUE);

	GetDlgItem(IDC_STATUS)->SetWindowTextW(_T("��ֹ����"));//���״̬��
}
void CDialogDlg::InitVariable()
{
	//���߳����������̵߳Ľӿ�
	//��getvoxel�̵߳Ľӿ�
	//depth_image = Mat(HEIGHT,WIDTH,Scalar(0));
	//left_image = Mat(Scalar(0));
	position.x = position.y = position.z = position.pitch = position.yaw = position.roll = 0;
	//��getvoxel�̵߳Ľӿ�
	count_voxel_file = 1;
	//��pathplan�Ľӿ�
	UpdateData(TRUE);
	start_and_end[0] = m_dstartx;
	start_and_end[1] = m_dstarty;
	start_and_end[2] = m_dstartz;
	start_and_end[3] = m_dendx;
	start_and_end[4] = m_dendy;
	start_and_end[5] = m_dendz;
	is_first_frame = 1; //�ñ�־λ
	//getimage��getvoxel�Ľӿ�
	vec_depth.clear();
	vec_left.clear();
	vec_position.clear();
	rece_count = 0;
	abandon_count = 0;
	receive_frames = 0;
	//getvoxel��pathplan�Ľӿ�
	voxel_x.clear();
	voxel_y.clear();
	voxel_z.clear();

	progress_status = is_stopped;
}

//UDPЭ��socket,��������
bool CDialogDlg::BuildConnection(SOCKET &sockRrv) //UDP���ӣ����ϵ��µĴ���
{
	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD(2, 2);
	WSAStartup(wVersionRequested, &wsaData);

	struct sockaddr_in my_addr;   //�����������ַ�ṹ��
	memset(&my_addr, 0, sizeof(my_addr)); //���ݳ�ʼ��--����
	my_addr.sin_family = AF_INET; //����ΪIPͨ��
	//my_addr.sin_addr.s_addr = INADDR_ANY;//������IP��ַ--�������ӵ����б��ص�ַ��
	my_addr.sin_addr.s_addr = INADDR_ANY;//������IP��ַ--�������ӵ����б��ص�ַ��
	my_addr.sin_port = htons(8000); //�������˿ں�
	/*�������������׽���--IPv4Э�飬����������ͨ�ţ�UDPЭ��*/
	if ((sockRrv = socket(AF_INET, SOCK_DGRAM, 0))<0)
	{
		perror("socket");
		return 1;
	}
	/*���׽��ְ󶨵��������������ַ��*/
	if (bind(sockRrv, (struct sockaddr *)&my_addr, sizeof(struct sockaddr))<0)
	{
		perror("bind");
		return 1;
	}
	//���ý��ջ�������С
	const int rcv_size = 310 * 1024*4;
	setsockopt(sockRrv, SOL_SOCKET, SO_RCVBUF, (char *)&rcv_size, sizeof(rcv_size));

	//int imode = 0; //1Ϊ������ģʽ��0Ϊ����ģʽ
	//int retVal = ioctlsocket(sockRrv, FIONBIO, (u_long *)&imode);
	//if (retVal == SOCKET_ERROR)
	//{
	//	printf("ioctlsocket failed!");

	//	closesocket(sockRrv);
	//	WSACleanup();
	//	return -1;
	//}
	////��������ʱ��
	//int nNetTimeout = 4000;//��λms
	//setsockopt(sockRrv, SOL_SOCKET, SO_RCVTIMEO, (const char*)&nNetTimeout, sizeof(int));

	GetDlgItem(IDC_STATUS_GETVOXEL)->SetWindowTextW(_T("�ȴ�UDP����"));
	GetDlgItem(IDC_STATUS_GETIMAGE)->SetWindowTextW(_T("�ȴ�UDP����"));
	GetDlgItem(IDC_STATUS_PATHPLAN)->SetWindowTextW(_T("�ȴ�UDP����"));
	GetDlgItem(IDC_STATUS)->SetWindowTextW(_T("�ȴ�UDP����"));
	
	return 1;
}

//TCPЭ��socket,�ͻ���
bool CDialogDlg::BuildConnectionCommand(SOCKET &socketClient)
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		printf("WSAStartup failed with error: %d\n", err);
		return 0;
	}
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
		printf("Could not find a usable version of Winsock.dll\n");
		WSACleanup();
		return 0;
	}

	socketClient = socket(AF_INET, SOCK_DGRAM, 0);

	addrServ.sin_addr.S_un.S_addr = inet_addr("192.168.3.33");
	addrServ.sin_family = AF_INET;
	addrServ.sin_port = htons(5000);

	return 1;
}

//bool CDialogDlg::BuildConnectionCommand(SOCKET &socketClient)
//{
//	WORD wVersionRequested;
//	WSADATA wsaData;
//	wVersionRequested = MAKEWORD(1, 1);
//	int err;
//	err = WSAStartup(wVersionRequested, &wsaData);
//	if (err != 0)
//	{
//		return 0;
//	}
//	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)
//	{
//		WSACleanup();
//		return 0;
//	}
//
//	//����ͨѶsocket  
//	socketClient = socket(AF_INET, SOCK_STREAM, 0);
//
//	SOCKADDR_IN addrSrv;
//	addrSrv.sin_addr.S_un.S_addr = inet_addr("192.168.3.33");//�趨��Ҫ���ӵķ�������ip��ַ  
//	addrSrv.sin_family = AF_INET;
//	addrSrv.sin_port = htons(5000);//�趨��Ҫ���ӵķ������Ķ˿ڵ�ַ  
//	//bool res = connect(socketClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));//���������������  
//	//return res==0 ? 1:0;
//
//	GetDlgItem(IDC_STATUS_GETVOXEL)->SetWindowTextW(_T("��������TCP"));
//	GetDlgItem(IDC_STATUS_GETIMAGE)->SetWindowTextW(_T("��������TCP"));
//	GetDlgItem(IDC_STATUS_PATHPLAN)->SetWindowTextW(_T("��������TCP"));
//	GetDlgItem(IDC_STATUS)->SetWindowTextW(_T("��������TCP"));
//	while (connect(socketClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR)) != 0)//���������������  
//	{
//		Sleep(2000); //��������������һ��
//	}
//	return 0;
//}

char*display_window_name[2] = { "view_left", "view_depth" }; //�����������Ҫ����
void CDialogDlg::InitWindow(CStatic *m_DisplayLeft, CStatic *m_DisplayDepth)
{
	this->SetWindowText(L"���˻����ܷ�����ʾϵͳ");
	//��������������ʾ�����ͼƬ
	//namedWindow(display_window_name[0], WINDOW_AUTOSIZE);

	namedWindow(display_window_name[0], WINDOW_NORMAL);
	CRect rect;
	CWnd *pWnd = GetDlgItem(IDC_DISPLAYLEFT);
	pWnd->GetClientRect(&rect);
	int height = rect.Height();
	int width = rect.Width();
	resizeWindow(display_window_name[0],width,height);
	HWND hWnd_left = (HWND)cvGetWindowHandle(display_window_name[0]);
	HWND hParent_left = ::GetParent(hWnd_left);
	::ShowWindow(hParent_left, SW_HIDE); //ԭ��������ʾ�Ĵ�������
	::SetParent(hWnd_left, m_DisplayLeft->m_hWnd);//����ʾ���渽����IDC_DISPLAYLEFT��
	//��������������ʾ���ͼ
	namedWindow(display_window_name[1], WINDOW_NORMAL);
	pWnd = GetDlgItem(IDC_DISPLAYDEPTH);
	pWnd->GetClientRect(&rect);
	height = rect.Height();
	width = rect.Width();
	resizeWindow(display_window_name[1],width,height);
	HWND hWnd_depth = (HWND)cvGetWindowHandle(display_window_name[1]);
	HWND hParent_depth = ::GetParent(hWnd_depth);
	::ShowWindow(hParent_depth, SW_HIDE);
	::SetParent(hWnd_depth, m_DisplayDepth->m_hWnd);

	//���ñ��������ʽ
	titleFont.CreatePointFont(300, L"����");
	GetDlgItem(IDC_TITLE)->SetFont(&titleFont, true);

	//����group box�ؼ��������ʽ
	groupFont.CreateFont(20, 0, 0, 0, FW_BLACK, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));
	GetDlgItem(IDC_START_POSI)->SetWindowText(L"�������");
	GetDlgItem(IDC_START_POSI)->SetFont(&groupFont);
	GetDlgItem(IDC_END_POSI)->SetFont(&groupFont);
	GetDlgItem(IDC_POSE)->SetFont(&groupFont);
	GetDlgItem(IDC_SENSOR_DATA)->SetFont(&groupFont);
	GetDlgItem(IDC_RESULT)->SetFont(&groupFont);

	//����λ�����ݵ������ʽ
	poseFont.CreateFont(20, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));
	GetDlgItem(IDC_X)->SetFont(&poseFont);
	GetDlgItem(IDC_Y)->SetFont(&poseFont);
	GetDlgItem(IDC_Z)->SetFont(&poseFont);
	GetDlgItem(IDC_ROLL)->SetFont(&poseFont);
	GetDlgItem(IDC_PITCH)->SetFont(&poseFont);
	GetDlgItem(IDC_YAW)->SetFont(&poseFont);
	GetDlgItem(IDC_RUNNING_TIME)->SetFont(&poseFont);
	GetDlgItem(IDC_FINISH_FRAMES)->SetFont(&poseFont);
	GetDlgItem(IDC_RECE_FRAMES)->SetFont(&poseFont);
	GetDlgItem(IDC_ABANDON_FRAMES)->SetFont(&poseFont);
	GetDlgItem(IDC_TRANSFER_SPEED)->SetFont(&poseFont);
	GetDlgItem(IDC_RESULTX)->SetFont(&poseFont);
	GetDlgItem(IDC_RESULTY)->SetFont(&poseFont);
	GetDlgItem(IDC_RESULTZ)->SetFont(&poseFont);


	//���þ�̬�ı�������ָ�ʽ
	staticFont.CreateFont(18, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));
	GetDlgItem(IDC_STATIC_X)->SetFont(&staticFont);
	GetDlgItem(IDC_STATIC_Y)->SetFont(&staticFont);
	GetDlgItem(IDC_STATIC_Z)->SetFont(&staticFont);
	GetDlgItem(IDC_STATIC_ROLL)->SetFont(&staticFont);
	GetDlgItem(IDC_STATIC_YAW)->SetFont(&staticFont);
	GetDlgItem(IDC_STATIC_PITCH)->SetFont(&staticFont);
	GetDlgItem(IDC_STATIC_RUNNING_TIME)->SetFont(&staticFont);
	GetDlgItem(IDC_STATIC_FINISH_FRAMES)->SetFont(&staticFont);
	GetDlgItem(IDC_STATIC_RECE_FRAMES)->SetFont(&staticFont);
	GetDlgItem(IDC_STATIC_ABANDON_FRAMES)->SetFont(&staticFont);
	GetDlgItem(IDC_STATIC_TRANSFER_SPEED)->SetFont(&staticFont);
	GetDlgItem(IDC_STATIC_RESULTX)->SetFont(&staticFont);
	GetDlgItem(IDC_STATIC_RESULTY)->SetFont(&staticFont);
	GetDlgItem(IDC_STATIC_RESULTZ)->SetFont(&staticFont);

	GetDlgItem(IDC_STOP)->EnableWindow(FALSE);//��ʼstop��ť������
	GetDlgItem(IDC_START)->EnableWindow(TRUE);//��ʼstop��ť������
}
void CDialogDlg::InitThread()
{
	m_pget_image_thread = (GetImageThread*)AfxBeginThread(RUNTIME_CLASS(GetImageThread));
	m_pget_voxel_thread = (GetVoxelThread*)AfxBeginThread(RUNTIME_CLASS(GetVoxelThread));
	m_ppath_plan_thread = (PathPlanThread*)AfxBeginThread(RUNTIME_CLASS(PathPlanThread));
	if (m_pget_image_thread && m_pget_voxel_thread && m_ppath_plan_thread)
	{
		GetDlgItem(IDC_STATUS_GETIMAGE)->SetWindowTextW(_T("GetImage�̴߳����ɹ�"));
		GetDlgItem(IDC_STATUS_GETVOXEL)->SetWindowTextW(_T("GetVoxel�̴߳����ɹ�"));
		GetDlgItem(IDC_STATUS_PATHPLAN)->SetWindowTextW(_T("PathPlan�̴߳����ɹ�"));
	}
	else
	{
		GetDlgItem(IDC_STATUS)->SetWindowTextW(_T("�̴߳���ʧ��"));
		GetDlgItem(IDC_START)->EnableWindow(FALSE);
	}
}

//-------------------------------//
//�˺��������������ͼ������ע
//-------------------------------//
extern const double u0 = 116.502;
extern const double v0 = 156.469;
extern const double fx = 239.439;
extern const double fy = 239.439;
void Label(Mat & left, double px, double py, double pz)
{
	double z = pz * 128.0;
	//double y = (py * fy) / z + v0;
	double y = v0/2-(py * fy) / z;

	double x = (px * fx) / z + u0;

	circle(left, cvPoint(x, y), 2, CV_RGB(255, 0, 0), 3, 8, 0);   //paint point
}

LRESULT CDialogDlg::DisplayImage(WPARAM wParam, LPARAM lParam)
{
	critical_single_rawdata.Lock();
	Mat depth_32S;//CV_16S��ʾ��ͼƬ�������⣬��ת��Ϊ8U
	Mat depth_image_cv8u;
	depth_image.convertTo(depth_32S, CV_32SC1);
	depth_32S = depth_32S * 255 / MAXA;
	depth_32S.convertTo(depth_image_cv8u, CV_8UC1);
	Mat depth_color(depth_image_cv8u.rows, depth_image_cv8u.cols, CV_8UC3);
	Pseudocolor(depth_image_cv8u, depth_color);

	double x = double(int(position.x * 1000)) / 1000; double y = double(int(position.y * 1000)) / 1000; double z = double(int(position.z * 1000)) / 1000;//����С�������λ
	double yaw_angle = double(int(position.yaw * 180 / PI * 1000)) / 1000; //��ɽǶȣ�����С�������λ
	double roll_angle = double(int(position.roll * 180 / PI * 1000)) / 1000;
	double pitch_angle = double(int(position.pitch * 180 / PI * 1000)) / 1000;

	m_dx = x;
	m_dy = y;
	m_dz = z;
	m_droll = roll_angle;
	m_dpitch = pitch_angle;
	m_dyaw = roll_angle;
	UpdateData(false);         // ��������
	//Label(left_image, subEndx, subEndy, subEndz);

	imshow(display_window_name[0], left_image);
	imshow(display_window_name[1], depth_color);
	waitKey(1); //����Ҫ�еģ���������
	critical_single_rawdata.Unlock();

	//��ʾ���е�ʱ�����֡��
	finish_time = clock();
	double time = double(finish_time - start_time) / CLOCKS_PER_SEC;
	m_drunning_time = time;
	m_dfinish_frames = count_voxel_file - 1;
	m_drece_frames = rece_count;
	m_dabandon_frames = abandon_count;
	if ((finish_time - last_time) / CLOCKS_PER_SEC < 0.2)
		UpdateData(FALSE);
	else
	{
		m_dtransfer_speed = (rece_count + abandon_count - receive_frames) / ((finish_time - last_time) / CLOCKS_PER_SEC);
		last_time = finish_time;//������ʼʱ��
		receive_frames = rece_count + abandon_count;//���½��յ���֡��
		UpdateData(FALSE);
	}

	return 1;
}

extern volatile get_image_ret_code get_image_status;
LRESULT CDialogDlg::UpdateStatus(WPARAM wParam, LPARAM lParam)
{
	if (wParam == receive_error)
	{
		GetDlgItem(IDC_STATUS_GETIMAGE)->SetWindowTextW(_T("���н��������մ���"));
		GetDlgItem(IDC_STATUS)->SetWindowTextW(_T("���н��������մ���"));
		get_image_status = get_image_complete;
	}
	else if (wParam == get_one_image)
	{
		DisplayImage(NULL, NULL);
		GetDlgItem(IDC_STATUS_GETIMAGE)->SetWindowTextW(_T("����һ֡���"));
	}
	else if (wParam == wrong_IP)
	{
		GetDlgItem(IDC_STATUS_GETIMAGE)->SetWindowTextW(_T("���н�����IP��ַ����"));
		GetDlgItem(IDC_STATUS)->SetWindowTextW(_T("���н�����IP��ַ����"));
		get_image_status = get_image_complete;
	}
	else if (wParam == wrong_length)
	{
		GetDlgItem(IDC_STATUS_GETIMAGE)->SetWindowTextW(_T("���н��������ճ��ȴ���"));
		GetDlgItem(IDC_STATUS)->SetWindowTextW(_T("���н��������ճ��ȴ���"));
		get_image_status = get_image_complete;
	}
	else if (wParam == wrong_frame)
	{
		GetDlgItem(IDC_STATUS_GETIMAGE)->SetWindowTextW(_T("���н�������Ƭ����"));
		GetDlgItem(IDC_STATUS)->SetWindowTextW(_T("���н�������Ƭ����"));
		get_image_status = get_image_complete;
	}
	else if (wParam == send_error)
	{
		GetDlgItem(IDC_STATUS_GETIMAGE)->SetWindowTextW(_T("���н��������ʹ���"));
		GetDlgItem(IDC_STATUS)->SetWindowTextW(_T("���н��������ʹ���"));
		get_image_status = get_image_complete;
	}
	else if (wParam == data_error)
	{
		GetDlgItem(IDC_STATUS_GETIMAGE)->SetWindowTextW(_T("���ִ�������"));
	}
	else if (wParam == no_data_in_queue)
	{
		CString txt;
		GetDlgItemText(IDC_STATUS_GETVOXEL, txt);
		if (txt != _T("GetVoxel�������ڵȴ�"))
		{
			GetDlgItem(IDC_STATUS_GETVOXEL)->SetWindowTextW(_T("GetVoxel�������ڵȴ�"));
		}
		m_pget_voxel_thread->PostThreadMessage(WM_GETVOXEL_BEGIN, NULL, NULL);
	}
	else if (wParam == no_voxel_in_queue)
	{
		GetDlgItem(IDC_STATUS_PATHPLAN)->SetWindowTextW(_T("PathPlan�������ڵȴ�"));
		m_pget_voxel_thread->PostThreadMessage(WM_GETVOXEL_BEGIN, NULL, NULL);//�ٴο�ʼgetvoxel�߳�
	}
	else if (wParam == get_image_is_stopped)
	{
		GetDlgItem(IDC_STATUS_GETIMAGE)->SetWindowTextW(_T("���н�����GetImage������ǿ�ƽ���"));
	}
	else if (wParam == get_voxel_is_stopped)
	{
		GetDlgItem(IDC_STATUS_GETVOXEL)->SetWindowTextW(_T("���н�����GetVoxel������ǿ�ƽ���"));
		GetDlgItem(IDC_STATUS_PATHPLAN)->SetWindowTextW(_T("���н�����PathPlan������ǿ�ƽ���"));
	}
	else if (wParam == path_plan_is_stopped)
	{
		GetDlgItem(IDC_STATUS_PATHPLAN)->SetWindowTextW(_T("���н�����PathPlan������ǿ�ƽ���"));
		GetDlgItem(IDC_STATUS_GETVOXEL)->SetWindowTextW(_T("���н�����GetVoxel������ǿ�ƽ���"));
	}
	else if (wParam == get_one_voxel)
	{
		GetDlgItem(IDC_STATUS_GETVOXEL)->SetWindowTextW(_T("�ɹ���ȡһ֡����"));
		m_ppath_plan_thread->PostThreadMessage(WM_PATHPLAN_BEGIN, NULL, NULL);
	}
	else if (wParam == get_image_complete)
	{
		GetDlgItem(IDC_STATUS_GETIMAGE)->SetWindowTextW(_T("���н��������ݶ�ȡ���"));
	}
	else if (wParam == get_all_voxel_complete)
	{
		GetDlgItem(IDC_STATUS_GETVOXEL)->SetWindowTextW(_T("���н��������ػ����"));
		m_ppath_plan_thread->PostThreadMessage(WM_PATHPLAN_BEGIN, NULL, NULL);
	}
	else if (wParam == TCP_break_off)
	{
		progress_status = complete;
		GetDlgItem(IDC_STOP)->EnableWindow(FALSE);
		GetDlgItem(IDC_START)->EnableWindow(TRUE);

		GetDlgItem(IDC_STATUS_GETVOXEL)->SetWindowTextW(_T("TCP���ӶϿ�"));
		GetDlgItem(IDC_STATUS_GETIMAGE)->SetWindowTextW(_T("TCP���ӶϿ�"));
		GetDlgItem(IDC_STATUS_PATHPLAN)->SetWindowTextW(_T("TCP���ӶϿ�"));
		GetDlgItem(IDC_STATUS)->SetWindowTextW(_T("TCP���ӶϿ�"));
		GetDlgItem(IDC_START)->SetWindowTextW(_T("��ʼ"));

		//��ʾ���е�ʱ�����֡��
		finish_time = clock();
		double time = double(finish_time - start_time) / CLOCKS_PER_SEC;
		m_drunning_time = time;
		m_dfinish_frames = count_voxel_file - 1;
		m_drece_frames = rece_count;
		m_dabandon_frames = abandon_count;
		if ((finish_time - last_time) / CLOCKS_PER_SEC < 1.0)
			UpdateData(FALSE);
		else
		{
			m_dtransfer_speed = (rece_count + abandon_count - receive_frames) / ((finish_time - last_time) / CLOCKS_PER_SEC);
			last_time = finish_time;//������ʼʱ��
			receive_frames = rece_count + abandon_count;//���½��յ���֡��
			UpdateData(FALSE);
		}
	}
	else if (wParam == subpath_accessible) //��ʾͼƬ����Ϣ
	{


		//Sleep(5); //������Ϊ�˷��������ٶ�

		m_dresultx = subEndx - currentX;
		m_dresulty = subEndy - currentY;
		m_dresultz = subEndz - currentZ;
		UpdateData(false);

		//��·���滮�����ע��ͼ����
		Label(left_image, subEndx, subEndy, subEndz);
		imshow(display_window_name[0], left_image);

		m_pget_voxel_thread->PostThreadMessage(WM_GETVOXEL_BEGIN, NULL, NULL);
		GetDlgItem(IDC_STATUS_PATHPLAN)->SetWindowTextW(_T("�ɹ����һ��·��"));
	}
	else if (wParam == path_accessible)
	{
		progress_status = complete;
		GetDlgItem(IDC_STOP)->EnableWindow(FALSE);
		GetDlgItem(IDC_START)->EnableWindow(TRUE);

		//GetDlgItem(IDC_STATUS_GETVOXEL)->SetWindowTextW(_T("�ɹ��ҵ�·��"));
		//GetDlgItem(IDC_STATUS_GETIMAGE)->SetWindowTextW(_T("�ɹ��ҵ�·��"));
		GetDlgItem(IDC_STATUS_PATHPLAN)->SetWindowTextW(_T("�ɹ��ҵ�·��"));
		GetDlgItem(IDC_STATUS)->SetWindowTextW(_T("�ɹ��ҵ�·��"));
		GetDlgItem(IDC_START)->SetWindowTextW(_T("��ʼ"));
	
		//��ʾ���е�ʱ�����֡��
		finish_time = clock();
		double time = double(finish_time - start_time) / CLOCKS_PER_SEC;
		m_drunning_time = time;
		m_dfinish_frames = count_voxel_file-1;
		m_drece_frames = rece_count;
		m_dabandon_frames = abandon_count;
		if ((finish_time - last_time) / CLOCKS_PER_SEC < 0.2)
			UpdateData(FALSE);
		else
		{
			m_dtransfer_speed = (rece_count + abandon_count - receive_frames) / ((finish_time - last_time) / CLOCKS_PER_SEC);
			last_time = finish_time;//������ʼʱ��
			receive_frames = rece_count + abandon_count;//���½��յ���֡��
			UpdateData(FALSE);
		}

	}
	else if (wParam == no_path_accessible)
	{
		progress_status = complete;
		GetDlgItem(IDC_STOP)->EnableWindow(FALSE);
		GetDlgItem(IDC_START)->EnableWindow(TRUE);

		GetDlgItem(IDC_STATUS_GETVOXEL)->SetWindowTextW(_T("���н�����δ�ҵ�·��"));
		GetDlgItem(IDC_STATUS_GETIMAGE)->SetWindowTextW(_T("���н�����δ�ҵ�·��"));
		GetDlgItem(IDC_STATUS_PATHPLAN)->SetWindowTextW(_T("���н�����δ�ҵ�·��"));
		GetDlgItem(IDC_STATUS)->SetWindowTextW(_T("���н�����δ�ҵ�·��"));
		GetDlgItem(IDC_START)->SetWindowTextW(_T("��ʼ"));
		
		//��ʾ���е�ʱ�����֡��
		finish_time = clock();
		double time = double(finish_time - start_time) / CLOCKS_PER_SEC;
		m_drunning_time = time;
		m_dfinish_frames = count_voxel_file-1;
		m_drece_frames = rece_count;
		m_dabandon_frames = abandon_count;
		if ((finish_time - last_time) / CLOCKS_PER_SEC < 1.0)
			UpdateData(FALSE);
		else
		{
			m_dtransfer_speed = (rece_count + abandon_count - receive_frames) / ((finish_time - last_time) / CLOCKS_PER_SEC);
			last_time = finish_time;//������ʼʱ��
			receive_frames = rece_count + abandon_count;//���½��յ���֡��
			UpdateData(FALSE);
		}
	}
	return 1;
}