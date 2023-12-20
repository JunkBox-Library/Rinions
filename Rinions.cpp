/**
	Rinions.cpp  by Fumi.Iseki (c)

		http://www.nsl.tuis.ac.jp/
*/


#include  "stdafx.h"

#include  "Rinions.h"

#include  "BasicLib.h"
#include  "MathLib.h"
#include  "GraphLib.h"
#include  "MessageBoxDLG.h"
#include  "Rotation.h"

#include  "SensorFrame.h"
#include  "DepthFrame.h"
#include  "CameraView.h"

#include  "Dialog/SetLogMode.h"
#include  "Dialog/SetViewMode.h"
#include  "Dialog/SetNetwork.h"
#include  "Dialog/SetAnimation.h"
#include  "Dialog/SetDataSave.h"
#include  "Dialog/SetDevice.h"
#include  "Dialog/SetMotion.h"
#include  "Dialog/SetOutData.h"
#include  "Dialog/SetSpeech.h"




using namespace jbxl;
using namespace jbxwl;




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CRinionsApp メインクラス
//

BEGIN_MESSAGE_MAP(CRinionsApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CRinionsApp::OnAppAbout)
	// 標準のファイル基本ドキュメント コマンド
	ON_COMMAND(ID_MODEL_WND_OPEN, &CRinionsApp::OnModelingWndOpen)
	ON_COMMAND(ID_LOG_WND_OPEN, &CRinionsApp::OnLogWndOpen)
	ON_COMMAND(ID_SNSR_WND_OPEN_1, &CRinionsApp::OnSnsrWndOpen_x1)
	ON_COMMAND(ID_SNSR_WND_OPEN_2, &CRinionsApp::OnSnsrWndOpen_x2)
	ON_COMMAND(ID_SNSR_WND_OPEN_3, &CRinionsApp::OnSnsrWndOpen_x3)
	ON_COMMAND(ID_SNSR_WND_OPEN_4, &CRinionsApp::OnSnsrWndOpen_x4)
	ON_COMMAND(ID_LOAD_WND_OPEN_1, &CRinionsApp::OnLddWndOpen_x1)
	ON_COMMAND(ID_LOAD_WND_OPEN_2, &CRinionsApp::OnLddWndOpen_x2)
	ON_COMMAND(ID_LOAD_WND_OPEN_3, &CRinionsApp::OnLddWndOpen_x3)
	ON_COMMAND(ID_LOAD_WND_OPEN_4, &CRinionsApp::OnLddWndOpen_x4)
	ON_COMMAND(ID_DPTH_WND_OPEN_1, &CRinionsApp::OnDepthWndOpen_x1)
	ON_COMMAND(ID_DPTH_WND_OPEN_2, &CRinionsApp::OnDepthWndOpen_x2)
	ON_COMMAND(ID_DPTH_WND_OPEN_3, &CRinionsApp::OnDepthWndOpen_x3)
	ON_COMMAND(ID_DPTH_WND_OPEN_4, &CRinionsApp::OnDepthWndOpen_x4)
	ON_COMMAND(ID_SETTING_LOGINGMODE, &CRinionsApp::OnSettingLogingMode)
	ON_COMMAND(ID_SETTING_VIEWMODE, &CRinionsApp::OnSettingViewMode)
	ON_COMMAND(ID_SETTING_OUTDATA, &CRinionsApp::OnSettingOutData)
	ON_COMMAND(ID_SETTING_NETWORK, &CRinionsApp::OnSettingNetwork)
	ON_COMMAND(ID_SETTING_ANIMATION, &CRinionsApp::OnSettingAnimation)
	ON_COMMAND(ID_SETTING_SAVEDATA, &CRinionsApp::OnSettingDataSave)
	ON_COMMAND(ID_SETTING_SPEECH, &CRinionsApp::OnSettingSpeech)
	ON_COMMAND(ID_SETTING_DEVICE, &CRinionsApp::OnSettingDevice)
	ON_COMMAND(ID_SETTING_MOTION, &CRinionsApp::OnSettingMotion)
	ON_COMMAND(ID_APP_LOGIN, &CRinionsApp::OnAppLogin)
	ON_COMMAND(ID_APP_LOGOUT, &CRinionsApp::OnAppLogout)
	ON_COMMAND(ID_APP_DATA_LOAD, &CRinionsApp::OnAppDataLoad)
	ON_COMMAND(ID_APP_DATA_SAVE, &CRinionsApp::OnAppDataSave)
	ON_COMMAND(ID_APP_DATA_RECV, &CRinionsApp::OnAppDataRecv)
	ON_COMMAND(ID_APP_REBOOT, &CRinionsApp::OnAppReboot)
	ON_COMMAND(ID_APP_INIT, &CRinionsApp::OnAppInitialize)
END_MESSAGE_MAP()




// CRinionsApp コンストラクション
CRinionsApp::CRinionsApp()
{
	pDocTemplLOG = NULL;
	pDocTemplSNR = NULL;
	pDocTemplDPT = NULL;
	pDocTemplMDL = NULL;
	pMainFrame	 = NULL;

	pLogFrame	 = NULL;
	pLogDoc		 = NULL;
	pSensorView	 = NULL;
	pDepthView	 = NULL;
//	pModelView	 = NULL;

	niDevice	 = NULL;
	niJoints	 = NULL;
	niNetwork	 = NULL;
	sharedMem	 = NULL;

	memset(&windowSize, -1, sizeof(RECT));
	appParam.init();

	isLoggedIn	 = FALSE;
	isRecvData	 = FALSE;

	dataSaving	 = FALSE;
	savedData	 = FALSE;
	loadedData	 = FALSE;
	logfSaving	 = FALSE;
	logfSending	 = FALSE;

	destructorf	 = FALSE;

	lockFilePath = NULL;

	cameraScale	 = 2;
	motorTilt	 = 0;
}



CRinionsApp::~CRinionsApp()
{
	DEBUG_INFO("DESTRUCTOR: CRinions START");
//	DEBUG_LOG("DESTRUCTOR: CRinions START");

	destructorf = TRUE;

	deleteLockFile();
	if (lockFilePath!=NULL) {
		::free(lockFilePath);
		lockFilePath = NULL;
	}

	//
	ShutdownNiDevice();
	Dx9ReleaseInterface();
	
	//
	DEBUG_INFO("DESTRUCTOR: CRinions END");
//	DEBUG_LOG("DESTRUCTOR: CRinions END");
}



CRinionsApp  theApp;



// CRinionsApp 初期化

BOOL  CRinionsApp::InitInstance()
{
	CWinApp::InitInstance();

	SetRegistryKey(_T("Rinions by Fumi.Iseki and NSL"));
	LoadStdProfileSettings(4);  // 標準の INI ファイルのオプションをロードします (MRU を含む)

	//
	CMultiDocTemplate* pDocTemplate;

	// ログ ウィドウ
	pDocTemplate = new CMultiDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CLogWndDoc),
		RUNTIME_CLASS(CLogWndFrame),
		RUNTIME_CLASS(CLogWndView));
	if (!pDocTemplate) return FALSE;
	AddDocTemplate(pDocTemplate);
	pDocTemplLOG = pDocTemplate;

	// イメージ（センサー） ウィドウ
	/*
	pDocTemplate = new CMultiDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CExDocument),
		RUNTIME_CLASS(CSensorFrame), 
		RUNTIME_CLASS(CDxDirectView));
	if (!pDocTemplate) return FALSE;
	AddDocTemplate(pDocTemplate);
	pDocTemplSNR = pDocTemplate;
	*/
	pDocTemplate = new CMultiDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CExDocument),
		RUNTIME_CLASS(CSensorFrame), 
		RUNTIME_CLASS(CCameraView));
	if (!pDocTemplate) return FALSE;
	AddDocTemplate(pDocTemplate);
	pDocTemplSNR = pDocTemplate;

	// 深度 ウィドウ
	pDocTemplate = new CMultiDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CExDocument),
		RUNTIME_CLASS(CDepthFrame), 
		RUNTIME_CLASS(CCameraView));
	if (!pDocTemplate) return FALSE;
	AddDocTemplate(pDocTemplate);
	pDocTemplDPT = pDocTemplate;

/*
	// 3Dモデル ウィンドウ (未実装)
	pDocTemplate = new CMultiDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CExDocument),
		RUNTIME_CLASS(CExFrame), 
		RUNTIME_CLASS(CDx3DDirectView));
	if (!pDocTemplate) return FALSE;
	AddDocTemplate(pDocTemplate);
	pDocTemplMDL = pDocTemplate;
*/

	// メインフレーム ウィンドウを作成
	pMainFrame = new CMainFrame;
	pMainFrame->pApp = this;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME)) return FALSE;
	m_pMainWnd = pMainFrame;

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Locale
	setSystemLocale();

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Lock File & Config File
	//
	BOOL ret;
	lockFilePath = makeLockFile(ret);
	if (!ret) return FALSE;

	//configFilePath = makeConfigFilePath();

	///////////////////////////////////////////////////////////////////////////////////////////////
	// ウィンドウサイズ
	//
	int wx = RINIONS_WND_XSIZE;
	int wy = RINIONS_WND_YSIZE;
	int rx = GetSystemMetrics(SM_CXSCREEN);			// X方向の解像度
	int ry = GetSystemMetrics(SM_CYSCREEN) - 14;	// Y方向の解像度（-ツールバー）

	// Default Size
	windowSize.left   = rx/2 - wx/2;
	windowSize.right  = wx;
	windowSize.top    = ry/2 - wy/2;
	windowSize.bottom = wy;

	//
	//readWindowSize();	// read  windowSize
	appParam.readWindowSize(&windowSize);

	int sx, sy;
	if (windowSize.left<=rx) {
		sx = windowSize.left;
		wx = windowSize.right;
		if (wx<=0 || wx>=rx) wx = RINIONS_WND_XSIZE; 
	}
	else {
		sx = rx/2 - RINIONS_WND_XSIZE/2;
	}
	if (windowSize.top<=ry) {
		sy = windowSize.top;
		wy = windowSize.bottom;
		if (wy<=0 || wy>=ry) wy = RINIONS_WND_YSIZE;
	}
	else {
		sy = ry/2 - RINIONS_WND_YSIZE/2;
	}

	m_pMainWnd->SetWindowPos(NULL, sx, sy, wx, wy, 0);

	///////////////////////////////////////////////////////////////////////////////////////////////
	// メインフレームウィンドウの表示
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Rebooting
	//
	/*
	ret = WaitPidTerm(5);
	if (!ret) {
		MessageBoxDLG(IDS_STR_ERROR, IDS_STR_ERR_REBOOT, MB_OK, m_pMainWnd); // "プログラムの再起動に失敗しました"
		return FALSE;
	}*/

	///////////////////////////////////////////////////////////////////////////////////////////////
	// DirectX9 の初期化
	//
	ret = Dx9CreateInterface(this);
	if (!ret) {
		MessageBoxDLG(IDS_STR_ERROR, IDS_STR_FAIL_GET_DX9DEV, MB_OK, m_pMainWnd); // "DirectX9の初期化に失敗しました"
		return FALSE;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	// NI の初期化
	//
	ret = InitNiDevice();

	return ret;
}






BOOL  CRinionsApp::InitNiDevice(void)
{
	///////////////////////////////////////////////////////////////////////////////////////////////
	// NI デバイスの生成
	//
	niDevice = new CExNiDevice();
	if (niDevice==NULL) return FALSE;
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// 設定ファイルの読み込み
	//
	//readConfigFile();
	appParam.readConfigFile();

	///////////////////////////////////////////////////////////////////////////////////////////////
	// 初期設定
	//
	BOOL use_image = appParam.isUseImage;	// Backup
	appParam.isUseImage = FALSE;
	niDevice->setParameter(appParam);		// デバイスに関する設定は反映されない

	niDevice->pMainFrame = pMainFrame;
	niDevice->mWnd		 = m_pMainWnd;
	niDevice->hWnd		 = m_pMainWnd->m_hWnd;
	niDevice->nowSDKLib  = appParam.nextSDKLib;

	///////////////////////////////////////////////////////////////////////////////////////////////
	// NI Device インスタンスの初期化
	CMessageBoxDLG* mesgBox = NULL;

	///////////////////////////////////////////////////
	// NIデバイスの初期化
	mesgBox = MessageBoxDLG(IDS_STR_INFO, IDS_STR_INIT_DEVICE, m_pMainWnd);		// "NIデバイスの初期化中"
	//
	BOOL ret = niDevice->init();
	updateStatusBar();
	if (mesgBox!=NULL) delete(mesgBox);
	if (!ret) MessageBoxDLG(IDS_STR_INFO, niDevice->getErrorMessage(), MB_OK, m_pMainWnd);

	//
	appParam.isUseImage = use_image;		// Restore
	appParam.nextSDKLib = niDevice->nowSDKLib;
	appParam.enableFaceDetect = niDevice->enableFaceDetect;
	niDevice->setParameter(appParam);
	appParam.saveConfigFile();

	///////////////////////////////////////////////////
	//  カメラ イメージの初期化
	if (ret && appParam.isUseImage) {
		MakeCameraImage();
	}

	///////////////////////////////////////////////////
	// Speech Platform の初期化
	appParam.enableSpeechReco = FALSE;
	if (appParam.langSpeech.IsEmpty()) appParam.langSpeech = ::getResourceLocale();
	if (ret) {
		StartSpeechRecognition();
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	// リングバッファの初期化
	//
	ret = niDevice->initRingBuffer();
	if (!ret) {
		MessageBoxDLG(IDS_STR_WARN, niDevice->getErrorMessage(), MB_OK, m_pMainWnd);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	// メニューの設定
	//
	updateMenuBar();
	updateStatusBar();

	///////////////////////////////////////////////////////////////////////////////////////////////
	// ネットワークと共有メモリ，ジョイント制約クラスの生成
	//
	niJoints  = new CExNiJoints(niDevice->nowSDKLib);
	niNetwork = new CExNiNetwork();
	sharedMem = new CExNiSHMemory();

	niDevice->setNetwork(niNetwork);
	niDevice->setSharedMem(sharedMem);
	niDevice->setNiJoints(niJoints);
	niDevice->setNiFileDev();

// OpenCV
	// for OpenNI
	if (niDevice->nowSDKLib==NiSDK_OpenNI || niDevice->nowSDKLib==NiSDK_OpenNI2) {
		if (appParam.useFaceDetect) {
			if (niJoints->init_opencv()) {
				//
#ifdef ENABLE_OPENCV	// OpenCV
				if (!niJoints->enableOpencvFace) {
					MessageBoxDLG(IDS_STR_WARN, IDS_STR_NOT_READ_FACE_CSCDFL, MB_OK, m_pMainWnd);
					//appParam.useFaceDetect = FALSE;
				}
				else {
					if (!niJoints->enableOpencvEyes) {
						MessageBoxDLG(IDS_STR_WARN, IDS_STR_NOT_READ_EYES_CSCDFL, MB_OK, m_pMainWnd);
					}
					if (!niJoints->enableOpencvMouth) {
						MessageBoxDLG(IDS_STR_WARN, IDS_STR_NOT_READ_MOUTH_CSCDFL, MB_OK, m_pMainWnd);
					}
				}
#endif
			}
			else {
				niJoints->clear_opencv();
				MessageBoxDLG(IDS_STR_WARN, IDS_STR_UNABLE_FACE_DETECTION, MB_OK, m_pMainWnd);
			}
		}
	}

	//
	niDevice ->setParameter(appParam);
	niNetwork->setSharedMem(sharedMem);
	sharedMem->setLocalAnimationIndex();

	return TRUE;
}




void  CRinionsApp::ShutdownNiDevice(void)
{
	DEBUG_INFO("CRinionsApp::ShutdownNiDevice(): START");
//	DEBUG_LOG("CRinionsApp::ShutdownNiDevice(): START");

	if (niNetwork!=NULL) {
		if (isLoggedIn) OnAppLogout();
	}

	//
	if (niDevice!=NULL) {
		niDevice->clearLocalAvgFPS();
		//
		if (niDevice->has_device) {
			//
			niDevice->deleteSpeech(!destructorf);
			//niDevice->deleteSpeech();
			//
			if (dataSaving) {
				niDevice->stopRecorde();
				DEBUG_INFO("CRinionsApp::ShutdownNiDevice(): STOP RECORD");
				dataSaving = FALSE;
			}
			if (logfSaving) {
				niDevice->stopLogRecorde();
				logfSaving = FALSE;
			}
			if (logfSending) {
				niDevice->stopLogTransfer();
				logfSending = FALSE;
			}
	
			if (niDevice->getDevState()==NI_STATE_DETECT_EXEC) {
				niDevice->stopDetection();
				DEBUG_INFO("CRinionsApp::ShutdownNiDevice(): STOP DETECTION");
			}
			if (niDevice->enableUSBDev) {
				niDevice->setLEDColor(NI_LED_BLINK_GREEN);
				niDevice->closeUSBDevice();
			}
		}
		delete(niDevice);
		niDevice = NULL;
	}
	//::Sleep(NIDEVICE_TERM_WAIT_TIME);

	//
	if (niNetwork!=NULL) {
		delete(niNetwork);
		niNetwork = NULL;
	}

	//
	if (sharedMem!=NULL) {
		delete(sharedMem);
		sharedMem = NULL;
	}

	//
	dataSaving	 = FALSE;
	savedData	 = FALSE;
	loadedData	 = FALSE;
	logfSaving	 = FALSE;

	cameraScale	 = 2;
	motorTilt	 = 0;

	DEBUG_INFO("CRinionsApp::ShutdownNiDevice(): END");
//	DEBUG_LOG("CRinionsApp::ShutdownNiDevice(): END");
}




////////////////////////////////////////////////////////////////////////////////////////////
// アプリケーションのバージョン情報に使われる CAboutDlg ダイアログ

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// ダイアログ データ
	enum { IDD = IDD_ABOUTBOX };


protected:
	BOOL		 DoLocalTerminateAction(LPCTSTR com);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnSpeechEvent(WPARAM wParam, LPARAM lParam);
};



CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}



void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}



BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	ON_MESSAGE(JBXWL_WM_SPEECH_EVENT, &CAboutDlg::OnSpeechEvent)
END_MESSAGE_MAP()



//////////////////////////////////////////////////////////////////
// ダイアログを実行するためのアプリケーション コマンド
void CRinionsApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}





//////////////////////////////////////////////////////////////////
// Destruction of child window
//

// センサー ビュー
void CRinionsApp::ViewDestructor(CExView* view)
{
	if (view==pSensorView) {
		DEBUG_INFO("CRinionsApp::ViewDestructor(): STOP SENSOR VIEW");

		if (niDevice->pSensorFrame!=NULL) {
			niDevice->setSensorFramePtr(NULL);
			::Sleep(NIDEVICE_WAIT_TIME);
		}
		pSensorView = NULL;
		//
		if (sharedMem!=NULL) sharedMem->isTracking = FALSE;
	}

	//
	else if (view==pDepthView) {
		DEBUG_INFO("CRinionsApp::ViewDestructor(): STOP DEPTH VIEW");

		if (niDevice->pDepthFrame!=NULL) {
			niDevice->setDepthFramePtr(NULL);
			::Sleep(NIDEVICE_WAIT_TIME);
		}
		pDepthView = NULL;
	}

	//
/*
	else if (view==pModelView) {
		DEBUG_INFO("CRinionsApp::ViewDestructor(): STOP MODEL VIEW");

		if (niDevice->pModelFrame!=NULL) {
			niDevice->setModelFramePtr(NULL);
			::Sleep(NIDEVICE_WAIT_TIME);
		}
		pModelView = NULL;
	}
*/

	updateMenuBar();
}





// ログ ビュー
void CRinionsApp::ViewDestructor(CExTextView* view)
{
	if (view==pLogFrame->pView) {
		DEBUG_INFO("CRinionsApp::ViewDestructor(): STOP LOG VIEW");
		niDevice->clearLogDocPtr();
		niNetwork->pLogDoc = NULL;
		pLogDoc = NULL;
		::Sleep(NIDEVICE_WAIT_TIME);
	}

	updateMenuBar();
}




// ログ フレーム
void CRinionsApp::FrameDestructor(CExTextFrame* frm)
{
	if (frm==pLogFrame) {
		DEBUG_INFO("CRinionsApp::FrameDestructor(): STOP LOG FRAME");

		niDevice->setLogFramePtr(NULL);
		pLogFrame = NULL;
	}

	updateMenuBar();
}






///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CRinionsApp メニュー
//

/////////////////////////////////////////////////////////////////
// ファイル メニュー
//

//
// ログイン
//
void CRinionsApp::OnAppLogin()
{
	if (isLoggedIn) return;
	
	// "ログイン中 ..."
	CMessageBoxDLG* mesgBox = MessageBoxDLG(IDS_STR_INFO, IDS_STR_PROCSING_LOGIN, m_pMainWnd);	
	//unsigned char ret = niNetwork->serverLogin(appParam.animationSrvr, appParam.serverPort, appParam.groupID);
	unsigned char ret = niNetwork->serverLogin();
	if (mesgBox!=NULL) delete(mesgBox);

	//
	if (ret==ANM_COM_ERROR_SERVER_DOWN) {
		// "ログインサーバが応答しません"
		MessageBoxDLG(IDS_STR_ATTNSN, IDS_STR_NORSPNS_SERVER, MB_OK, m_pMainWnd);
		return;
	}
	if (ret==ANM_COM_ERROR_TIMEOUT) {
		// "中継プロセスが応答しません"
		MessageBoxDLG(IDS_STR_ATTNSN, IDS_STR_NORSPNS_RELAY, MB_OK, m_pMainWnd);
		return;
	}
	else if (ret==ANM_COM_ERROR_SOCKET) {
		// "通信ソケットをオープンできません"
		MessageBoxDLG(IDS_STR_ATTNSN, IDS_STR_NOTOPEN_SOCKET, MB_OK, m_pMainWnd);
		return;
	}
	else if (ret==ANM_COM_REPLY_FORBIDDEN) {
		// "このサーバを使用することはできません"
		MessageBoxDLG(IDS_STR_ATTNSN, IDS_STR_FRBDDN_SERVER, MB_OK, m_pMainWnd);
		return;
	}
	else if (ret==ANM_COM_REPLY_REGIST_DUPLI) {
		// "指定されたアニメーションUUIDは既に登録されています"
		MessageBoxDLG(IDS_STR_ATTNSN, IDS_STR_DUPLI_ANIM, MB_OK, m_pMainWnd);
		return;
	}
	else if (ret==ANM_COM_REPLY_REGIST_ALRDY) {
		// "既にログイン済みです"
		MessageBoxDLG(IDS_STR_ATTNSN, IDS_STR_ALREADY_LOGIN, MB_OK, m_pMainWnd);
		//return;
	}
	else if (ret!=ANM_COM_REPLY_OK) {
		TCHAR buf[32];
		sntprintf(buf, 32, _T("\nreturn code = 0x%02x"), ret);
		CString err;
		err.LoadString(IDS_STR_NOT_LOGIN);	// "サーバにログインできませんでした"
		err += buf;
		MessageBoxDLG(IDS_STR_ATTNSN, err, MB_OK, m_pMainWnd);
		return;
	}

	isLoggedIn = TRUE;
	if (sharedMem!=NULL) {
		if (appParam.netOutMode==NETonly) sharedMem->clearLocalAnimationIndex();
	}

	OnAppDataRecv();

	//
	CMenu* menu = pMainFrame->GetMenu();
	menu->EnableMenuItem(ID_APP_LOGIN,     MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	menu->EnableMenuItem(ID_APP_LOGOUT,    MF_BYCOMMAND | MF_ENABLED);
	menu->EnableMenuItem(ID_APP_DATA_RECV, MF_BYCOMMAND | MF_ENABLED);

	updateStatusBar();
}



//
// ログアウト
//
void CRinionsApp::OnAppLogout()
{
	closeNetwork(TRUE);
}




void CRinionsApp::closeNetwork(BOOL logout)
{
	if (!isLoggedIn) return;
	isLoggedIn = FALSE;
	isRecvData = FALSE;

	if (niNetwork!=NULL) {
		if (logout) niNetwork->serverLogout();
		else        niNetwork->closeNetwork();
		::Sleep(NIDEVICE_WAIT_TIME);
	}

	if (niDevice->pSensorFrame!=NULL) {
		((CSensorFrame*)niDevice->pSensorFrame)->OnCalibrationStop();
	}
	if (sharedMem!=NULL) sharedMem->setLocalAnimationIndex();

	CMenu* menu = NULL;
	if (pMainFrame!=NULL) menu = pMainFrame->GetMenu();
	if (menu!=NULL) {
		menu->EnableMenuItem(ID_APP_LOGOUT,   MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		menu->EnableMenuItem(ID_APP_LOGIN,    MF_BYCOMMAND | MF_ENABLED);
		menu->CheckMenuItem(ID_APP_DATA_RECV, MF_BYCOMMAND | MF_UNCHECKED);
		menu->EnableMenuItem(ID_APP_DATA_RECV,MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}
	updateStatusBar();
}




//
// ネットワークからの Data 受信
//
void CRinionsApp::OnAppDataRecv()
{
	if (niNetwork==NULL) return;

	CMenu* menu = pMainFrame->GetMenu();

	if (!isRecvData) {
		niNetwork->openRecvSocket();
		if (niNetwork->recvSocket<=0) {
			// "注意", "データの受信ができません"
			MessageBoxDLG(IDS_STR_ATTNSN, IDS_STR_FAIL_RSCV, MB_OK, m_pMainWnd); 
			return;
		}
		isRecvData = TRUE;
		menu->CheckMenuItem(ID_APP_DATA_RECV, MF_BYCOMMAND | MF_CHECKED);
		//
		// THREAD_PRIORITY_NORMAL, THREAD_PRIORITY_HIGHEST	
		AfxBeginThread(niNetworkRecieveLoop, (LPVOID)niNetwork, THREAD_PRIORITY_TIME_CRITICAL);
	}
	else {
		menu->CheckMenuItem(ID_APP_DATA_RECV, MF_BYCOMMAND | MF_UNCHECKED);
		sharedMem->clearNetworkAnimationIndex();
		sharedMem->clearNetworkAnimationData();
		niNetwork->closeRecvSocket();
		isRecvData = FALSE;
	}
	updateStatusBar();
}



//
// Data 読み込み
//
void  CRinionsApp::OnAppDataLoad()
{
	CString mesg  = LoadString_byID(IDS_STR_SPECIFY_LOAD_FILE);
	CString fname = EasyGetOpenFileName((LPCTSTR)mesg, m_pMainWnd->m_hWnd);
	if (fname.IsEmpty()) return;

	CMessageBoxDLG* mbox = MessageBoxDLG(IDS_STR_INFO, IDS_STR_RDNG_FILE, m_pMainWnd);
	BOOL ret   = niDevice->openLoadFile(fname);
	loadedData = niDevice->loaded_oni;			// TRUE or FALSE
	if (mbox!=NULL) delete(mbox);

	//
	if (!ret) {
		MessageBoxDLG(IDS_STR_WARN, niDevice->getErrorMessage(), MB_OK, m_pMainWnd);
		return;
	}
	
	// ONI
	if (loadedData) {
		cameraScale = 2;	// 1/2
		SensorWndOpen();
	}
	//
	else {
		niDevice->execFilePlayer(isLoggedIn, fname);
		//		
		appParam = niDevice->appParam;
		niDevice->setParameter(appParam);
		appParam.saveConfigFile();
	}

	return;
}



//
// Data 保存
//
void  CRinionsApp::OnAppDataSave()
{
	//if (!niDevice->has_device) return;

	//
	CString mesg  = LoadString_byID(IDS_STR_SPECIFY_SAVE_FILE);
	CString fname = EasyGetSaveFileName((LPCTSTR)mesg, _T(""), m_pMainWnd->m_hWnd);
	if (fname.IsEmpty()) return;

	//
	int ret = niDevice->backupRecordeTempFile(fname, FALSE);	// FALSE: 上書きチェック

	if (ret>0) {
		niDevice->deleteRecordeTempFile();
		savedData = FALSE;
	}
	else {
		// "エラー", "ファイルの読み込みに失敗しました"
		DEBUG_ERR("CRinionsApp::OnAppDataSave(): ERROR: Fail to Write File (%d)", ret);
		MessageBoxDLG(IDS_STR_ERROR, IDS_STR_ERR_WRITE_FILE, MB_OK, m_pMainWnd);
		savedData = TRUE;
	}
	
//	savedData = FALSE;
	return;
}



//
// プログラムの再起動
//
void  CRinionsApp::OnAppReboot()
{
	int ret = MessageBoxDLG(IDS_STR_CNFRM, IDS_STR_CNFRM_REBOOT, MB_YESNO, m_pMainWnd);
	if (ret==IDNO) return;

	appParam.saveConfigFile();
	RebootProgram();	// => WaitPidTerm()

	return;
}



//
// NIデバイスの初期化
//
void  CRinionsApp::OnAppInitialize()
{
//	OnAppReboot();
//	return;

	//
	int ret = MessageBoxDLG(IDS_STR_CNFRM, IDS_STR_CNFRM_INIT, MB_YESNO, m_pMainWnd);
	if (ret==IDNO) return;

	updateStatusBar(TRUE);
	appParam.saveConfigFile();

	// "NIデバイスのリセット中"
	CMessageBoxDLG* mesgBox = MessageBoxDLG(IDS_STR_INFO, IDS_STR_RESET_DEVICE, m_pMainWnd);
	ShutdownNiDevice();
	if (mesgBox!=NULL) delete(mesgBox);

	//
	InitNiDevice();

	updateStatusBar();
	return;
}






/////////////////////////////////////////////////////////////////
//  ウィンドウ メニュー
//


void  CRinionsApp::OnModelingWndOpen()
{
/*
	if (pModelView!=NULL) return;		// Open 済み

	CExFrame* pfrm = CreateDocFrmView(pDocTemplMDL, this);

	if (pfrm!=NULL) {
		pfrm->pDoc->Title		= _T("Modeling Window");
		pfrm->pDoc->hasReadData = TRUE;                                           
		pfrm->pDoc->hasViewData = TRUE;                                        
		pfrm->pView->cnstXYRate = TRUE;
   		pfrm->pView->colorMode  = GRAPH_COLOR_BGRA;
		pfrm->pView->xsize		= niDevice->image_xsize;
		pfrm->pView->ysize		= niDevice->image_ysize;
		pModelView = (CDx3DDirectView*)pfrm->pView;

		int ret = ExecDocFrmView(pfrm);
		if (ret>0) {
			ExecDocFrmViewError(pMainFrame->m_hWnd, ret);
		}
	}
	else {
		ExecDocFrmViewError(pMainFrame->m_hWnd, MSG_DFV_FR_NULL);	// Frameへのポインタが NULLです
	}
*/

	return;
}



// Log Window
void CRinionsApp::OnLogWndOpen()
{
	if (pLogFrame==NULL) {
		pLogFrame = ExecLogWnd(pDocTemplLOG, _T("Log Window"), this);
		pLogDoc = pLogFrame->pDoc;
		niNetwork->pLogDoc = pLogDoc;
		niDevice->setLogFramePtr((CExTextFrame*)pLogFrame);
	}

	updateMenuBar();
	return;
}



// Camera Window
void CRinionsApp::OnSnsrWndOpen_x1()
{
	cameraScale = 1;	// 1/1
	
	niDevice->prvInputDevice = niDevice->inputDevice;
	niDevice->inputDevice = NiDevice_NI;
	SensorWndOpen();
	return;
}



void CRinionsApp::OnSnsrWndOpen_x2()
{
	cameraScale = 2;	// 1/2
	
	niDevice->prvInputDevice = niDevice->inputDevice;
	niDevice->inputDevice = NiDevice_NI;
	SensorWndOpen();
	return;
}



void CRinionsApp::OnSnsrWndOpen_x3()
{
	cameraScale = 3;	// 1/3

	niDevice->prvInputDevice = niDevice->inputDevice;
	niDevice->inputDevice = NiDevice_NI;
	SensorWndOpen();
	return;
}



void CRinionsApp::OnSnsrWndOpen_x4()
{
	cameraScale = 4;	// 1/4
	
	niDevice->prvInputDevice = niDevice->inputDevice;
	niDevice->inputDevice = NiDevice_NI;
	SensorWndOpen();
	return;
}



void CRinionsApp::OnLddWndOpen_x1()
{
	cameraScale = 1;	// 1/1
	
	niDevice->prvInputDevice = niDevice->inputDevice;
	niDevice->inputDevice = NiDevice_File;
	SensorWndOpen();
	return;
}



void CRinionsApp::OnLddWndOpen_x2()
{
	cameraScale = 2;	// 1/2
	
	niDevice->prvInputDevice = niDevice->inputDevice;
	niDevice->inputDevice = NiDevice_File;
	SensorWndOpen();
	return;
}



void CRinionsApp::OnLddWndOpen_x3()
{
	cameraScale = 3;	// 1/3

	niDevice->prvInputDevice = niDevice->inputDevice;
	niDevice->inputDevice = NiDevice_File;
	SensorWndOpen();
	return;
}



void CRinionsApp::OnLddWndOpen_x4()
{
	cameraScale = 4;	// 1/4
	
	niDevice->prvInputDevice = niDevice->inputDevice;
	niDevice->inputDevice = NiDevice_File;
	SensorWndOpen();
	return;
}




void CRinionsApp::SensorWndOpen()
{
	if (pSensorView!=NULL) return;		// Open 済み

	CExFrame* pfrm = CreateDocFrmView(pDocTemplSNR, this);

	if (pfrm!=NULL) {
		//		
		if (niDevice->prvInputDevice!=niDevice->inputDevice) {
			if (niDevice->prvInputDevice==NiDevice_File) loadedData = FALSE;
			if (niDevice->inputDevice==NiDevice_NI) {
				niDevice->restoreDevice();
				if (!niDevice->hasImageGenerator() && appParam.isUseImage) {
					MakeCameraImage();
				}
			}
		}

		//
		((CSensorFrame*)pfrm)->thisApp = this;
		pfrm->pDoc->Title		= _T("Camera Window");
		pfrm->pDoc->hasReadData = TRUE;                                           
		pfrm->pDoc->hasViewData = TRUE;                                        
		pfrm->pView->cnstXYRate	= TRUE;
   		pfrm->pView->colorMode  = GRAPH_COLOR_BGRA;		// for faster with Little Endian
		pfrm->pView->xsize		= niDevice->image_xsize/cameraScale;
		pfrm->pView->ysize		= niDevice->image_ysize/cameraScale;
		//
		pSensorView = (CCameraView*)pfrm->pView;
//		pSensorView->thisApp = this;

		// Tool Bar
		pfrm->toolBar->LoadToolBar(IDR_CALIB_TOOLBAR);	// Tool Bar
		if (niDevice->inputDevice==NiDevice_NI) {
			pfrm->toolBar->LoadToolBar(IDR_CALIB_TOOLBAR2);
		}
		else if (niDevice->inputDevice==NiDevice_File) {
			appParam.isMirroring = TRUE;
			niDevice->setParameter(appParam);
		}

		//
		niDevice->setImageScale(cameraScale);
		niDevice->setSensorFramePtr(pfrm);
		if (niDevice->hasImageGenerator() && niDevice->hasDepthGenerator()) niDevice->setViewPoint();
		
		// Thread
		pSensorView->m_handler = new EventHandler(FALSE);
		pSensorView->m_thread = AfxBeginThread(niDeviceEventLoop, (LPVOID)niDevice, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
		pSensorView->m_thread->m_bAutoDelete = FALSE;
		pSensorView->m_thread->ResumeThread();
		::Sleep(10);
	}
	//
	else {
		ExecDocFrmViewError(pMainFrame->m_hWnd, MSG_DFV_FR_NULL);	// Frameへのポインタが NULLです
	}

	updateMenuBar();
	return;
}



void CRinionsApp::StopSensorThread(void)
{
	if (pSensorView!=NULL) {
		pSensorView->stopThread();
	}
}




void  CRinionsApp::MakeCameraImage(void)
{
	// イメージの作成
	if (niDevice->has_device && !niDevice->hasImageGenerator() && niDevice->appParam.isUseImage) {
		CMessageBoxDLG* mesgBox = MessageBoxDLG(IDS_STR_INFO, IDS_STR_CREATE_IMAGE, m_pMainWnd);
		niDevice->createImageGenerator();
		if (mesgBox!=NULL) delete(mesgBox);
	}
}




void  CRinionsApp::StartSpeechRecognition(void)
{
	BOOL ret = TRUE;
	niDevice->enableSpeechReco = FALSE;
	//
	if (appParam.useSpeechReco) {
		CMessageBoxDLG* mesgBox = MessageBoxDLG(IDS_STR_INFO, IDS_STR_START_SPEECH, m_pMainWnd);// "音声認識機能の起動中"
		ret = niDevice->initSpeech();
		if (ret) ret = niDevice->createSpeech();			
		if (ret) ret = niDevice->startSpeech();
		if (mesgBox!=NULL) delete(mesgBox);
	}
	//
	if (!ret) MessageBoxDLG(IDS_STR_INFO, niDevice->getErrorMessage(), MB_OK, m_pMainWnd);

	appParam.enableSpeechReco = niDevice->enableSpeechReco;
//	if (!appParam.enableSpeechReco) appParam.useSpeechReco = FALSE;
	niDevice->setParameter(appParam);
	appParam.saveConfigFile();

	//
	updateStatusBar();
}




void  CRinionsApp::StopSpeechRecognition(void)
{
	CMessageBoxDLG* mesgBox = MessageBoxDLG(IDS_STR_INFO, IDS_STR_STOP_SPEECH, m_pMainWnd);	// "音声認識機能の停止中"
	niDevice->stopSpeech();
	if (mesgBox!=NULL) delete(mesgBox);

	//
	appParam.enableSpeechReco = FALSE;
	niDevice->setParameter(appParam);
	//
	updateStatusBar();
}




void  CRinionsApp::ReStartSpeechRecognition(void)
{
	BOOL ret = TRUE;

	CMessageBoxDLG* mesgBox = MessageBoxDLG(IDS_STR_INFO, IDS_STR_RESTART_SPEECH, m_pMainWnd);	// "音声認識機能の停止中"
	niDevice->stopSpeech();
	//
	niDevice->enableSpeechReco = FALSE;
	if (appParam.useSpeechReco) {
		ret = niDevice->initSpeech();
		if (ret) ret = niDevice->createSpeech();			
		if (ret) ret = niDevice->startSpeech();
	}
	if (mesgBox!=NULL) delete(mesgBox);
	//
	if (!ret) MessageBoxDLG(IDS_STR_INFO, niDevice->getErrorMessage(), MB_OK, m_pMainWnd);

	appParam.enableSpeechReco = niDevice->enableSpeechReco;
//	if (!appParam.enableSpeechReco) appParam.useSpeechReco = FALSE;
	niDevice->setParameter(appParam);
	//
	updateStatusBar();
}





// Depth Window
void CRinionsApp::OnDepthWndOpen_x1()
{
	depthScale = 1;	// 1/1
	
	DepthWndOpen();
	return;
}



void CRinionsApp::OnDepthWndOpen_x2()
{
	depthScale = 2;	// 1/2
	
	DepthWndOpen();
	return;
}



void CRinionsApp::OnDepthWndOpen_x3()
{
	depthScale = 3;	// 1/3

	DepthWndOpen();
	return;
}



void CRinionsApp::OnDepthWndOpen_x4()
{
	depthScale = 4;	// 1/4
	
	DepthWndOpen();
	return;
}





void CRinionsApp::DepthWndOpen()
{
	if (!niDevice->hasDepthGenerator()) return;
	if (pDepthView!=NULL) return;				// Open 済み

	CExFrame* pfrm = CreateDocFrmView(pDocTemplDPT, this);

	if (pfrm!=NULL) {
		//		
		((CDepthFrame*)pfrm)->thisApp = this;
		pfrm->pDoc->Title		= _T("Depth Window");
		pfrm->pDoc->hasReadData = TRUE;                                           
		pfrm->pDoc->hasViewData = TRUE;                                        
		pfrm->pView->cnstXYRate	= TRUE;
   		pfrm->pView->colorMode  = GRAPH_COLOR_MONO16;
		pfrm->pView->xsize		= niDevice->image_xsize/depthScale;
		pfrm->pView->ysize		= niDevice->image_ysize/depthScale;
		pfrm->pView->cMin		= 400;		// 40cm
		pfrm->pView->cMax		= 8000;		// 8m
		pfrm->pView->vMin		= 400;		// 40cm
		pfrm->pView->vMax		= 8000;		// 8m
		pfrm->toolBar->LoadToolBar(IDR_DEPTH_TOOLBAR);	// Tool Bar
		//
		pDepthView = (CCameraView*)pfrm->pView;
//		pDepthView->thisApp = this;

		int ret = ExecDocFrmView(pfrm);
		if (ret>0) {
			ExecDocFrmViewError(pMainFrame->m_hWnd, ret);
		}
		
		//
		niDevice->setDepthScale(depthScale);
		niDevice->setDepthFramePtr(pfrm);
	}
	//
	else {
		ExecDocFrmViewError(pMainFrame->m_hWnd, MSG_DFV_FR_NULL);	// Frameへのポインタが NULLです
	}

	updateMenuBar();
	return;
}






/////////////////////////////////////////////////////////////////
//　設定 メニュー
//

// NI デバイス
void CRinionsApp::OnSettingDevice()
{
	CSetDevice* devdlg = new CSetDevice(niDevice, appParam);
	if (devdlg==NULL) return;
	devdlg->DoModal();
	appParam = devdlg->getParameter();

	niDevice->setParameter(appParam);

	niDevice->setUseLED(TRUE);
	if (appParam.useDevLED) {
		niDevice->setLEDColor(NI_LED_GREEN);
	}
	else {
		niDevice->setLEDColor(NI_LED_BLINK_GREEN);
	}
	
	niDevice->setUseMotor(appParam.useDevMotor);
	niDevice->setUseLED(appParam.useDevLED);

	delete(devdlg);
	appParam.saveConfigFile();
}



// モーション設定
void CRinionsApp::OnSettingMotion()
{
	CSetMotion* mtndlg = NULL;
	
	BOOL prev_face_detect = appParam.useFaceDetect;

	if (niDevice->nowSDKLib==NiSDK_OpenNI || niDevice->nowSDKLib==NiSDK_OpenNI2) {
		mtndlg = new CSetOpenNIMotion(appParam);
	}
	else if (niDevice->nowSDKLib==NiSDK_Kinect) {
		mtndlg = new CSetKinectMotion(appParam);
	}
	if (mtndlg==NULL) return;
	
	mtndlg->DoModal();
	appParam = mtndlg->getParameter();
	delete(mtndlg);

//
	// for OpenNI
	if (niDevice->nowSDKLib==NiSDK_OpenNI || niDevice->nowSDKLib==NiSDK_OpenNI2) {
		if (appParam.useFaceDetect!=prev_face_detect) {
			if (appParam.useFaceDetect) {
				if (niJoints->init_opencv()) {
#ifdef ENABLE_OPENCV
					//
					if (!niJoints->enableOpencvFace) {
						MessageBoxDLG(IDS_STR_WARN, IDS_STR_NOT_READ_FACE_CSCDFL, MB_OK, m_pMainWnd);
						//appParam.useFaceDetect = FALSE;
					}
					else {
						if (!niJoints->enableOpencvEyes) {
							MessageBoxDLG(IDS_STR_WARN, IDS_STR_NOT_READ_EYES_CSCDFL, MB_OK, m_pMainWnd);
						}
						if (!niJoints->enableOpencvMouth) {
							MessageBoxDLG(IDS_STR_WARN, IDS_STR_NOT_READ_MOUTH_CSCDFL, MB_OK, m_pMainWnd);
						}
					}
#endif
				}
				else {
					niJoints->clear_opencv();
					MessageBoxDLG(IDS_STR_WARN, IDS_STR_UNABLE_FACE_DETECTION, MB_OK, m_pMainWnd);
				}
			}
			//else {
			//	niJoints->clear_opencv();
			//}
		}
	}

	// Kinect SDK
	else {
		if (appParam.useFaceDetect!=prev_face_detect) {
			if (appParam.useFaceDetect) {
				if (!appParam.enableFaceDetect) {
					MessageBoxDLG(IDS_STR_WARN, IDS_STR_UNABLE_FACE_DETECTION, MB_OK, m_pMainWnd);
				}
			}
		}
	}

	niDevice->setParameter(appParam);
	appParam.saveConfigFile();
}



// ログ ウィンドウ
void CRinionsApp::OnSettingLogingMode()
{
	CSetLogMode* logdlg = new CSetLogMode(niDevice->nowSDKLib, appParam);
	if (logdlg==NULL) return;
	logdlg->DoModal();
	appParam = logdlg->getParameter();

	niDevice ->setParameter(appParam);	
	if (niNetwork!=NULL) niNetwork->setParameter(appParam);

	delete(logdlg);
	appParam.saveConfigFile();
}



// センサー ウィンドウ
void CRinionsApp::OnSettingViewMode()
{
	CSetViewMode* viwdlg = new CSetViewMode(appParam, niDevice->inputDevice);
	if (viwdlg==NULL) return;
	viwdlg->DoModal();
	appParam = viwdlg->getParameter();

	niDevice->setParameter(appParam);

	delete(viwdlg);
	appParam.saveConfigFile();

	if (appParam.isUseImage) MakeCameraImage();
}



// データ出力
void CRinionsApp::OnSettingOutData()
{
	CSetOutData* smrdlg = new CSetOutData(appParam);
	if (smrdlg==NULL) return;
	smrdlg->DoModal();
	appParam =smrdlg->getParameter();

	niDevice->setParameter(appParam);
	if (niNetwork!=NULL) niNetwork->setParameter(appParam);
	if (sharedMem!=NULL) sharedMem->setParameter(appParam);

	delete(smrdlg);
	appParam.saveConfigFile();
}



// ネットワーク
void CRinionsApp::OnSettingNetwork()
{
	CSetNetwork* netdlg = new CSetNetwork(appParam, isLoggedIn);
	if (netdlg==NULL) return;
	netdlg->DoModal();
	appParam = netdlg->getParameter();

	niDevice->setParameter(appParam);	
	if (niNetwork!=NULL) niNetwork->setParameter(appParam);

	delete(netdlg);
	appParam.saveConfigFile();
}



// アニメーション UUID
void CRinionsApp::OnSettingAnimation()
{
	CSetAnimation* anmdlg = new CSetAnimation(appParam, isLoggedIn);
	if (anmdlg==NULL) return;
	anmdlg->DoModal();
	appParam = anmdlg->getParameter();

	if (niNetwork!=NULL) niNetwork->setParameter(appParam);
	if (sharedMem!=NULL) {
		sharedMem->setParameter(appParam);
		sharedMem->setLocalAnimationIndex();
	}

	delete(anmdlg);
	appParam.saveConfigFile();
}



// データ保存
void CRinionsApp::OnSettingDataSave()
{
//	BOOL fexst = (BOOL)file_exist(RINIONS_BVH_TEMPLATE);

	CSetDataSave* datdlg = new CSetDataSave(niDevice->nowSDKLib, appParam);
	if (datdlg==NULL) return;
	datdlg->DoModal();
	appParam = datdlg->getParameter();
	niDevice->setParameter(appParam);

	delete(datdlg);
	appParam.saveConfigFile();
}




// 音声認識
void CRinionsApp::OnSettingSpeech()
{
	// Backup
	BOOL speechReco = appParam.useSpeechReco;
	CString langSpeech = appParam.langSpeech;

	CSetSpeech* datdlg = new CSetSpeech(niDevice->nowSDKLib, appParam);
	if (datdlg==NULL) return;
	datdlg->DoModal();
	appParam = datdlg->getParameter();
	niDevice->setParameter(appParam);

	delete(datdlg);
	appParam.saveConfigFile();

	//
	if (speechReco!=appParam.useSpeechReco) {
		if (appParam.useSpeechReco) {
			StartSpeechRecognition();
		}
		else {
			StopSpeechRecognition();
		}
	}
	else if (langSpeech!=appParam.langSpeech) {	// 再起動
		if (appParam.useSpeechReco) {
			ReStartSpeechRecognition();
		}
	}
}




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Tools
//

void  CRinionsApp::updateMenuBar(CMenu* menu)
{
	if (menu==NULL) menu = pMainFrame->GetMenu();
	int state = niDevice->getDevState();


	// データ保存
	if (dataSaving || logfSaving || logfSending) {
		menu->EnableMenuItem(ID_SETTING_SAVEDATA, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}
	else {
		menu->EnableMenuItem(ID_SETTING_SAVEDATA, MF_BYCOMMAND | MF_ENABLED);
	}

	if (savedData) {
		menu->EnableMenuItem(ID_APP_DATA_SAVE, MF_BYCOMMAND | MF_ENABLED);
	}
	else {
		menu->EnableMenuItem(ID_APP_DATA_SAVE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}

	//
	if (state==NI_STATE_DETECT_STOPPED) {
		menu->EnableMenuItem(ID_SETTING_MOTION,    MF_BYCOMMAND | MF_ENABLED);		
		menu->EnableMenuItem(ID_SETTING_ANIMATION, MF_BYCOMMAND | MF_ENABLED);
	}
	else {
		menu->EnableMenuItem(ID_SETTING_MOTION,    MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		menu->EnableMenuItem(ID_SETTING_ANIMATION, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}

	// No NI Devce
	if (!niDevice->has_device) {
		if (niDevice->nowSDKLib==NiSDK_None) {
			menu->EnableMenuItem(ID_SETTING_VIEWMODE,  MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
			menu->EnableMenuItem(ID_SETTING_MOTION,    MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
			menu->EnableMenuItem(ID_SETTING_SAVEDATA,  MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
			menu->EnableMenuItem(ID_SETTING_DEVICE,    MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		}
		else if (niDevice->nowSDKLib==NiSDK_Kinect) {
			menu->EnableMenuItem(ID_SETTING_VIEWMODE,  MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
			menu->EnableMenuItem(ID_SETTING_MOTION,    MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
			menu->EnableMenuItem(ID_SETTING_SAVEDATA,  MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
			//menu->EnableMenuItem(ID_APP_DATA_SAVE,     MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		}
		enableSensorWndMenu(menu, FALSE);
	}

	//
	else {
		if (pSensorView!=NULL) {
			enableSensorWndMenu(menu, FALSE);
		}
		else {
			enableSensorWndMenu(menu, TRUE);
		}
	}
	
	// センサーウィンドウ
	if (pSensorView!=NULL) {
		enableLoadedWndMenu(menu, FALSE);
		menu->EnableMenuItem(ID_APP_DATA_LOAD, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		if (pDepthView==NULL) enableDepthWndMenu(menu, TRUE);
		else                  enableDepthWndMenu(menu, FALSE);
	}
	else {
		enableLoadedWndMenu(menu, loadedData);
		menu->EnableMenuItem(ID_APP_DATA_LOAD, MF_BYCOMMAND | MF_ENABLED);
		enableDepthWndMenu(menu, FALSE);
	}

	// ログウィンドウ
	if (pLogFrame!=NULL) {
		menu->EnableMenuItem(ID_LOG_WND_OPEN, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}
	else {
		menu->EnableMenuItem(ID_LOG_WND_OPEN, MF_BYCOMMAND | MF_ENABLED);
	}

	// ログイン
	if (isLoggedIn) {
		menu->EnableMenuItem(ID_APP_LOGIN,  MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		menu->EnableMenuItem(ID_APP_LOGOUT, MF_BYCOMMAND | MF_ENABLED);
		menu->EnableMenuItem(ID_APP_DATA_RECV, MF_BYCOMMAND | MF_ENABLED);
		if (isRecvData) menu->CheckMenuItem(ID_APP_DATA_RECV, MF_BYCOMMAND | MF_CHECKED);
		else            menu->CheckMenuItem(ID_APP_DATA_RECV, MF_BYCOMMAND | MF_UNCHECKED);
		menu->EnableMenuItem(ID_SETTING_ANIMATION, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
//		menu->EnableMenuItem(ID_SETTING_NETWORK, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}
	//
	else {
		if (state!=NI_STATE_DETECT_STOPPED && state!=NI_STATE_UNKNOWN) {
			menu->EnableMenuItem(ID_APP_LOGIN, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		}
		else {
			menu->EnableMenuItem(ID_APP_LOGIN, MF_BYCOMMAND | MF_ENABLED);
		}
		menu->EnableMenuItem(ID_APP_LOGOUT, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		menu->EnableMenuItem(ID_APP_DATA_RECV, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
//		menu->EnableMenuItem(ID_SETTING_ANIMATION, MF_BYCOMMAND | MF_ENABLED);
//		menu->EnableMenuItem(ID_SETTING_NETWORK, MF_BYCOMMAND | MF_ENABLED);
	}
}



//
void  CRinionsApp::enableSensorWndMenu(CMenu* menu, BOOL enable)
{
	CMenu* winmn = GetMenu_byStringID(menu, IDS_STR_MENU_WINDOW);

	if (enable) {
		if (winmn!=NULL) winmn->EnableMenuItem(0, MF_BYPOSITION | MF_ENABLED);
		menu->EnableMenuItem(ID_SNSR_WND_OPEN_1, MF_BYCOMMAND | MF_ENABLED);
		menu->EnableMenuItem(ID_SNSR_WND_OPEN_2, MF_BYCOMMAND | MF_ENABLED);
		menu->EnableMenuItem(ID_SNSR_WND_OPEN_3, MF_BYCOMMAND | MF_ENABLED);
		menu->EnableMenuItem(ID_SNSR_WND_OPEN_4, MF_BYCOMMAND | MF_ENABLED);
	}
	else {
		if (winmn!=NULL) winmn->EnableMenuItem(0, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
		menu->EnableMenuItem(ID_SNSR_WND_OPEN_1, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		menu->EnableMenuItem(ID_SNSR_WND_OPEN_2, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		menu->EnableMenuItem(ID_SNSR_WND_OPEN_3, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		menu->EnableMenuItem(ID_SNSR_WND_OPEN_4, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}
}



void  CRinionsApp::enableLoadedWndMenu(CMenu* menu, BOOL enable)
{
	CMenu* winmn = GetMenu_byStringID(menu, IDS_STR_MENU_WINDOW);

	if (enable) {
		if (winmn!=NULL) winmn->EnableMenuItem(1, MF_BYPOSITION | MF_ENABLED);
		menu->EnableMenuItem(ID_LOAD_WND_OPEN_1, MF_BYCOMMAND | MF_ENABLED);
		menu->EnableMenuItem(ID_LOAD_WND_OPEN_2, MF_BYCOMMAND | MF_ENABLED);
		menu->EnableMenuItem(ID_LOAD_WND_OPEN_3, MF_BYCOMMAND | MF_ENABLED);
		menu->EnableMenuItem(ID_LOAD_WND_OPEN_4, MF_BYCOMMAND | MF_ENABLED);
	}
	else {
		if (winmn!=NULL) winmn->EnableMenuItem(1, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
		menu->EnableMenuItem(ID_LOAD_WND_OPEN_1,   MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		menu->EnableMenuItem(ID_LOAD_WND_OPEN_2,   MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		menu->EnableMenuItem(ID_LOAD_WND_OPEN_3,   MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		menu->EnableMenuItem(ID_LOAD_WND_OPEN_4,   MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}
}



//
void  CRinionsApp::enableDepthWndMenu(CMenu* menu, BOOL enable)
{
	CMenu* winmn = GetMenu_byStringID(menu, IDS_STR_MENU_WINDOW);

	if (enable) {
		if (winmn!=NULL) winmn->EnableMenuItem(3, MF_BYPOSITION | MF_ENABLED);
		menu->EnableMenuItem(ID_DPTH_WND_OPEN_1, MF_BYCOMMAND | MF_ENABLED);
		menu->EnableMenuItem(ID_DPTH_WND_OPEN_2, MF_BYCOMMAND | MF_ENABLED);
		menu->EnableMenuItem(ID_DPTH_WND_OPEN_3, MF_BYCOMMAND | MF_ENABLED);
		menu->EnableMenuItem(ID_DPTH_WND_OPEN_4, MF_BYCOMMAND | MF_ENABLED);
	}
	else {
		if (winmn!=NULL) winmn->EnableMenuItem(3, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
		menu->EnableMenuItem(ID_DPTH_WND_OPEN_1, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		menu->EnableMenuItem(ID_DPTH_WND_OPEN_2, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		menu->EnableMenuItem(ID_DPTH_WND_OPEN_3, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		menu->EnableMenuItem(ID_DPTH_WND_OPEN_4, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}
}



void  CRinionsApp::updateStatusBar(BOOL init)
{
	if (pMainFrame==NULL) return;

	CString rc;
	CString mesg = _T(" ");

	if (init) {
		mesg = mesg + LoadString_byID(IDS_STR_STATUSBAR_INIT);
		pMainFrame->SetStausBarLocalFPS(0.0);
		pMainFrame->SetStausBarNetworkFPS(0.0);
		pMainFrame->SetStausBarNetworkBPS(0.0);
		//pMainFrame->SetStausBarSpeech(_T("N/A"));
	}
	//
	else {
		if (niDevice!=NULL && niDevice->nowSDKLib==NiSDK_OpenNI) {
			mesg = mesg + _T("OpenNI");
		}
		else if (niDevice!=NULL && niDevice->nowSDKLib==NiSDK_OpenNI2) {
			mesg = mesg + _T("OpenNI2");
		}
		else if (niDevice!=NULL && niDevice->nowSDKLib==NiSDK_Kinect) {
			mesg = mesg + _T("Kinect SDK");
		}
		else {	// NiSDK_None
			mesg = mesg + _T("No NI Library");
		}

		if (isLoggedIn) {
			rc.LoadString(IDS_STR_LOGGING_IN);					// ログイン中
			mesg = mesg + _T(" | " )+ rc;		
			if (isRecvData) {
				rc.LoadString(IDS_STR_RECEIVING_DATA);			// データ受信中
				mesg = mesg + _T(" | ") + rc;
			}
		}
		//
	}

	if (appParam.enableSpeechReco) {
		pMainFrame->SetStausBarSpeech(_T("Speech is Available"));
	}
	else {
		pMainFrame->SetStausBarSpeech(_T("N/A"));
	}

	pMainFrame->SetStausBarText(mesg);
}




//#include <imagehlp.h>
//#pragma comment(lib, "imagehlp.lib")

char*  CRinionsApp::makeLockFile(BOOL& exec)
{
	exec = TRUE;

	// Local
	CString path = MakeWorkingFolderPath(_T(RINIONS_LOCK_FILE), TRUE, _T(RINIONS_CONFIG_PATH), TRUE);

	if (file_exist_t(path)) {
		int ret = MessageBoxDLG(IDS_STR_CNFRM, IDS_STR_EXEC_MULTI, MB_YESNO, m_pMainWnd);
		if (ret==IDNO) {
			exec = FALSE;
			return NULL;
		}
	}
	else {
		FILE* fp = tfopen(path, _T("w"));
		if (fp!=NULL) fclose(fp);
	}

	char* pathstr = NULL;
	if (path!=_T("")) pathstr = ts2mbs(path);

	return pathstr;
}



void  CRinionsApp::deleteLockFile(void) 
{ 
	if (lockFilePath!=NULL) {
		unlink(lockFilePath);
		::free(lockFilePath);
		lockFilePath = NULL;
	}
	else {
		//DEBUG_INFO("CRinionsApp::deleteLockFile: ERROR: Lock File Path is NULL");
	}

	return;
}






///////////////////////////////////////////////////////////////////////////////////
//
afx_msg LRESULT CAboutDlg::OnSpeechEvent(WPARAM wParam, LPARAM lParam)
{
	if (wParam!=NULL && *(WORD*)wParam) {
		CWnd* wnd = GetForegroundWindow();	// GetActiveWindow(), GetLastActivePopup()
		if (wnd!=NULL) {
			if (wnd->m_hWnd==m_hWnd) {
				BOOL ret = DoSystemKeyAction((LPCTSTR)lParam, TRUE);
				if (!ret)  DoLocalTerminateAction((LPCTSTR)lParam);
			}
		}
	}

	return 0;
}




BOOL CAboutDlg::DoLocalTerminateAction(LPCTSTR com)
{
	if (!tcscmp(com, _T("OK"))) {
		OnOK();
	}
	else if (!tcscmp(com, _T("NO"))) {
		OnClose();
	}
	else if (!tcscmp(com, _T("CANCEL"))) {
		OnCancel();
	}
	else {
		return FALSE;
	}

	return TRUE;
}