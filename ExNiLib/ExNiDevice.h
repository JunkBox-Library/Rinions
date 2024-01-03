#pragma once


#include  <delayimp.h>
#pragma comment(lib, "delayimp.lib")

#include  "MainFrm.h"

#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
	#include  "ExNiLib/ExOpenNiWin.h"
#endif
#ifdef  ENABLE_KINECT_SDK
	#include  "ExNiLib/ExKinectWin.h"
#endif

#include  "MFCTool.h"




#define	   NIDEVICE_WAIT_TIME		200		// ms
#define	   NIDEVICE_TERM_WAIT_TIME	1000	// ms




////////////////////////////////////////////////
// ライブラリのラッパー



//using namespace jbxl;
using namespace jbxwl;




////////////////////////////////////////////////////////////////////////////////////
//
// CExNiDevice:　
//

class  CExNiDevice
{
public:
	CExNiDevice();
	virtual ~CExNiDevice();


private:
	//
	#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
	CExOpenNiWin*	openni_win;
	#else
	void*			openni_win;
	#endif

	#ifdef ENABLE_KINECT_SDK
	CExKinectWin*	kinect_sdk;
	#else
	void*			kinect_sdk;
	#endif

	CExNiFileDev*	nifile_dev;
	CExportNiData*	niexport_data;
	

public:
	CString			err_message;


public:
	BOOL			init(void);
	BOOL			check_lib(void);
	BOOL			init_device(void);
	BOOL			load_lib(LPCTSTR lib_name);
	//
	BOOL			initializeOpenNI2(void);
	BOOL			initializeNiTE2(void);
	void			shutdownOpenNI2(void);
	void			shutdownNiTE2(void);


public:
	NiFileType		checkFileType(LPCTSTR fname);
	NiFileType		rftype;
	void			execFilePlayer(BOOL login, CString fname);

	float			getLocalAvgFPS(void);
	void			clearLocalAvgFPS(void);

	int				execLoadedData(BOOL login,  FileDevParam ctrl);
	int				getFileFrameNum(void) { return nifile_dev->frames_num;}
	double			getFilePlayTime(void) { return nifile_dev->playback_time;}
	
	void			saveFramesDataAsBVH(CString fname, BVHSaveParam param);

public:
	NiSDK_Lib		nowSDKLib;

	CWnd*			mWnd;
	HWND			hWnd;
	BOOL			has_device;
	BOOL			loaded_oni;

	int				image_xsize;
	int				image_ysize;
	int				image_fps;

	//
	CMainFrame*		pMainFrame;
	CExFrame*		pSensorFrame;
	CExFrame*		pDepthFrame;
	CExTextFrame*	pLogFrame;
	CExFrame*		pModelFrame;

	CExNiNetwork*	niNetwork;
	CExNiSHMemory*	sharedMem;
	CExNiJoints*	niJoints;

	CParameterSet	appParam;
	FileDevParam	flpParam;
	BOOL			enableUSBDev;

	BOOL			enableFaceDetect;
	BOOL			enableSpeechReco;
	
	NiInputDevice	inputDevice;
	NiInputDevice	prvInputDevice;


public:
	// Interface to basic class	
	void	setParameter(CParameterSet param);
	void	setStatusBarMemFPS(float fps) { if (pMainFrame!=NULL) pMainFrame->SetStausBarLocalFPS(fps);}
	void	setStatusBarNetFPS(float fps) { if (pMainFrame!=NULL) pMainFrame->SetStausBarNetworkFPS(fps);}
	void	setStatusBarNetBPS(float bps) { if (pMainFrame!=NULL) pMainFrame->SetStausBarNetworkBPS(bps);}

	void	setStatusBarSpeech(LPCTSTR str) { if (pMainFrame!=NULL) pMainFrame->SetStausBarSpeech(str);}


	BOOL	initRingBuffer(void);
	CString	getErrorMessage(BOOL dev=FALSE);
	void	execMirroring(void);

	int		getDevState(void);
	void	setImageScale(int scale);
	void	setDepthScale(int scale);
	void	setSkeletonLine(int line);

	void	clearJointsData(void);
	void	clearAvatarDetected(void);

	void	clearLogDocPtr(void);
	void	setLogFramePtr(CExTextFrame*  ptr);
	void	setSensorFramePtr(CExFrame*   ptr);
	void	setDepthFramePtr(CExFrame*   ptr);
	void	setModelFramePtr(CExFrame* ptr);

	void	setNiJoints (CExNiJoints*   joints);
	void	setNetwork  (CExNiNetwork*  net);
	void	setSharedMem(CExNiSHMemory* mem);

	void	setNiFileDev(void);
	void	setViewPoint(void);

	void	deleteDevice(void);

	BOOL	createImageGenerator(void);
	BOOL	createDepthGenerator(void);
	void	deleteImageGenerator(void);
	void	deleteDepthGenerator(void);

	BOOL    hasImageGenerator(void);
	BOOL    hasDepthGenerator(void);

	void	setUseImage(BOOL use);
	void	setMirroring(BOOL use);

	void	setEnableFaceTracker(void);
	void	setUseFaceTracker(BOOL use);

	// Detection
	BOOL	startDetection(void);
	BOOL	stopDetection(void);
	void	setDenyTrackingSearch(void);

	// Speech
	BOOL	initSpeech(void);
	BOOL	createSpeech(void);
	BOOL	startSpeech(void);

	void	setSpeechConfidence(float confd);
	void	stopSpeech(void);
	void	deleteSpeech(BOOL rls=TRUE);

	// File
	BOOL	startRecorde(void);
	void	stopRecorde (void);
	int		backupRecordeTempFile(LPCTSTR backup_file, BOOL force);
	void    deleteRecordeTempFile(void);
	BOOL    openLoadFile(LPCTSTR fname);

	// Local Data Log
	BOOL	startLogRecorde(void);
	void	stopLogRecorde (void);

	// Network Data Log
	BOOL	startLogTransfer(void);
	void	stopLogTransfer (void);

	// USB
	BOOL	openUSBDevice(void);
	void	closeUSBDevice(void);
	void	setTiltMotor(int ang);
	void	setLEDColor (int col);

	void	setUseMotor(BOOL use);
	void	setUseLED(BOOL use);

	
	// Device
	BOOL	makeFileDevice(LPCTSTR fname);
	BOOL    restoreDevice(void);



public:
	friend  UINT  niDeviceEventLoop(LPVOID pParam);

};





//////////////////////////////////////////////////////////////////////
//

UINT  niDeviceEventLoop(LPVOID pParam);



