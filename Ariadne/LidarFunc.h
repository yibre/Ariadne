#pragma once

/// LastofLidar.h 용

//#define _WINSOCK2API_
//#define WIN32_LEAN_AND_MEAN
// #define WINSOCK_API_LINKAGE
//#define _WINSOCK_DEPRECATED

#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <queue>
#include <cmath>
#include <process.h>
#include <thread>
#include <WinSock2.h>   					    // <----- stdafx.h
#pragma comment(lib,"ws2_32.lib")				// <----- stdafx.h
#include <Windows.h>


#ifdef _DEBUG
#define LOG_OUT(fmt) printf( "[%s:%d] %s\n",__FUNCTION__,__LINE__,fmt);
#else
#define LOG_OUT
#endif

#define ACCESS_MASK ::ACCESS_MASK

using namespace std;
using namespace cv;


typedef enum {
	SICK_STATUS_UNDEFINED		= 0,
	SICK_STATUS_INITIALIZATION	= 1,
	SICK_STATUS_CONFIGURATION	= 2,
	SICK_STATUS_IDLE			= 3,
	SICK_STATUS_ROTATED			= 4,
	SICK_STATUS_PREPARATION		= 5,
	SICK_STATUS_READY			= 6,
	SICK_STATUS_MEASUREMENT		= 7,
	SICK_STATUS_SCANCONTINOUS	= 8
} status_t;

enum {
	//SICK_SCAN_NUM_MAX = 541,
	SICK_SCAN_FRAME_NUM = 1,			// average of 2 frame
	SICK_SCAN_ROI_X = 3000,				// 반폭: 3.0 m, 좌우 총 6.0m
	SICK_SCAN_ROI_Y = 6000,				// 6.0 m
	SICK_SCAN_DIST_MIN = 100,		    // 0.1 m
	SICK_SCAN_DIST_MAX = 50000,			// 50 m
	SICK_SCAN_DIST_AVERAGE = 100,		// 0.1 m
	SICK_SCAN_DIST_OBJECT = 50,         // 0.05 m
	SICK_SCAN_DIST_CIRCLE_MIN = 25,     // 0.05 m
	SICK_SCAN_DIST_CIRCLE_MAX = 500,    // 0.5 m
	SICK_SCAN_DEG_START = 0,			// 0 deg
	SICK_SCAN_DEG_RESOLUTION = 250,		// 0.25 deg
	//SICK_SCAN_DEG_CENTER = 90000,		// 90 deg
	SICK_SCAN_VEC_SAME_OBJECT = 120,	// 0.12 m
	SICK_SCAN_VEC_IS_MOVED = 3,			// 0.03 m
	SICK_SCAN_OBJ_OFFSET = 3,			// 3 times
	CAR_WIDTH = 1200,
	CAR_HEIGH = 2000

};

class LastOfLiDAR {
public:
	LastOfLiDAR();
	~LastOfLiDAR();


	bool Initialize(string host_ip = "169.254.65.192");
	bool UnInitialize();
	bool StartCapture();
	bool StopCapture();
	bool GetValidDataRTheta(vector<pair<int, double> > &vecRTheta);
	static unsigned int CALLBACK proc(void* arg);
	
	void Conversion(vector<pair<int, double> > &vecRTheta, queue<vector<Point2d> > &finQVecXY);
	bool Average(queue<vector<Point2d> > &finQVecXY, vector<Point2d> &finVecXY);
	void Clustering(vector<Point2d> &finVecXY, queue<vector<vector<double> > > &finObjData);
	bool Vector(queue<vector<vector<double> > > &finObjData, vector<Point2d> &finVecData, vector<bool> &finBoolData);
	//void DrawData(vector<Point2d> &finVecXY, queue<vector<vector<double> > > &finObjData, vector<Point2d> &finVecData, vector<bool> &finBoolData, Mat &imgLiDAR);
	//int funcLiDAR();
	bool m_bDataAvailable;

	//Using Data
	vector<pair<int, double> > validScans; //R, Theta 스캔 데이터 - 1프레임
	queue<vector<Point2d> > finQVecXY; //노이즈 포함 데이터 집합 - 5프레임
	vector<Point2d> finVecXY; //최종 좌표 집합 - 1프레임
	
	//Final Data
	queue<vector<vector<double> > > finObjData; //최종 데이터 집합 - 2프레임
	vector<Point2d> finVecData; //물체의 벡터 집합 - 1프레임
	vector<bool> finBoolData; //물체의 동정적 여부 집합 - 1프레임

	//GUI
	Mat imgLiDAR; //최종 이미지

private:
	bool ConnectToDevice(string host, int port);
	bool DisconnectToDevice();
	inline bool IsConnected() { return m_bConnect; }
	status_t QueryStatus();
	bool ScanContinuous(int start);
	void ParseSickData(string& strBuf);
	void ConvertRawToRTheta();

	enum {
		SICK_STX = 0x02,
		SICK_ETX = 0x03
	};

	// 상태
	bool m_bConnect;
	status_t m_status;

	// data
	vector<double> m_vecRawScans;
	vector<pair<int, double> > m_vecValidScanRTheta;

	// communication
	SOCKET m_sockDesc;
	WSADATA m_wsadata;
	string m_strRemain;
	int m_nSETContinuous;
	int m_nACKContinuous;

	// thread
	HANDLE m_hThread;
	volatile bool m_bRunThread;
};