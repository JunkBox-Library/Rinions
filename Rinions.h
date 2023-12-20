/**
	Rinions.h  by Fumi.Iseki (c)


*/
#pragma once


#ifndef __AFXWIN_H__
	#error "PCH に対してこのファイルをインクルードする前に 'stdafx.h' をインクルードしてください"
#endif


#include  "resource.h"       // メイン シンボル

#include  "ExNiLib/ExNiDevice.h"
#include  "WinBaseLib.h"
#include  "MFCLib.h"
#include  "CameraView.h"

#include  "Rinions_config.h"


extern     CRinionsApp   theApp;


using namespace jbxl;
using namespace jbxwl;




////////////////////////////////////////////////////////////////////////////////////
//
// CRinionsApp:
//

class CRinionsApp : public CWinApp, public CAppCallBack
{
public:
	CRinionsApp();
	virtual ~CRinionsApp();

public:
	CMainFrame*			pMainFrame;
	CLogWndFrame*		pLogFrame;
	CLogWndDoc*			pLogDoc;
		
	CCameraView*		pSensorView;
	CCameraView*		pDepthView;
//	CDx3DDirectView*	pModelView;

	CExNiJoints*		niJoints;
	CExNiDevice*		niDevice;
	CExNiNetwork*		niNetwork;
	CExNiSHMemory*		sharedMem;

	CParameterSet		appParam;

	BOOL				destructorf;

public:
	CMultiDocTemplate*  pDocTemplLOG;
	CMultiDocTemplate*  pDocTemplSNR;
	CMultiDocTemplate*  pDocTemplDPT;
	CMultiDocTemplate*  pDocTemplMDL;

public:
	RECT	windowSize;

	BOOL	isLoggedIn;
	BOOL	isRecvData;
	BOOL	dataSaving;		// データを一時作業ファイルに保存中
	BOOL	savedData;		// 有効（保存可能）な一時作業ファイル有り
	BOOL	loadedData;		// メモリに読み込み済みのデータ有り（ONIファイル用）
	BOOL	logfSaving;		// ログデータを保存中
	BOOL	logfSending;	// ログデータを送信中

	int		cameraScale;
	int		depthScale;
	int		motorTilt;

public:
	BOOL	InitNiDevice(void);
	void	ShutdownNiDevice(void);

	void	SensorWndOpen(void);
	void	StopSensorThread(void);
	void	DepthWndOpen(void);
//	void	DepthWndClose(void);

	void	MakeCameraImage(void);

	void	StartSpeechRecognition(void);
	void	StopSpeechRecognition(void);
	void	ReStartSpeechRecognition(void);

public:
	//
	void	updateStatusBar(BOOL init=FALSE);
	void	updateMenuBar(CMenu* menu = NULL);
	void	enableSensorWndMenu(CMenu* menu, BOOL enable);
	void	enableDepthWndMenu(CMenu* menu, BOOL enable);
	void	enableLoadedWndMenu(CMenu* menu, BOOL enable);

	void	closeNetwork(BOOL logout);

	//char*	configFilePath;
	char*	lockFilePath;

	char*	makeConfigFilePath(void);
	//void	readWindowSize (void);
	//void	readConfigFile (void);
	//void	writeConfigFile(void);

	char*   makeLockFile(BOOL& exec);
	void    deleteLockFile(void);


public:
	virtual BOOL InitInstance();

	virtual void ViewDestructor(CExView* view);
	virtual void ViewDestructor(CExTextView* view);
	virtual void FrameDestructor(CExTextFrame* frm);

	afx_msg void OnAppAbout();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnModelingWndOpen();
	afx_msg void OnLogWndOpen();
	afx_msg void OnSnsrWndOpen_x1();
	afx_msg void OnSnsrWndOpen_x2();
	afx_msg void OnSnsrWndOpen_x3();
	afx_msg void OnSnsrWndOpen_x4();
	afx_msg void OnLddWndOpen_x1();
	afx_msg void OnLddWndOpen_x2();
	afx_msg void OnLddWndOpen_x3();
	afx_msg void OnLddWndOpen_x4();
	afx_msg void OnDepthWndOpen_x1();
	afx_msg void OnDepthWndOpen_x2();
	afx_msg void OnDepthWndOpen_x3();
	afx_msg void OnDepthWndOpen_x4();
	afx_msg void OnSettingLogingMode();
	afx_msg void OnSettingViewMode();
	afx_msg void OnSettingOutData();
	afx_msg void OnSettingNetwork();
	afx_msg void OnSettingAnimation();
	afx_msg void OnSettingDataSave();
	afx_msg void OnSettingSpeech();
	afx_msg void OnSettingDevice();
	afx_msg void OnSettingMotion();
	afx_msg void OnAppLogin();
	afx_msg void OnAppLogout();
	afx_msg void OnAppDataLoad();
	afx_msg void OnAppDataSave();
	afx_msg void OnAppDataRecv();
	afx_msg void OnAppReboot();
	afx_msg void OnAppInitialize();
};



