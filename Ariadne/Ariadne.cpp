#include "Ariadne.h"
#include <iostream>
#include "math.h"
#include "qevent.h"
#include <codecvt>
#include <string>

using utf8_str = std::string;
using u16_str = std::wstring;

// This file has same function as HyeAhn View
using namespace std;

Ui::AriadneClass* Ariadne::ui = new Ui::AriadneClass;

Ariadne::Ariadne(QWidget *parent)
	: QMainWindow(parent)
{
	ui->setupUi(this);

	dataContainer = DataContainer::getInstance();

	QPixmap pix("C:/Users/D-Ace/Documents/DYibre/temp.png");
	ui->AriadneLogo->setPixmap(pix);

	//  -------------------  Sensor Thread control ------------------------- //

	platformCom = new PlatformCom;
	platformThread = new QThread;
	platformCom->moveToThread(platformThread);
	connect(platformThread, SIGNAL(started()), platformCom, SLOT(comPlatform()));
	connect(platformCom, SIGNAL(PlatformExit()), platformCom, SLOT(comPlatform()));

	gpsCom = new GPSCom;
	gpsThread = new QThread;
	gpsCom->moveToThread(gpsThread);
	connect(gpsThread, SIGNAL(started()), gpsCom, SLOT(comGPS()));
	connect(gpsCom, SIGNAL(GPSExit()), gpsCom, SLOT(comGPS()));

	lidarCom = new LidarCom;
	lidarThread = new QThread;
	lidarCom->moveToThread(lidarThread);
	connect(lidarThread, SIGNAL(started()), lidarCom, SLOT(comLidar()));
	connect(lidarCom, SIGNAL(LidarExit()), lidarCom, SLOT(comLidar()));

	scnn = new Scnn;
	scnnThread = new QThread;
	scnn->moveToThread(scnnThread);
	connect(scnnThread, SIGNAL(started()), scnn, SLOT(boostScnn()));
	connect(scnn, SIGNAL(drivingEnabled()), this, SLOT(onDrivingEnabled()));

	yolo = new Yolo;
	yoloThread = new QThread;
	yolo->moveToThread(yoloThread);
	connect(yoloThread, SIGNAL(started()), yolo, SLOT(comYolo()));

	//  -------------------  Driving control ------------------------- //

	driving = new Driving;
	drivingThread = new QThread;
	driving->moveToThread(drivingThread);

	//SWITCH LIDAR CONTROLING.
	connect(drivingThread, SIGNAL(started()), driving, SLOT(autoMode()));
	//connect(drivingThread, SIGNAL(started()), driving, SLOT(LOS()));


	view = new View;
	connect(driving, SIGNAL(send2View(int)), view, SLOT(comView(int)));

	//  -------------------  UI control ------------------------- //

	ui->comboBox_6->addItems({ "Drive" , "Neutral", "Reverse" }); // gaer items input
	ui->Btn_mission1->setStyleSheet("Text-align:left");
	ui->Btn_mission3->setStyleSheet("Text-align:left");
	ui->Btn_mission4->setStyleSheet("Text-align:left");
	ui->Btn_mission5->setStyleSheet("Text-align:left");
	ui->Btn_mission6->setStyleSheet("Text-align:left");
	ui->Btn_mission7->setStyleSheet("Text-align:left");
	ui->Btn_mission8->setStyleSheet("Text-align:left");
	ui->Btn_mission9->setStyleSheet("Text-align:left");


	// --------------------- Platform control Using UI ---------------------------------//

	QObject::connect(ui->Btn_mission1, SIGNAL(clicked()), this, SLOT(clicked_btn_mission1()));
	QObject::connect(ui->Btn_mission3, SIGNAL(clicked()), this, SLOT(clicked_btn_mission3()));
	QObject::connect(ui->Btn_mission4, SIGNAL(clicked()), this, SLOT(clicked_btn_mission4()));
	QObject::connect(ui->Btn_mission5, SIGNAL(clicked()), this, SLOT(clicked_btn_mission5()));
	QObject::connect(ui->Btn_mission6, SIGNAL(clicked()), this, SLOT(clicked_btn_mission6()));
	QObject::connect(ui->Btn_mission7, SIGNAL(clicked()), this, SLOT(clicked_btn_mission7()));
	QObject::connect(ui->Btn_mission8, SIGNAL(clicked()), this, SLOT(clicked_btn_mission8()));
	QObject::connect(ui->Btn_mission9, SIGNAL(clicked()), this, SLOT(clicked_btn_mission9()));

	QObject::connect(ui->pushButton_2, SIGNAL(clicked()), this, SLOT(clicked_btn_sensor()));
	QObject::connect(ui->pushButton_3, SIGNAL(clicked()), this, SLOT(clicked_btn_driving()));
	QObject::connect(ui->Btn_gearInput, SIGNAL(clicked()), this, SLOT(gear_input()));
	QObject::connect(ui->Btn_up, SIGNAL(clicked()), this, SLOT(clicked_speed_up()));
	QObject::connect(ui->Btn_down, SIGNAL(clicked()), this, SLOT(clicked_speed_down()));
	QObject::connect(ui->Btn_left, SIGNAL(clicked()), this, SLOT(clicked_steer_left()));
	QObject::connect(ui->Btn_right, SIGNAL(clicked()), this, SLOT(clicked_steer_right()));
	QObject::connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(clicked_E_stop()));
	QObject::connect(ui->Btn_auto, SIGNAL(clicked()), this, SLOT(clicked_auto()));
	QObject::connect(ui->Btn_manual, SIGNAL(clicked()), this, SLOT(clicked_manual()));
	QObject::connect(ui->Btn_Bust, SIGNAL(clicked(bool)), this, SLOT(clicked_btn_bust(bool)));
	QObject::connect(ui->Btn_kidsafe, SIGNAL(clicked(bool)), this, SLOT(clicked_btn_kidsafe(bool)));


	// ------------------- UI update for Platform & GPS status ----------------------//

	connect(platformCom, SIGNAL(AorMChanged(int)), this, SLOT(onAorMChanged(int)));
	connect(platformCom, SIGNAL(EStopChanged(int)), this, SLOT(onEStopChanged(int)));
	connect(platformCom, SIGNAL(GearChanged(int)), this, SLOT(onGearChanged(int)));
	connect(platformCom, SIGNAL(SpeedChanged(int)), this, SLOT(onSpeedChanged(int)));
	connect(platformCom, SIGNAL(SteerChanged(int)), this, SLOT(onSteerChanged(int)));
	connect(platformCom, SIGNAL(BreakChanged(int)), this, SLOT(onBreakChanged(int)));
	connect(platformCom, SIGNAL(EncChanged(int)), this, SLOT(onEncChanged(int)));

	connect(gpsCom, SIGNAL(latitudeChanged(double)), this, SLOT(onLatitudeChanged(double)));
	connect(gpsCom, SIGNAL(longitudeChanged(double)), this, SLOT(onLongitudeChanged(double)));
	connect(gpsCom, SIGNAL(headingChanged(double)), this, SLOT(onHeadingChanged(double)));

	// ------------------- UI update for Mission ----------------------//

	QObject::connect(driving, SIGNAL(currentMission(int)), this, SLOT(onCurrentMission(int)));
	QObject::connect(yolo, SIGNAL(BustExist(bool)), this, SLOT(onBustExist(bool)));
	QObject::connect(yolo, SIGNAL(KidsafeExist(bool)), this, SLOT(onKidsafeExist(bool)));

	TimerUIUpdate = new QTimer(this);
	QTimer::connect(TimerUIUpdate, &QTimer::timeout, this, &Ariadne::updateUI);

}

void Ariadne::closeEvent(QCloseEvent *e) {
	//delete platformCom;
	//delete lidarCom;
	//delete gpsCom;
	//delete scnn;
	delete yolo;
	//delete view;
}

// This function is to change comport numbers from CString to QString.
CString ConvertQstringtoCString(QString qs)
{
	std::string utf8_text = qs.toUtf8().constData();
	CString cs(utf8_text.c_str());
	return cs;
}
void Ariadne::clicked_auto() {
	dataContainer->setValue_UtoP_AorM(1);
}

void Ariadne::clicked_manual() {
	dataContainer->setValue_UtoP_AorM(0);
}

void Ariadne::onDrivingEnabled() {
	Sleep(1000);
	ui->pushButton_3->setEnabled(true);
}

void Ariadne::onCurrentMission(int id) {
	ui->Btn_mission1->setEnabled(true);
	ui->Btn_mission3->setEnabled(true);
	ui->Btn_mission4->setEnabled(true);
	ui->Btn_mission5->setEnabled(true);
	ui->Btn_mission6->setEnabled(true);
	ui->Btn_mission7->setEnabled(true);
	ui->Btn_mission8->setEnabled(true);
	ui->Btn_mission9->setEnabled(true);

	switch (id) {
	case PARKING:
		//ui->Btn_mission1->setStyleSheet("background-color: rgb()");
		ui->Btn_mission1->setEnabled(false);
		ui->plainTextEdit->appendPlainText("Parking Mission Start");
		break;
	case INTER_LEFT:
		ui->Btn_mission3->setEnabled(false);
		ui->plainTextEdit->appendPlainText("Intersection left mission start");
		break;
	case INTER_RIGHT:
		ui->Btn_mission4->setEnabled(false);
		ui->plainTextEdit->appendPlainText("Intersection right mission start");
		break;
	case INTER_STRAIGHT:
		ui->Btn_mission5->setEnabled(false);
		ui->plainTextEdit->appendPlainText("intersection straight mission start");
		break;
	case INTER_STOP:
		ui->Btn_mission6->setEnabled(false);
		ui->plainTextEdit->appendPlainText("intersection stop mission start");
		break;
	case STATIC_OBSTACLE:
		ui->Btn_mission7->setEnabled(false);
		ui->plainTextEdit->appendPlainText("Static Obstacle Mission Start");
		break;
	case DYNAMIC_OBSTACLE:
		ui->Btn_mission8->setEnabled(false);
		ui->plainTextEdit->appendPlainText("Dynamic Obstacle Mission Start");
		break;
	case BASIC:
		ui->Btn_mission9->setEnabled(false);
		ui->plainTextEdit->appendPlainText("Basic PASIV algorithm start");
		break;
	}

}

void Ariadne::onBustExist(bool bust) {
	if (bust)
		ui->label_21->setStyleSheet("background-color: rgb(255, 51, 51)");
	else
		ui->label_21->setStyleSheet("background-color: rgb(0, 204, 102)");
}

void Ariadne::onKidsafeExist(bool kidsafe) {
	if (kidsafe)
		ui->label_20->setStyleSheet("background-color: rgb(255, 51, 51)");
	else
		ui->label_20->setStyleSheet("background-color: rgb(0, 204, 102)");
}

// This function is to start communication with sensor.
void Ariadne::clicked_btn_sensor() {
	//SENSOR SWITCH

	AutoPortFinder();

	if (!scnnThread->isRunning()) { scnnThread->start(); }

	if (!yoloThread->isRunning()){ yoloThread->start(); }

	//if (!platformThread->isRunning()) { platformThread->start(); }
	
	//if (!lidarThread->isRunning()) { lidarThread->start(); }

	//if (!gpsThread->isRunning()) { gpsThread->start(); }

	TimerSensorStatus = new QTimer(this);
	QTimer::connect(TimerSensorStatus, &QTimer::timeout, this, &Ariadne::updateSensorStatus);
	TimerSensorStatus->start(1000);

	ui->pushButton_3->setEnabled(false);
	
}

void Ariadne::clicked_btn_bust(bool bust) {
	/// if bust is on(true), speed ratio will be decreased.
	if (bust) { dataContainer->setValue_speed_ratio_bust(SPEED_RATIO_LOW);
	ui->label_21->setStyleSheet("background-color: rgb(255, 51, 51)");
	}
	else { dataContainer->setValue_speed_ratio_bust(1); 
	ui->label_21->setStyleSheet("background-color: rgb(0, 204, 102)");
	}
}

void Ariadne::clicked_btn_kidsafe(bool kidsafe) {
	/// if kidsafe is on(true) speed ratio will be decreased.
	if (kidsafe) { dataContainer->setValue_speed_ratio_kid(SPEED_RATIO_LOW);
	ui->label_20->setStyleSheet("background-color: rgb(255, 51, 51)");
	}
	else { dataContainer->setValue_speed_ratio_kid(1); 
	ui->label_20->setStyleSheet("background-color: rgb(0, 204, 102)");
	}
}


// This function is to start driving
void Ariadne::clicked_btn_driving() {

	//lidar, scnn 켰을 때만 실행
	//if (!drivingThread->isRunning())
	//	drivingThread->start();

	/// 0829 오전 6시 46분 DY: 하단 TimerUIUpdate를 clicked_btn_sensors() 에 옮겨서 테스트해보니 여기서 에러가 남.
	/// 하지만 데이터파싱은 잘 됨을 확인함.( sensorstatus.cpp 파일 430줄 ) 
	/// 우선 케이시티가서 디버깅할때 에러나지 말라고 여기 주석처리 해둘게 보경 UI update는 나중에 시간날때 천천히 살펴봐
	
	TimerUIUpdate->start(100);
}

void Ariadne::updateUI() {

	ui->pathmap->setPixmap(QPixmap::fromImage(dataContainer->getValue_ui_pathmap()));
	
	////yolo 켰을 때만 실행
	vector<int> arr = dataContainer->getValue_yolo_missions();
	QString str;

	if (arr[0] == 0) str = QString("STOP");
	else if (arr[0] == 1) str = QString("LEFT");
	else if (arr[0] == 2) str = QString("STRAIGHT");
	else if (arr[0] == 3) str = QString("RIGHT");
	else str = QString("None");

	ui->label_28->setText(str);
	ui->lcdNumber_16->display(arr[1]);
	ui->lcdNumber_18->display(arr[2]);
	ui->lcdNumber_19->display(arr[3]);
	ui->lcdNumber_20->display(arr[4]);
	ui->lcdNumber_21->display(arr[5]);
	ui->lcdNumber_22->display(arr[6]);
	

	////scnn 켰을때만 실행
	arr = dataContainer->getValue_scnn_existLanes();

	if (arr[0] == 1) str = QString("White Lane");
	else if (arr[0] == 2) str = QString("Yellow Lane");
	else if (arr[0] == 3) str = QString("Bus Lane");
	else str = QString("None");
	ui->label_31->setText(str);

	if (arr[1] == 1) str = QString("White Lane");
	else if (arr[1] == 2) str = QString("Yellow Lane");
	else if (arr[1] == 3) str = QString("Bus Lane");
	else str = QString("None");
	ui->label_32->setText(str);

	if (arr[2] == 1) str = QString("White Lane");
	else if (arr[2] == 2) str = QString("Yellow Lane");
	else if (arr[2] == 3) str = QString("Bus Lane");
	else str = QString("None");
	ui->label_33->setText(str);

	if (arr[3] == 1) str = QString("White Lane");
	else if (arr[3] == 2) str = QString("Yellow Lane");
	else if (arr[3] == 3) str = QString("Bus Lane");
	else str = QString("None");
	ui->label_34->setText(str);

		
}

void Ariadne::clicked_btn_mission1() {
	AutoPortFinder();
	dataContainer->setValue_yolo_missionID(PARKING);
}

void Ariadne::clicked_btn_mission2() {
	dataContainer->setValue_yolo_missionID(INTER_READY);
}

void Ariadne::clicked_btn_mission3() {
	dataContainer->setValue_yolo_missionID(INTER_LEFT);	
}

void Ariadne::clicked_btn_mission4() {
	dataContainer->setValue_yolo_missionID(INTER_RIGHT);
}

void Ariadne::clicked_btn_mission5() {
	dataContainer->setValue_yolo_missionID(INTER_STRAIGHT);
}

void Ariadne::clicked_btn_mission6() {
	dataContainer->setValue_yolo_missionID(INTER_STOP);
}

void Ariadne::clicked_btn_mission7() {
	dataContainer->setValue_yolo_missionID(STATIC_OBSTACLE);	
}

void Ariadne::clicked_btn_mission8() {
	dataContainer->setValue_yolo_missionID(DYNAMIC_OBSTACLE);
}

void Ariadne::clicked_btn_mission9() {
	dataContainer->setValue_yolo_missionID(BASIC);
}

// These functions is to control gplatform
void Ariadne::gear_input()
{
	QString qs;
	qs = ui->comboBox_6->currentText();
	if (qs == "Drive") { dataContainer->setValue_UtoP_GEAR(0); }
	else if (qs == "Neutral") { dataContainer->setValue_UtoP_GEAR(1); }
	else { dataContainer->setValue_UtoP_GEAR(2); }
}

void Ariadne::clicked_speed_up() { dataContainer->setValue_UtoP_SPEED(dataContainer->getValue_UtoP_SPEED() + 1); }
void Ariadne::clicked_speed_down() { dataContainer->setValue_UtoP_SPEED(dataContainer->getValue_UtoP_SPEED() - 1); }
void Ariadne::clicked_steer_left() { dataContainer->setValue_UtoP_STEER(dataContainer->getValue_UtoP_STEER() - 100); }
void Ariadne::clicked_steer_right() { dataContainer->setValue_UtoP_STEER(dataContainer->getValue_UtoP_STEER() + 100); }
void Ariadne::clicked_E_stop()
{
	if (dataContainer->getValue_UtoP_E_STOP() == 0) { dataContainer->setValue_UtoP_E_STOP(1); }
	else { dataContainer->setValue_UtoP_E_STOP(0); }
}

void Ariadne::keyPressEvent(QKeyEvent *event) {
	if (event->key() == Qt::Key_Left) { clicked_steer_left(); }
	if (event->key() == Qt::Key_Right) { clicked_steer_right(); }
	if (event->key() == Qt::Key_Up) { clicked_speed_up(); }
	if (event->key() == Qt::Key_Down) { clicked_speed_down(); }
	if (event->key() == Qt::Key_Enter) { clicked_E_stop(); }
}

//  _    _ _____    _    _ _____  _____       _______ ______
// | |  | |_   _|  | |  | |  __ \|  __ \   /\|__   __|  ____|
// | |  | | | |    | |  | | |__) | |  | | /  \  | |  | |__
// | |  | | | |    | |  | |  ___/| |  | |/ /\ \ | |  |  __|
// | |__| |_| |_   | |__| | |    | |__| / ____ \| |  | |____
//  \____/|_____|   \____/|_|    |_____/_/    \_\_|  |______|
// this part is for UI updating functions; display, slots... etc

void Ariadne::onAorMChanged(int Number) { ui->lcdNumber->display(Number); }
void Ariadne::onEStopChanged(int Number) { ui->lcdNumber_2->display(Number); }
void Ariadne::onGearChanged(int Number) { ui->lcdNumber_3->display(Number); }
void Ariadne::onSpeedChanged(int Number) { ui->lcdNumber_4->display(Number); }
void Ariadne::onSteerChanged(int Number) { ui->lcdNumber_5->display(Number); }
void Ariadne::onBreakChanged(int Number) { ui->lcdNumber_6->display(Number); }
void Ariadne::onEncChanged(int Number) { ui->lcdNumber_7->display(Number); }
void Ariadne::onLatitudeChanged(double Number) { ui->lcdNumber_8->display(Number); }

void Ariadne::onLongitudeChanged(double Number) { ui->lcdNumber_9->display(Number); }
void Ariadne::onHeadingChanged(double Number) { ui->lcdNumber_10->display(Number); }

void Ariadne::AutoPortFinder() {

	SP_DEVINFO_DATA devInfoData = {};
	devInfoData.cbSize = sizeof(devInfoData);

	// get the tree containing the info for the ports
	HDEVINFO hDeviceInfo = SetupDiGetClassDevs(&GUID_DEVCLASS_PORTS,
		0,
		nullptr,
		DIGCF_PRESENT
	);
	if (hDeviceInfo == INVALID_HANDLE_VALUE)
	{
		return;
	}

	// iterate over all the devices in the tree
	int nDevice = 0;
	while (SetupDiEnumDeviceInfo(hDeviceInfo,            // Our device tree
		nDevice++,            // The member to look for
		&devInfoData))
	{
		DWORD regDataType;
		DWORD reqSize = 0;

		//428843652
		// find the size required to hold the device info
		SetupDiGetDeviceRegistryProperty(hDeviceInfo, &devInfoData, SPDRP_HARDWAREID, nullptr, nullptr, 0, &reqSize);
		BYTE* hardwareId = new BYTE[(reqSize > 1) ? reqSize : 1];
		// now store it in a buffer

		if (SetupDiGetDeviceRegistryProperty(hDeviceInfo, &devInfoData, SPDRP_HARDWAREID, &regDataType, hardwareId, sizeof(hardwareId) * reqSize, nullptr))
		{
			// find the size required to hold the friendly name
			reqSize = 0;
			SetupDiGetDeviceRegistryProperty(hDeviceInfo, &devInfoData, SPDRP_FRIENDLYNAME, nullptr, nullptr, 0, &reqSize);

			BYTE* friendlyName = new BYTE[(reqSize > 1) ? reqSize : 1];
			//TCHAR friendly_name[256];
			// now store it in a buffer


			if (!SetupDiGetDeviceRegistryProperty(hDeviceInfo, &devInfoData, SPDRP_FRIENDLYNAME, nullptr, friendlyName, sizeof(friendlyName) * reqSize, nullptr))
			{
				// device does not have this property set
				memset(friendlyName, 0, reqSize > 1 ? reqSize : 1);
			}
			else {
				CString hwID((const wchar_t*)hardwareId);
				CString strPort((const wchar_t*)friendlyName);
				CString port = strPort.Mid(reqSize / 2 - 6, 4);
				//printf("port = %S\n port name = %S\n port num = %S\n", strPort, portName, port);
				if (hwID == "AX99100MF\\AX99100_COM") {
					dataContainer->setValue_gps_port(port);
				}
				else if (hwID == "USB\\VID_067B&PID_2303&REV_0300") {
					dataContainer->setValue_platform_port(port);
				}
			}
			// use friendlyName here
			delete[] friendlyName;

		}
		delete[] hardwareId;
	}
}

// This function is to change UI according to Sensor communication status
void Ariadne::updateSensorStatus()
{
	// Platform communication
	if (dataContainer->getValue_platform_status() > 5) {
		ui->statusPlatform->setStyleSheet("background-color: rgb(0, 204, 102);");
		ui->statusPlatform->setFixedWidth(60);
	}
	else if (dataContainer->getValue_platform_status() > 0) {
		ui->statusPlatform->setStyleSheet("background-color: yellow;");
		ui->statusPlatform->setFixedWidth(40);
	}
	else {
		ui->statusPlatform->setStyleSheet("background-color: rgb(255, 51, 51);");
		ui->statusPlatform->setFixedWidth(20);
	}
	dataContainer->setValue_platform_status(0);

	// LiDAR communication code
	if (dataContainer->getValue_lidar_status() > 5) {
		ui->statusLidar->setStyleSheet("background-color: rgb(0, 204, 102);");
		ui->statusLidar->setFixedWidth(60);
	}
	else if (dataContainer->getValue_lidar_status() > 0) {
		ui->statusLidar->setStyleSheet("background-color: yellow;");
		ui->statusLidar->setFixedWidth(40);
	}
	else {
		ui->statusLidar->setStyleSheet("background-color: rgb(255, 51, 51);");
		ui->statusLidar->setFixedWidth(20);
	}
	dataContainer->setValue_lidar_status(0);

	// GPS communication code
	if (dataContainer->getValue_gps_status() > 5) {
		ui->statusGps->setStyleSheet("background-color: rgb(0, 204, 102);");
		ui->statusGps->setFixedWidth(60);
	}
	else if (dataContainer->getValue_gps_status() > 0) {
		ui->statusGps->setStyleSheet("background-color: yellow;");
		ui->statusGps->setFixedWidth(40);
	}
	else {
		ui->statusGps->setStyleSheet("background-color: rgb(255, 51, 51);");
		ui->statusGps->setFixedWidth(20);
	}
	dataContainer->setValue_gps_status(0);

	// scnn communication code
	if (dataContainer->getValue_scnn_status() > 5) {
		ui->statusScnn->setStyleSheet("background-color: rgb(0, 204, 102);");
		ui->statusScnn->setFixedWidth(60);
	}
	else if (dataContainer->getValue_scnn_status() > 0) {
		ui->statusScnn->setStyleSheet("background-color: yellow;");
		ui->statusScnn->setFixedWidth(40);
	}
	else {
		ui->statusScnn->setStyleSheet("background-color: rgb(255, 51, 51);");
		ui->statusScnn->setFixedWidth(20);
	}
	dataContainer->setValue_scnn_status(0);

	// yolo communication code
	if (dataContainer->getValue_yolo_status() > 5) {
		ui->statusYolo->setStyleSheet("background-color: rgb(0, 204, 102);");
		ui->statusYolo->setFixedWidth(60);
	}
	else if (dataContainer->getValue_yolo_status() > 0) {
		ui->statusYolo->setStyleSheet("background-color: yellow;");
		ui->statusYolo->setFixedWidth(40);
	}
	else {
		ui->statusYolo->setStyleSheet("background-color: rgb(255, 51, 51);");
		ui->statusYolo->setFixedWidth(20);
	}
	dataContainer->setValue_yolo_status(0);

}

Ui::AriadneClass* Ariadne::getUI() { return ui; }

/// ----------------- Platform Communication Function -------------- ///

PlatformCom::PlatformCom()
{
	dataContainer = DataContainer::getInstance();
	ui = Ariadne::getUI();
}

void PlatformCom::comPlatform() {
	cout << "platform start" << endl;

	/// dataContainer->getValue_platform_port()
	if (_platform.OpenPort(dataContainer->getValue_platform_port()))
	{
		_platform.ConfigurePort(CBR_115200, 8, FALSE, NOPARITY, ONESTOPBIT);
		_platform.SetCommunicationTimeouts(0, 0, 0, 0, 0);

		while (loopStatusPlatform)
		{
			if (_platform.MyCommRead()) {}
			else {
				_platform.ClosePort();
				emit(PlatformExit());
				return;
			}
			_platform.MyCommWrite();
			dataContainer->updateValue_platform_status();

			emit(AorMChanged(dataContainer->getValue_PtoU_AorM()));
			emit(EStopChanged(dataContainer->getValue_PtoU_E_STOP()));
			emit(SpeedChanged(dataContainer->getValue_PtoU_SPEED()));
			emit(SteerChanged(dataContainer->getValue_PtoU_STEER()));
			emit(GearChanged(dataContainer->getValue_PtoU_GEAR()));
			emit(BreakChanged(dataContainer->getValue_PtoU_BRAKE()));
			emit(EncChanged(dataContainer->getValue_PtoU_ENC()));

			Sleep(100);
		}
	}
	else {
		cout << "platform not connect\n";
		emit(PlatformExit());
		return;
	}
}

/// ------------------ GPS Communication Function -----------------///

#define threshold 0.5
#define PI 3.14159265358979323846
#define radiRatio 1 / 298.257223563
#define lamda 129.0*PI/180.0
#define Eoff 500000
#define k0 0.9996
#define radi 6378137
#define GPSCOM L"COM14"

bool sign;

CSerialPort _gps;
///void HeroNeverDies();
vector <double>UTM(double lat, double lng);
bool GEOFENCE(double x, double y, vector<vector<double>> map_link, double heading);

GPSCom::GPSCom() {
	ui = Ariadne::getUI();
	dataContainer = DataContainer::getInstance();

}

//얘가 실시간 좌표찍는 애임 , 클릭버튼했을 대 이 함수호출되도록했음
void GPSCom::comGPS() { // rt ; Real Time

	// dataContainer->getValue_gps_port()
	if (_gps.OpenPort(GPSCOM)) {

		_gps.ConfigurePortW(CBR_115200, 8, FALSE, NOPARITY, ONESTOPBIT);
		_gps.SetCommunicationTimeouts(0, 0, 0, 0, 0);
		string tap;
		string tap2;
		vector<string> vec;
		while (1) {
			BYTE * pByte = new BYTE[512]; // 2028
			if (_gps.ReadByte(pByte, 512)) {

				dataContainer->updateValue_gps_status();

				pByte[511] = '\0'; // 2027

				const char * p = (const char*)pByte;

				stringstream str(p);

				while (getline(str, tap, '\n')) {
					//cout << tap;
					stringstream qwe(tap);

					while (getline(qwe, tap2, ',')) {
						vec.push_back(tap2);
					}
					//cout << vec[0] << endl;

					cout << vec.size() << endl;

					if (vec.size() > 8) {
						if (vec[0] == "$GNRMC" && vec[2] == "A") {
							//ofile << vec[0] << ',' << vec[3] << ',' << vec[5]  << endl;
							_lat = ((atof(vec[3].c_str()) - 3500) / 60) + 35; // 35랑 128은 상황에 따라 바꿔줘야함
							_lng = ((atof(vec[5].c_str()) - 12800) / 60) + 128;
							heading = atof(vec[8].c_str()) *CV_PI / 180; //[rad]

							vector<double >utm = UTM(_lat, _lng);
							lat = utm[0];
							lng = utm[1];

							dataContainer->resetValue_gps_valid();
							dataContainer->setValue_gps_heading(heading);
							dataContainer->setValue_gps_latitude(lat);
							dataContainer->setValue_gps_longitude(lng);

							emit latitudeChanged(lat / 1000);
							emit longitudeChanged(lng / 1000); /// 숫자가 너무 커서 나눴음
							emit headingChanged(heading);

							cout << lat << "    " << lng << endl;
						}
						else if (vec[2] == "V") {
							dataContainer->count_gps_valid();

						}
					}
					vec.clear();
				}
			}
			else {
				cout << "GPS not connect" << endl;
				_gps.ClosePort();
				emit(GPSExit());
				return;
			}
			Sleep(100);
		}
	}
	else {
		cout << "port not connect" << endl;
		emit(GPSExit());
	}
}

vector <double>UTM(double lat, double lng) { /// This function is to calculate UTM parameters.
	double lat_rad = lat * PI / 180.0;
	double lng_rad = lng * PI / 180.0;

	double n = radiRatio / (2 - radiRatio);
	double A = radi * (1 - n + 5.0 / 4.0 * (pow(n, 2) - pow(n, 3)) + 81.0 / 64.0 * (pow(n, 4) - pow(n, 5)));
	double B = 3.0 / 2.0 * radi*(n - pow(n, 2) + 7.0 / 8.0 * (pow(n, 3) - pow(n, 4)) + 55.0 / 64.0* pow(n, 5));
	double C = 15.0 / 16.0 * radi*(pow(n, 2) - pow(n, 3) + 3.0 / 4.0 * (pow(n, 4) - pow(n, 5)));
	double D = 35.0 / 48.0 *radi*(pow(n, 3) - pow(n, 4) + 11.0 / 16.0 * pow(n, 5));
	double Ed = 315.0 / 512.0 * radi*(pow(n, 4) - pow(n, 5));
	double S = A * lat_rad - B * sin(2 * lat_rad) + C * sin(4 * lat_rad) - D * sin(6 * lat_rad) + Ed * sin(8 * lat_rad);
	double esq = radiRatio * (2 - radiRatio);
	double v = radi / sqrt(1 - esq * pow(sin(lat_rad), 2));
	double esqd = radiRatio * (2 - radiRatio) / pow((1 - radiRatio), 2);

	double T1 = k0 * S;
	double T2 = v * sin(lat_rad)*cos(lat_rad)*k0 / 2.0;
	double T3 = v * sin(lat_rad)*pow(cos(lat_rad), 3)*k0 / 24.0 * (5.0 - pow(tan(lat_rad), 2) + 9.0 * esqd*pow(cos(lat_rad), 2) + 4.0 * pow(esqd, 2)*pow(cos(lat_rad), 4));
	double T4 = v * sin(lat_rad)*pow(cos(lat_rad), 5)*k0 / 720.0 * (61.0 - 58.0 * pow(tan(lat_rad), 2) + pow(tan(lat_rad), 4) + 270.0 * esqd*pow(cos(lat_rad), 2) - 330.0 * esqd*pow(tan(lat_rad)*cos(lat_rad), 2)
		+ 445.0 * pow(esqd, 2)*pow(cos(lat_rad), 4) + 324.0 * pow(esqd, 3)*pow(cos(lat_rad), 6) - 680.0 *pow(tan(lat_rad), 2)* pow(esqd, 2)*pow(cos(lat_rad), 4) + 88.0 * pow(esqd, 4)*pow(cos(lat_rad), 8) - 600.0 * pow(esqd, 3)*pow(tan(lat_rad), 2)*pow(cos(lat_rad), 6)
		- 192.0 * pow(esqd, 4)*pow(tan(lat_rad), 2)*pow(cos(lat_rad), 8));
	double T5 = v * sin(lat_rad)*pow(cos(lat_rad), 7)*k0 / 40320.0 * (1385.0 - 3111.0 * pow(tan(lat_rad), 2) + 543.0 * pow(tan(lat_rad), 4) - pow(tan(lat_rad), 6));
	double T6 = v * cos(lat_rad)*k0;
	double T7 = v * pow(cos(lat_rad), 3)*k0 / 6.0 * (1.0 - pow(tan(lat_rad), 2) + esqd * pow(cos(lat_rad), 2));
	double T8 = v * pow(cos(lat_rad), 5)*k0 / 120.0 * (5.0 - 18.0 * pow(tan(lat_rad), 2) + pow(tan(lat_rad), 4) + 14.0 * esqd*pow(cos(lat_rad), 2) - 58.0 * esqd*pow(tan(lat_rad)*cos(lat_rad), 2) + 13.0 * pow(esqd, 2)*pow(cos(lat_rad), 4) + 4.0 * pow(esqd, 3)*pow(cos(lat_rad), 6)
		- 64.0 * pow(esqd*tan(lat_rad), 2)*pow(cos(lat_rad), 4) - 24.0 * pow(tan(lat_rad), 2)*pow(esqd, 3)*pow(cos(lat_rad), 6));
	double T9 = v * pow(cos(lat_rad), 7)*k0 / 5040.0 * (61.0 - 479.0 * pow(tan(lat_rad), 2) + 179.0 * pow(tan(lat_rad), 4) - pow(tan(lat_rad), 6));

	double dellng = lng_rad - lamda;

	//offset
	double N = T1 + pow(dellng, 2)*T2 + pow(dellng, 4)*T3 + pow(dellng, 6)*T4 + pow(dellng, 8)*T5 + 1.32;
	double E = Eoff + dellng * T6 + pow(dellng, 3)*T7 + pow(dellng, 5)*T8 + pow(dellng, 7)*T9;

	vector<double> utm;
	utm.push_back(E);
	utm.push_back(N);
	return utm;
}

double L1Distance(vector<double> coor1, vector<double> coor2) {
	double L1 = 0;
	L1 = abs(coor1[0] - coor2[0]) + abs(coor1[1] - coor2[1]);

	return L1;
}

double L2Distance(double x2, double y2, double x1 = 0, double y1 = 0) {
	double L2 = 0;
	L2 = pow(pow(x2 - x1, 2) + pow(y2 - y1, 2), 0.5);

	return L2;
}

vector<int> mins(double x, double y, vector<vector<double>> map_link_cut, vector<double> unitHeading) {

	int min = 0;
	int smin = 0;
	int ssmin = 0;
	int sssmin = 0;
	vector <double> rt_postion{ x,y };
	double temp = 1000000000;

	for (int i = 0; i < map_link_cut.size(); i++) {
		double ref = L1Distance(rt_postion, map_link_cut[i]);
		if (ref <= temp) {
			sssmin = ssmin;
			ssmin = smin;
			smin = min;
			min = i;
			temp = ref;
		}
	}
	int lastPoint = 0;

	if ((unitHeading[0] * (map_link_cut[ssmin][0] - x) + unitHeading[1] * (map_link_cut[ssmin][1]) - y) >= 0) {
		lastPoint = ssmin;
	}
	if ((unitHeading[0] * (map_link_cut[ssmin][0] - x) + unitHeading[1] * (map_link_cut[ssmin][1]) - y) < 0) {
		if ((unitHeading[0] * (map_link_cut[sssmin][0] - x) + unitHeading[1] * (map_link_cut[sssmin][1]) - y) >= 0) {
			lastPoint = sssmin;
		}
		else {
			lastPoint = ssmin;
		}
	}

	vector<int> temp2{ lastPoint, smin, min };
	return temp2;
}


//�������� ���͸� ����
vector<double> makeVector(double x1, double y1, double x2, double y2) {
	double _x = x1 - x2;
	double _y = y1 - y2;
	vector<double> temp{ _x,_y };

	return temp;
}

//���� ����
double outerProduct(double x1, double y1, double x2, double y2) {
	double temp = x1 * y2 - y1 * x2; // 2���� ��������

	// �����̸� ������ �߾��� �����ʹ���, �����̸� ���ʹ���
	return temp;
}

//������ �̿��ؼ� �Ÿ����ϴ� �˰���������
double getDistance(vector<double> a, vector<double> h) {
	vector<double> _d;

	_d.push_back(a[0] - (a[0] * h[0] + a[1] * h[1])*h[0]);
	_d.push_back(a[1] - (a[0] * h[0] + a[1] * h[1])*h[1]);
	//(unitHeading[0], unitHeading[1], vec1[0], vec1[1]);
	double d = outerProduct(h[0], h[1], a[0], a[1]);
	return d;
}

double getError(double x, double y, vector<vector<double>> map_link, vector<double> unitHeading) {

	vector<vector<double>> map_link_cut;

	//map_link���� ������ǥ�� �������� �ȿ� �����ִ� ��ǥ���� ���� ����
	//��, ������ġ�� ���� ����������ŭ �ڸ��� ����
	for (int i = 0; i < map_link.size(); i++) {
		if ((map_link[i][0] >= x - 50 && map_link[i][0] <= x + 50) && (map_link[i][1] >= y - 50 && map_link[i][1] <= y + 50)) {
			map_link_cut.push_back(map_link[i]);
		}
	}

	vector<int> _mins = mins(x, y, map_link_cut, unitHeading); // return {min , smin, ssmin} �ε��� �ѹ�

	//������ ���� ������ ���� ���ͻ���
	vector<double> vec1 = makeVector(map_link_cut[_mins[0]][0], map_link_cut[_mins[0]][1], x, y);
	vector<double> vec2 = makeVector(map_link_cut[_mins[1]][0], map_link_cut[_mins[1]][1], x, y);
	vector<double> vec3 = makeVector(map_link_cut[_mins[2]][0], map_link_cut[_mins[2]][1], x, y);

	//������ ���鿡 ���� �Ÿ�
	double d1 = getDistance(vec1, unitHeading);
	double d2 = getDistance(vec2, unitHeading);
	double d3 = getDistance(vec3, unitHeading);

	double Error = (abs(d1) + abs(d2) + abs(d3)) / 3;

	return Error;
}

// threshole�� 0.9���� ���Ƿ� ����, ���������� �ϴ� 0.9���� ������(�ʿ信 ���� �ٲ� �� ����)
// threshold = 0.9
// if Error > threshold , GEOFENCE = True
// else GEOFENCE = FALUSE
bool GEOFENCE(double x, double y, vector<vector<double>> map_link, double heading) {

	vector<double> unitHeading{ sin(heading*(PI / 180)),cos(heading*(PI / 180)) }; // ������ ���ֺ��� ����

	double Error = getError(x, y, map_link, unitHeading);
	if (Error > 0) {
		sign = true;
	}
	else {
		sign = false;
	}
	if (abs(Error) >= threshold) {
		return true;
	}
	else {
		return false;
	}
}