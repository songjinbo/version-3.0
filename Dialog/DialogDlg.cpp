
// DialogDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "afxmt.h"
#include <stdlib.h>
#include <queue>
#include <fstream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "Dialog.h"
#include "afxdialogex.h"
#include "DialogDlg.h"
#include "GetImageThread.h"
#include "GetVoxelThread.h"
#include "..\\SkinSharp\\SkinH.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CDialogDlg 对话框



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
}

BEGIN_MESSAGE_MAP(CDialogDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_START, &CDialogDlg::OnBnClickedStart)
	ON_MESSAGE(WM_DISPLAY_IMAGE, DisplayImage)
	ON_MESSAGE(WM_UPDATE_STATUS, UpdateStatus)
	ON_BN_CLICKED(IDC_STOP, &CDialogDlg::OnBnClickedStop)
END_MESSAGE_MAP()


// CDialogDlg 消息处理程序
BOOL CDialogDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标
	// TODO:  在此添加额外的初始化代码
	SkinH_AttachEx(L"../SkinSharp/Skins/TVB.she", NULL);//添加皮肤
	InitWindow(&m_DisplayLeft, &m_DisplayDepth);
	InitThread();
	system("md data"); //创建一个文件夹，存放产生的数据

	return false;  // 除非将焦点设置到控件，否则返回 TRUE
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CDialogDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CDialogDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



/**********************************************************/

using namespace cv;
using namespace std;

#define MAXA 6400
#define PI 3.1415926

//主线程与三个子线程的接口
volatile ProgressStatus progress_status = is_stopped; //有冲突隐患

//getvoxel线程与主线程的接口变量
CCriticalSection critical_single_rawdata;//控制depth_iamge、left_image和position的访问
Mat depth_image;
Mat left_image;
Position position;
int count_voxel_file = 1;//用于对体素化的数据进行计数

//getimage线程与主线程的接口变量
SOCKET sockConn,sockRrv;

//pathplan线程与主线程的接口
double start_and_end[6]; //传给路径规划模块,有冲突隐患
bool is_first_frame=1;//是否是第一帧
double subEndx, subEndy, subEndz; //用来做标注的数据

//getimage线程与getvoxel线程的接口变量
CCriticalSection critical_rawdata;//控制vec_depth、vec_left和vec_position的访问
vector<Mat> vec_depth;
vector<Mat> vec_left;
vector<Position> vec_position;
int rece_count = 0;

//getvoxel线程与pathplan线程的接口变量
vector<double> voxel_x; //GetVoxelThread的输出,PathPlanThread的输入
vector<double> voxel_y;
vector<double> voxel_z;

clock_t start_time, finish_time; //这两个变量分别存储运行开始时间和结束时间

void CDialogDlg::OnBnClickedStart()
{
	GetDlgItem(IDC_START)->EnableWindow(FALSE);
	//初始化过程，可以多次点击展示
	InitVariable();
	bool succe = BuildConnection(sockRrv, sockConn);
	if (!succe)
	{
		GetDlgItem(IDC_START)->EnableWindow(TRUE);
		return;
	}

	progress_status = is_ruuning;
	GetDlgItem(IDC_STOP)->EnableWindow(TRUE);

	m_pget_image_thread->PostThreadMessage(WM_GETIMAGE_BEGIN, NULL, NULL);
	GetDlgItem(IDC_STATUS_GETIMAGE)->SetWindowTextW(_T("GetImage函数正在运行"));
	m_pget_voxel_thread->PostThreadMessage(WM_GETVOXEL_BEGIN, NULL, NULL);
	GetDlgItem(IDC_STATUS_GETVOXEL)->SetWindowTextW(_T("GetVoxel函数正在运行"));
	GetDlgItem(IDC_STATUS_PATHPLAN)->SetWindowTextW(_T("PathPlan函数正在运行"));
	GetDlgItem(IDC_STATUS)->SetWindowTextW(_T("正在运行"));//清空状态栏
	
	start_time = clock();
}
void CDialogDlg::OnBnClickedStop()
{
	// TODO:  在此添加控件通知处理程序代码
	progress_status = is_stopped;
	GetDlgItem(IDC_STOP)->EnableWindow(FALSE);
	GetDlgItem(IDC_START)->EnableWindow(TRUE);

	GetDlgItem(IDC_STATUS)->SetWindowTextW(_T("终止进程"));//清空状态栏
}
void CDialogDlg::InitVariable()
{
	//主线程与三个子线程的接口
	//与getvoxel线程的接口
	depth_image = Mat(Scalar(0));
	left_image = Mat(Scalar(0));
	position.x = position.y = position.z = position.pitch = position.yaw = position.roll = 0;
	//与getvoxel线程的接口
	count_voxel_file = 1;
	//与pathplan的接口
	UpdateData(TRUE);
	start_and_end[0] = m_dstartx;
	start_and_end[1] = m_dstarty;
	start_and_end[2] = m_dstartz;
	start_and_end[3] = m_dendx;
	start_and_end[4] = m_dendy;
	start_and_end[5] = m_dendz;
	is_first_frame = 1; //置标志位
	//getimage与getvoxel的接口
	vec_depth.clear();
	vec_left.clear();
	vec_position.clear();
	rece_count = 0;
	//getvoxel与pathplan的接口
	voxel_x.clear();
	voxel_y.clear();
	voxel_z.clear();

	progress_status == is_stopped;
}
bool CDialogDlg::BuildConnection(SOCKET &sockRrv, SOCKET &sockConn)
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	SOCKADDR_IN addrSrv;
	SOCKADDR_IN addrClient;
	int len;
	wVersionRequested = MAKEWORD(1, 1);
	err = WSAStartup(wVersionRequested, &wsaData);
	sockRrv = socket(AF_INET, SOCK_STREAM, 0);
	addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//  
	addrSrv.sin_port = htons(6001); //端口号 
	addrSrv.sin_family = AF_INET;  //固定参数
	bind(sockRrv, (SOCKADDR *)&addrSrv, sizeof(SOCKADDR));  //固定流程，绑定端口号
	listen(sockRrv, 5);  //第二个参数应该是客户端连接的最大数
	len = sizeof(SOCKADDR);

	int rcv_size = 310 * 1024; //310K的缓冲区
	err = setsockopt(sockRrv, SOL_SOCKET, SO_RCVBUF, (char *)&rcv_size, sizeof(rcv_size));
	if (err<0){
		closesocket(sockRrv);
		GetDlgItem(IDC_STATUS)->SetWindowTextW(_T("建立缓冲区错误"));
		return 0;//代表返回正常值
	}
	GetDlgItem(IDC_STATUS)->SetWindowTextW(_T("等待连接"));
	sockConn = accept(sockRrv, (SOCKADDR *)&addrClient, &len); //固定流程 
	GetDlgItem(IDC_STATUS)->SetWindowTextW(_T("连接成功"));
	return 1;
}

char*display_window_name[2] = { "view_left", "view_depth" }; //这个变量不需要更改
void CDialogDlg::InitWindow(CStatic *m_DisplayLeft, CStatic *m_DisplayDepth)
{
	this->SetWindowText(L"无人机智能飞行演示系统");
	//创建窗口用来显示左相机图片
	namedWindow(display_window_name[0], WINDOW_AUTOSIZE);
	HWND hWnd_left = (HWND)cvGetWindowHandle(display_window_name[0]);
	HWND hParent_left = ::GetParent(hWnd_left);
	::ShowWindow(hParent_left, SW_HIDE); //原先用来显示的窗口消隐
	::SetParent(hWnd_left, m_DisplayLeft->m_hWnd);//将显示画面附着在IDC_DISPLAYLEFT上
	//创建窗口用来显示深度图
	namedWindow(display_window_name[1], WINDOW_AUTOSIZE);
	HWND hWnd_depth = (HWND)cvGetWindowHandle(display_window_name[1]);
	HWND hParent_depth = ::GetParent(hWnd_depth);
	::ShowWindow(hParent_depth, SW_HIDE);
	::SetParent(hWnd_depth, m_DisplayDepth->m_hWnd);

	//设置标题字体格式
	titleFont.CreatePointFont(300, L"楷体");
	GetDlgItem(IDC_TITLE)->SetFont(&titleFont, true);

	//设置group box控件的字体格式
	groupFont.CreateFont(20, 0, 0, 0, FW_BLACK, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));
	GetDlgItem(IDC_START_POSI)->SetWindowText(L"起点坐标");
	GetDlgItem(IDC_START_POSI)->SetFont(&groupFont);
	GetDlgItem(IDC_END_POSI)->SetFont(&groupFont);
	GetDlgItem(IDC_POSE)->SetFont(&groupFont);
	GetDlgItem(IDC_SENSOR_DATA)->SetFont(&groupFont);

	//设置位姿数据的字体格式
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

	//设置静态文本框的文字格式
	staticFont.CreateFont(18, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));
	GetDlgItem(IDC_STATIC_X)->SetFont(&staticFont);
	GetDlgItem(IDC_STATIC_Y)->SetFont(&staticFont);
	GetDlgItem(IDC_STATIC_Z)->SetFont(&staticFont);
	GetDlgItem(IDC_STATIC_ROLL)->SetFont(&staticFont);
	GetDlgItem(IDC_STATIC_YAW)->SetFont(&staticFont);
	GetDlgItem(IDC_STATIC_PITCH)->SetFont(&staticFont);
	GetDlgItem(IDC_STATIC_RUNNING_TIME)->SetFont(&staticFont);
	GetDlgItem(IDC_STATIC_FINISH_FRAMES)->SetFont(&staticFont);

	GetDlgItem(IDC_STOP)->EnableWindow(FALSE);//开始stop按钮不可用
	GetDlgItem(IDC_START)->EnableWindow(TRUE);//开始stop按钮不可用
}
void CDialogDlg::InitThread()
{
	m_pget_image_thread = (GetImageThread*)AfxBeginThread(RUNTIME_CLASS(GetImageThread));
	m_pget_voxel_thread = (GetVoxelThread*)AfxBeginThread(RUNTIME_CLASS(GetVoxelThread));
	m_ppath_plan_thread = (PathPlanThread*)AfxBeginThread(RUNTIME_CLASS(PathPlanThread));
	if (m_pget_image_thread && m_pget_voxel_thread && m_ppath_plan_thread)
	{
		GetDlgItem(IDC_STATUS_GETIMAGE)->SetWindowTextW(_T("GetImage线程创建成功"));
		GetDlgItem(IDC_STATUS_GETVOXEL)->SetWindowTextW(_T("GetVoxel线程创建成功"));
		GetDlgItem(IDC_STATUS_PATHPLAN)->SetWindowTextW(_T("PathPlan线程创建成功"));
	}
	else
	{
		GetDlgItem(IDC_STATUS)->SetWindowTextW(_T("线程创建失败"));
		GetDlgItem(IDC_START)->EnableWindow(FALSE);
	}
}

//-------------------------------//
//此函数用来给左相机图像做标注
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
	if (wParam == subpath_accessible) //显示图片的消息
	{
		critical_single_rawdata.Lock();
		Mat depth_32S;//CV_16S显示成图片会有问题，需转化为8U
		Mat depth_image_cv8u;
		depth_image.convertTo(depth_32S, CV_32SC1);
		depth_32S = depth_32S * 255 / MAXA;
		depth_32S.convertTo(depth_image_cv8u, CV_8UC1);

		double x = double(int(position.x * 1000)) / 1000; double y = double(int(position.y * 1000)) / 1000; double z = double(int(position.z * 1000)) / 1000;//保留小数点后三位
		double yaw_angle = double(int(position.yaw * 180 / PI * 1000)) / 1000; //变成角度，保留小数点后三位
		double roll_angle = double(int(position.roll * 180 / PI * 1000)) / 1000;
		double pitch_angle = double(int(position.pitch * 180 / PI * 1000)) / 1000;
		
		m_dx = x;
		m_dy = y;
		m_dz = z;
		m_droll = roll_angle;
		m_dpitch = pitch_angle;
		m_dyaw = roll_angle;
		UpdateData(false);         // 更新数据
		Label(left_image, subEndx, subEndy, subEndz);

		imshow(display_window_name[0], left_image);
		imshow(display_window_name[1], depth_image_cv8u);
		waitKey(1); //必须要有的，不能忘记
		critical_single_rawdata.Unlock();
		
		Sleep(50);

		m_pget_voxel_thread->PostThreadMessage(WM_GETVOXEL_BEGIN, NULL, NULL);	
		GetDlgItem(IDC_STATUS_PATHPLAN)->SetWindowTextW(_T("成功获得一条路径"));

		//显示运行的时间和总帧数
		finish_time = clock();
		double time = double(finish_time - start_time) / CLOCKS_PER_SEC;
		m_drunning_time = time;
		m_dfinish_frames = count_voxel_file-1;
		m_drece_frames = rece_count;
		UpdateData(FALSE);
	}
	return 1;
}

extern volatile get_image_ret_code get_image_status;
LRESULT CDialogDlg::UpdateStatus(WPARAM wParam, LPARAM lParam)
{
	GetDlgItem(IDC_START)->EnableWindow(FALSE);
	if (wParam == receive_error)
	{
		GetDlgItem(IDC_STATUS_GETIMAGE)->SetWindowTextW(_T("运行结束，接收错误"));
		GetDlgItem(IDC_STATUS)->SetWindowTextW(_T("运行结束，接收错误"));
		get_image_status = get_image_complete;
	}
	else if (wParam == send_error)
	{
		GetDlgItem(IDC_STATUS_GETIMAGE)->SetWindowTextW(_T("运行结束，发送错误"));
		GetDlgItem(IDC_STATUS)->SetWindowTextW(_T("运行结束，发送错误"));
		get_image_status = get_image_complete;
	}
	else if (wParam == data_error)
	{
		GetDlgItem(IDC_STATUS_GETIMAGE)->SetWindowTextW(_T("出现错误数据"));
	}
	else if (wParam == no_data_in_queue)
	{
		GetDlgItem(IDC_STATUS_GETVOXEL)->SetWindowTextW(_T("GetVoxel函数正在等待"));
		m_pget_voxel_thread->PostThreadMessage(WM_GETVOXEL_BEGIN, NULL, NULL);
	}
	else if (wParam == no_voxel_in_queue)
	{
		GetDlgItem(IDC_STATUS_PATHPLAN)->SetWindowTextW(_T("PathPlan函数正在等待"));
		m_pget_voxel_thread->PostThreadMessage(WM_GETVOXEL_BEGIN, NULL, NULL);//再次开始getvoxel线程
	}
	else if (wParam == get_image_is_stopped)
	{
		GetDlgItem(IDC_STATUS_GETIMAGE)->SetWindowTextW(_T("运行结束，GetImage函数被强制结束"));
	}
	else if (wParam == get_voxel_is_stopped)
	{
		GetDlgItem(IDC_STATUS_GETVOXEL)->SetWindowTextW(_T("运行结束，GetVoxel函数被强制结束"));
		GetDlgItem(IDC_STATUS_PATHPLAN)->SetWindowTextW(_T("运行结束，PathPlan函数被强制结束"));
	}
	else if (wParam == path_plan_is_stopped)
	{
		GetDlgItem(IDC_STATUS_PATHPLAN)->SetWindowTextW(_T("运行结束，PathPlan函数被强制结束"));
		GetDlgItem(IDC_STATUS_GETVOXEL)->SetWindowTextW(_T("运行结束，GetVoxel函数被强制结束"));
	}
	else if (wParam == get_one_voxel)
	{
		GetDlgItem(IDC_STATUS_GETVOXEL)->SetWindowTextW(_T("成功获取一帧体素"));
		m_ppath_plan_thread->PostThreadMessage(WM_PATHPLAN_BEGIN, NULL, NULL);
	}
	else if (wParam == get_image_complete)
	{
		GetDlgItem(IDC_STATUS_GETIMAGE)->SetWindowTextW(_T("运行结束，数据读取完毕"));
	}
	else if (wParam == get_all_voxel_complete)
	{
		GetDlgItem(IDC_STATUS_GETVOXEL)->SetWindowTextW(_T("运行结束，体素化完毕"));
		m_ppath_plan_thread->PostThreadMessage(WM_PATHPLAN_BEGIN, NULL, NULL);
	}
	else if (wParam == path_accessible)
	{
		progress_status = complete;
		GetDlgItem(IDC_STOP)->EnableWindow(FALSE);
		GetDlgItem(IDC_START)->EnableWindow(TRUE);

		GetDlgItem(IDC_STATUS_PATHPLAN)->SetWindowTextW(_T("运行结束，找到路径"));
		GetDlgItem(IDC_STATUS)->SetWindowTextW(_T("运行结束，找到路径"));
		GetDlgItem(IDC_START)->SetWindowTextW(_T("开始"));
	
		//显示运行的时间和总帧数
		finish_time = clock();
		double time = double(finish_time - start_time) / CLOCKS_PER_SEC;
		m_drunning_time = time;
		m_dfinish_frames = count_voxel_file-1;
		m_drece_frames = rece_count;
		UpdateData(FALSE);
	}
	else if (wParam == no_path_accessible)
	{
		progress_status = complete;
		GetDlgItem(IDC_STOP)->EnableWindow(FALSE);
		GetDlgItem(IDC_START)->EnableWindow(TRUE);

		GetDlgItem(IDC_STATUS_PATHPLAN)->SetWindowTextW(_T("运行结束，未找到路径"));
		GetDlgItem(IDC_STATUS)->SetWindowTextW(_T("运行结束，未找到路径"));
		GetDlgItem(IDC_START)->SetWindowTextW(_T("开始"));
		
		//显示运行的时间和总帧数
		finish_time = clock();
		double time = double(finish_time - start_time) / CLOCKS_PER_SEC;
		m_drunning_time = time;
		m_dfinish_frames = count_voxel_file-1;
		m_drece_frames = rece_count;
		UpdateData(FALSE);
	}

	GetDlgItem(IDC_START)->EnableWindow(TRUE);

	return 1;
}