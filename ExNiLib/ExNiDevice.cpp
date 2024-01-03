
#include  "stdafx.h"

#include  "resource.h"
#include  "ExNiDevice.h"
#include  "MFCio.h"

#include  "NiDevice.h"
#include  "WinTools.h"
#include  "MessageBoxDLG.h"
#include  "Dialog/FilePlayerDLG.h"





/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CExNiDevice クラス : ライブラリのラッパークラス
//

CExNiDevice::CExNiDevice(void)
{
	nowSDKLib		 = NiSDK_None;
	//
	#ifdef ENABLE_OPENNI
	nowSDKLib		 = NiSDK_OpenNI;
	#endif
	#ifdef ENABLE_OPENNI2
	nowSDKLib		 = NiSDK_OpenNI2;
	#endif
	#ifdef ENABLE_KINECT_SDK
	nowSDKLib		 = NiSDK_Kinect;
	#endif

	openni_win		 = NULL;
	kinect_sdk		 = NULL;
	nifile_dev		 = NULL;
	niexport_data	 = NULL;

	pMainFrame		 = NULL;
	pSensorFrame	 = NULL;
	pDepthFrame		 = NULL;
	pLogFrame		 = NULL;
	pModelFrame		 = NULL;

	niNetwork		 = NULL;
	sharedMem		 = NULL;
	niJoints		 = NULL;

	mWnd			 = NULL;
	hWnd			 = NULL;
	enableUSBDev	 = FALSE;

	enableFaceDetect = FALSE;
	enableSpeechReco = FALSE;

	has_device		 = FALSE;
	loaded_oni		 = FALSE;

	inputDevice		 = NiDevice_NI;
	prvInputDevice	 = NiDevice_NI;
	err_message		 = _T("");

	rftype			 = FileDataUnknown;

	appParam.init();

	flpParam.repeat		 = TRUE;
	flpParam.mirroring   = TRUE;
	flpParam.calc_quat   = FALSE;
	flpParam.init_pos    = FALSE;
	flpParam.size_scale  = 1.0f;
	flpParam.time_scale  = 1.0f;
	//
	flpParam.start_frame = 0;
	flpParam.controler   = NULL;
	flpParam.counter     = NULL;

	//
	image_xsize	   = 0;
	image_ysize	   = 0;
	image_fps      = 0;
}



CExNiDevice::~CExNiDevice()
{
	DEBUG_INFO("DESTRUCTOR: CExNiDevice: START");

	deleteRecordeTempFile();

	if (pDepthFrame!=NULL) {
		CExFrame* pfrm = pDepthFrame;
		setDepthFramePtr(NULL);
		::Sleep(NIDEVICE_WAIT_TIME);
		delete(pfrm);
	}
	deleteNull(pSensorFrame);

	/*
	if (pSensorFrame!=NULL) {
		//CExFrame* pfrm = pSensorFrame;
		//setSensorFramePtr(NULL);
		//while (locked_event) {
		//	::Sleep(NIDEVICE_WAIT_TIME);
		//	DisPatcher();	
		//}
		//delete(pfrm);
		deleteNull(pSensorFrame);
	}*/

	//
	if (pLogFrame!=NULL) {
		if (niNetwork!=NULL) niNetwork->pLogDoc = NULL;
		if (pLogFrame->pDoc!=NULL) delete(pLogFrame->pDoc);
		delete(pLogFrame);
		pLogFrame = NULL;
	}

	//
	if (pModelFrame!=NULL) {
		delete(pModelFrame);
		pModelFrame = NULL;
	}

	//
	DEBUG_INFO("DESTRUCTOR: CExNiDevice: deleting nifile_dev ...");
	if (nifile_dev!=NULL) {
		delete(nifile_dev);
		nifile_dev = NULL;
	}

	DEBUG_INFO("DESTRUCTOR: CExNiDevice: deleting niexport_data ...");
	if (niexport_data!=NULL) {
		delete(niexport_data);
		niexport_data = NULL;
	}

	DEBUG_INFO("DESTRUCTOR: CExNiDevice: deleting openni_win ...");
	if (openni_win!=NULL) {
		delete(openni_win);
		openni_win = NULL;
	}
	
	DEBUG_INFO("DESTRUCTOR: CExNiDevice: deleting kinect_sdk ...");
	if (kinect_sdk!=NULL) {
		delete(kinect_sdk);
		kinect_sdk = NULL;
	}

	if (nowSDKLib==NiSDK_OpenNI2) {
		#ifdef ENABLE_OPENNI2
		DEBUG_INFO("DESTRUCTOR: CExNiDevice: shutdown OpenNI2 ...");
		shutdownOpenNI2();
		#endif
	}

	DEBUG_INFO("DESTRUCTOR: CExNiDevice: END");
}




BOOL  CExNiDevice::init(void)
{
	CString mesg = _T("");
	has_device = TRUE;

	BOOL ret = check_lib();
	if (!ret) {
		#ifdef WIN64
		mesg.LoadString(IDS_STR_NOT_LIB64);			// "64bitライブラリがインストールされていません"
		#else
		mesg.LoadString(IDS_STR_NOT_LIB32);			// "32bitライブラリがインストールされていません"
		#endif

		err_message = mesg;
		has_device = FALSE;
	}

	//
	else {
		ret = init_device();
		if (!ret) {
			mesg.LoadString(IDS_STR_NOT_DEVICE);	// "NIデバイスが接続されていません"
			err_message = mesg;
//			err_message = mesg + _T("\n\n") + getErrorMessage();
			has_device = FALSE;
		}
	}

	//
	nifile_dev	  = new CExNiFileDev(nowSDKLib);
	niexport_data = new CExportNiData();
	
	#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
	if (openni_win!=NULL) {
		openni_win->niExportData = niexport_data;
	}
	#endif
	
	#ifdef ENABLE_KINECT_SDK	
	if (kinect_sdk!=NULL) {
		kinect_sdk->niExportData = niexport_data;
	}
	#endif
	
	if (nifile_dev!=NULL) nifile_dev->niExportData = niexport_data;

	return has_device;
}




BOOL  CExNiDevice::check_lib(void)
{
	err_message = _T("");

	// first Kinect SDK
	if (nowSDKLib==NiSDK_Kinect) {
		if (load_lib(_T(KINECT_SDK_DLLNAME))) {
			DEBUG_INFO("CExNiDevice::check_lib(): Loaded KINECT SDK (1)");
			if (load_lib(_T(KINECT_FACE_DLLNAME))) {
				DEBUG_INFO("CExNiDevice::check_lib(): Loaded KINECT SDK FACE DETECTION");
				enableFaceDetect = TRUE;
			}
			else {
				enableFaceDetect = FALSE;
				if (appParam.useFaceDetect) {
					MessageBoxDLG(IDS_STR_WARN, IDS_STR_NOT_LOAD_FACE_DLL, MB_OK, mWnd);
				}
			}
			nowSDKLib = NiSDK_Kinect;
		}


#ifdef ENABLE_OPENNI2
		else if (load_lib(_T(OPENNI2_DLLNAME)) && load_lib(_T(NITE2_DLLNAME))) {
			DEBUG_INFO("CExNiDevice::check_lib(): Loaded OPENNI2 (1)");
#ifdef ENABLE_OPENCV
			enableFaceDetect = TRUE;
#else
			enableFaceDetect = FALSE;
#endif
			nowSDKLib = NiSDK_OpenNI2;
		}

#elif defined(ENABLE_OPENNI)
		//
		else if (load_lib(_T(OPENNI_NITE_DLLNAME))) {
			DEBUG_INFO("CExNiDevice::check_lib(): Loaded OPENNI (1)");
#ifdef ENABLE_OPENCV
			enableFaceDetect = TRUE;
#else
			enableFaceDetect = FALSE;
#endif			
			nowSDKLib = NiSDK_OpenNI;
		}
#endif
		//
		else {
			DEBUG_INFO("CExNiDevice::check_lib(): Fail to Load DLL (1)");
			nowSDKLib = NiSDK_None;
		}
	}


	//
	else {
#ifdef ENABLE_OPENNI2
		if (load_lib(_T(OPENNI2_DLLNAME)) && load_lib(_T(NITE2_DLLNAME))) {
			DEBUG_INFO("CExNiDevice::check_lib(): Loaded OPENNI2 (2)");
#ifdef ENABLE_OPENCV
			enableFaceDetect = TRUE;
#else
			enableFaceDetect = FALSE;
#endif
			nowSDKLib = NiSDK_OpenNI2;
		}
		else if (load_lib(_T(KINECT_SDK_DLLNAME))) {

#elif defined(ENABLE_OPENNI)
		//
		if (load_lib(_T(OPENNI_NITE_DLLNAME))) {
			DEBUG_INFO("CExNiDevice::check_lib(): Loaded OPENNI (2)");
#ifdef ENABLE_OPENCV
			enableFaceDetect = TRUE;
#else
			enableFaceDetect = FALSE;
#endif
			nowSDKLib = NiSDK_OpenNI;
		}
		else if (load_lib(_T(KINECT_SDK_DLLNAME))) {

#else 
		if (load_lib(_T(KINECT_SDK_DLLNAME))) {
#endif

			DEBUG_INFO("CExNiDevice::check_lib(): Loaded KINECT SDK (2)");
			if (load_lib(_T(KINECT_FACE_DLLNAME))) {
				DEBUG_INFO("CExNiDevice::check_lib(): Loaded KINECT SDK FACE DETECTION");
				enableFaceDetect = TRUE;
			}
			else {
				enableFaceDetect = FALSE;
				if (appParam.useFaceDetect) {
					MessageBoxDLG(IDS_STR_WARN, IDS_STR_NOT_LOAD_FACE_DLL, MB_OK, mWnd);
				}
			}
			nowSDKLib = NiSDK_Kinect;
		}

		//
		else {
			DEBUG_INFO("CExNiDevice::check_lib(): Fail to Load DLL (2)");
			nowSDKLib = NiSDK_None;
		}
	}

	//
	if (nowSDKLib==NiSDK_None) return FALSE;
	return TRUE;
}




//
// need isUseImage, isUseMotor, isUseLED
//
BOOL  CExNiDevice::init_device()
{
	err_message = _T("");

	BOOL ret = FALSE;

	if (nowSDKLib==NiSDK_OpenNI2) {
		#ifdef ENABLE_OPENNI2
		ret = initializeOpenNI2();
		//
		if (ret) {
			openni_win = new CExOpenNiWin(NiSDK_OpenNI2);
			if (openni_win!=NULL) {
				setUseImage(appParam.isUseImage);
				setEnableFaceTracker();
				setUseFaceTracker(appParam.useFaceDetect);
				ret = openni_win->init();
				if (ret) {
					image_xsize = openni_win->getXSize();
					image_ysize = openni_win->getYSize();
					image_fps   = openni_win->getFPS();
				}
				else {
					openni_win->deleteDevice();
				}
				NiSetOpenNI2JointNums();
			}
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_OpenNI) {
		#ifdef ENABLE_OPENNI
		openni_win = new CExOpenNiWin(NiSDK_OpenNI);
		if (openni_win!=NULL) {
			setUseImage(appParam.isUseImage);
			setEnableFaceTracker();
			setUseFaceTracker(appParam.useFaceDetect);
			ret = openni_win->init();
			if (ret) {
				image_xsize = openni_win->getXSize();
				image_ysize = openni_win->getYSize();
				image_fps   = openni_win->getFPS();
			}
			else {
				openni_win->deleteDevice();
			}
			NiSetOpenNIJointNums();
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		kinect_sdk = new CExKinectWin();
		if (kinect_sdk!=NULL) {
			setUseImage(appParam.isUseImage);
			setEnableFaceTracker();
			setUseFaceTracker(appParam.useFaceDetect);
			ret = kinect_sdk->init();
			if (ret) {
				image_xsize = kinect_sdk->getXSize();
				image_ysize = kinect_sdk->getYSize();
				image_fps   = kinect_sdk->getFPS();
			}
			else {
				kinect_sdk->deleteDevice();
			}
			NiSetKinectJointNums();
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::init_device(): ERROR: Unknown Library Type.");
	}

	if (ret) {
		openUSBDevice();
		setUseLED(appParam.useDevLED);
		setUseMotor(appParam.useDevMotor);
		//setTiltMotor(0);
		setLEDColor(NI_LED_GREEN);
	}

	//
	initRBoundJointsRotation();

	return ret;
}




BOOL  CExNiDevice::initializeOpenNI2(void)
{
	err_message = _T("");

	BOOL ret = FALSE;

#ifdef ENABLE_OPENNI2
	SetCurrentDirectory((LPCTSTR)GetProgramFolder());
	Buffer mesg = jbxl::initialize_OpenNI2();
	
	if (mesg.buf==NULL) {
		ret = TRUE;
	}
	else {
		err_message = mbs2ts((char*)mesg.buf);
		free_Buffer(&mesg);
	}
#endif

	return ret;
}



void  CExNiDevice::shutdownOpenNI2(void)
{ 
#ifdef ENABLE_OPENNI2
	jbxl::shutdown_OpenNI2();
#endif
}




BOOL  CExNiDevice::initializeNiTE2(void)
{
	err_message = _T("");

	BOOL ret = FALSE;

#ifdef ENABLE_OPENNI2
	SetCurrentDirectory((LPCTSTR)GetProgramFolder());
	Buffer mesg = jbxl::initialize_NiTE2();
	
	if (mesg.buf==NULL) {
		ret = TRUE;
	}
	else {
		err_message = mbs2ts((char*)mesg.buf);
		free_Buffer(&mesg);
	}
#endif

	return ret;
}





void  CExNiDevice::shutdownNiTE2(void)
{ 
#ifdef ENABLE_OPENNI2
	jbxl::shutdown_NiTE2();
#endif
}





BOOL  CExNiDevice::load_lib(LPCTSTR lib_name)
{
	if (LoadLibrary(lib_name)==NULL) return FALSE;

	char* mbstr = ts2mbs(lib_name);
	HRESULT ret = __HrLoadAllImportsForDll(mbstr);
	::free(mbstr);

	if (FAILED(ret)) return FALSE;
	return TRUE;
}





//////////////////////////////////////////////////////////////////////////////////////////////////////////
//

NiFileType  CExNiDevice::checkFileType(LPCTSTR fname)
{
	char buf[LSNAME];		// 32
	memset(buf, 0, LSNAME);

	FILE* fp = tfopen(fname, _T("rb"));
	if (fp==NULL) return FileDataUnknown;
	fread(buf, LSNAME, 1, fp);
	fclose(fp);

	if      (!strncasecmp(buf, BVH_STR_HIERARCHY, strlen(BVH_STR_HIERARCHY))) {
		return FileDataBVH;
	}
	else if (!strncasecmp(buf, NI_JTXT_FILE_ID, strlen(NI_JTXT_FILE_ID))) {
		return FileDataJTXT;
	}
	else if (!strncasecmp(buf, VMD_FILE_HD_ID2, strlen(VMD_FILE_HD_ID2))) {
		return FileDataVMD2;
	}
	return FileDataONI;
}



void  CExNiDevice::execFilePlayer(BOOL login, CString fname)
{
	CFilePlayerDLG* player = new CFilePlayerDLG(this, login, flpParam, fname);
	if (player!=NULL) {
		player->DoModal();
		flpParam = player->getParameter();
		delete(player);
	}

	sharedMem->isTracking = FALSE;
	sharedMem->clearLocalAnimationData();
	nifile_dev->clear_data();
}



int  CExNiDevice::execLoadedData(BOOL login, FileDevParam param)
{
	if (login) niNetwork->openSendSocket();

	// 戻り値：　正: 最後のフレームの次のフレームの番号, 負: エラー
	int ret = nifile_dev->convertJointsData(niNetwork, param);
	if (ret<0) {
		*(param.controler) = NI_FILE_PLAYER_STOP;
		err_message = LoadString_byID(IDS_STR_ERR_NEXT_FRAME);
	}

	niNetwork->closeSendSocket();
	return ret;
}




float  CExNiDevice::getLocalAvgFPS(void)
{
	if (niexport_data!=NULL) return niexport_data->frameRate;
	else return 0.0f;
}



void  CExNiDevice::clearLocalAvgFPS(void)
{
	if (niexport_data!=NULL) niexport_data->clear();
	SendWinMessage(NISHM_WM_LCLFPS_UPDATE);
}




void  CExNiDevice::saveFramesDataAsBVH(CString fname, BVHSaveParam param)
{
	if (!fname.IsEmpty()) {
		CMessageBoxDLG* mbox = MessageBoxDLG(IDS_STR_INFO, IDS_STR_WRTNG_FILE, mWnd);
		nifile_dev->writeBVHFile(fname, param);
		if (mbox!=NULL) delete(mbox);
	}
}








//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface 
//

void  CExNiDevice::setParameter(CParameterSet param)
{	
	err_message = _T("");

	appParam = param;
	enableSpeechReco = param.enableSpeechReco;

	if (niNetwork!=NULL)  niNetwork ->setParameter(param);
	if (sharedMem!=NULL)  sharedMem ->setParameter(param);
	if (niJoints!=NULL)   niJoints  ->setParameter(param);
	if (nifile_dev!=NULL) nifile_dev->setParameter(param);


	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			openni_win->setParameter(param);
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			kinect_sdk->setParameter(param);
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::setParameter(): ERROR: Unknown Library Type.");
	}

	//
	setUseImage(param.isUseImage);
	setUseFaceTracker(appParam.useFaceDetect);
	setMirroring(param.isMirroring);
	setUseMotor(param.useDevMotor);
	setUseLED(param.useDevLED);
	setSpeechConfidence(param.confdSpeech);
	setSkeletonLine(param.lineSkeleton);


	if (niexport_data!=NULL) {
		if (param.outDataFPS>1) {
			int period = Max(1000/(param.outDataFPS-1), 25);	// 最小 25ms, 最大 40FPS
			if (period!=niexport_data->outPeriod) {
				niexport_data->modTime = 0;
				niexport_data->outPeriod = period;
			}
		}
		niexport_data->ctrlFPS = param.outCtrlFPS;
	}
	
	return;
}



BOOL  CExNiDevice::initRingBuffer(void)
{	
	err_message = _T("");

	BOOL ret = TRUE;

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			ret = openni_win->initRingBuffer();
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			ret = kinect_sdk->initRingBuffer();
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::initRingBuffer(): ERROR: Unknown Library Type.");
		ret = FALSE;
	}

	if (!ret && err_message.IsEmpty()) getErrorMessage(TRUE);

	return ret;
}



/*
void  CExNiDevice::execMirroring(void)
{
	err_message = _T("");

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			if (openni_win->device!=NULL) {
				if (openni_win->device->context!=NULL) {
					openni_win->device->context->SetGlobalMirror(appParam.isMirroring);
					::Sleep(NIDEVICE_WAIT_TIME);
				}
			}
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			kinect_sdk->m_mirroring = appParam.isMirroring;
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::execMirroring(): ERROR: Unknown Library Type.");
	}

	return;
}
*/




//
// dev==TRUE ならデバイスからエラーメッセージを取得する．
//
CString  CExNiDevice::getErrorMessage(BOOL dev)
{
	if (dev || err_message.IsEmpty()) {
		//
		if (nowSDKLib==NiSDK_OpenNI2) {
			#ifdef ENABLE_OPENNI2
			if (openni_win!=NULL) {
				err_message = openni_win->get_err_message();
			}
			else {
				err_message = _T("OpenNI2 device is NULL!!");
			}
			#else
			err_message = _T("OpenNI2 is disabled!!");
			#endif
		}
		//
		else if (nowSDKLib==NiSDK_OpenNI) {
			#ifdef ENABLE_OPENNI
			if (openni_win!=NULL) {
				err_message = openni_win->get_err_message();
			}
			else {
				err_message = _T("OpenNI device is NULL!!");
			}
			#else
			err_message = _T("OpenNI is disabled!!");
			#endif
		}
		//
		else if (nowSDKLib==NiSDK_Kinect) {
			#ifdef ENABLE_KINECT_SDK
			if (kinect_sdk!=NULL) {
				err_message = kinect_sdk->get_err_message();
			}
			else {
				err_message = _T("Kinect SDK device is NULL!!");
			}
			#else
			err_message = _T("Kinect SDK is disabled!!");
			#endif
		}
		//
		else {
			err_message = _T("CExNiDevice::getErrorMessage(): ERROR: Unknown Library Type.");
		}
	}
	
	if (err_message.IsEmpty()) err_message = _T("CExNiDevice::getErrorMessage(): Unknown Error Occurred.");

	return err_message;
}




int   CExNiDevice::getDevState(void)
{
	err_message = _T("");

	int st = NI_STATE_DETECT_STOPPED;

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			st = openni_win->getDevState();
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			st = kinect_sdk->getDevState();
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::getDevState(): ERROR: Unknown Library Type.");
	}

	return st;
}




void  CExNiDevice::setImageScale(int scale)
{
	err_message = _T("");

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			openni_win->m_image_scale = scale;
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			kinect_sdk->m_image_scale = scale;
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::setImageScale(): ERROR: Unknown Library Type.");
	}

	return;
}



void  CExNiDevice::setDepthScale(int scale)
{
	err_message = _T("");

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			openni_win->m_depth_scale = scale;
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			kinect_sdk->m_depth_scale = scale;
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::setDepthScale(): ERROR: Unknown Library Type.");
	}

	return;
}



void  CExNiDevice::setSkeletonLine(int line)
{
	err_message = _T("");

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			openni_win->m_skeleton_line = line;
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			kinect_sdk->m_skeleton_line = line;
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::setSkeletonLine(): ERROR: Unknown Library Type.");
	}

	return;
}






//////////////////////////////////////////////////////////////////////////////////////////

void  CExNiDevice::clearJointsData(void)
{
	err_message = _T("");

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			openni_win->clearJointsData();
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			kinect_sdk->clearJointsData();
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::clearJointsData(): ERROR: Unknown Library Type.");
	}

	return;
}



void  CExNiDevice::clearAvatarDetected(void)
{
	err_message = _T("");

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			openni_win->clearAvatarDetected();
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			kinect_sdk->clearAvatarDetected();
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::clearAvatarDetected(): ERROR: Unknown Library Type.");
	}

	return;
}




//////////////////////////////////////////////////////////////////////////////////////////

void  CExNiDevice::clearLogDocPtr(void)
{
	err_message = _T("");

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			openni_win->pLogDoc = NULL;
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			kinect_sdk->pLogDoc = NULL;
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::clearLogDocPtr(): ERROR: Unknown Library Type.");
	}

	return;
}



void  CExNiDevice::setLogFramePtr(CExTextFrame* ptr)
{
	err_message = _T("");

	pLogFrame = ptr;

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			openni_win->setLogFramePtr(ptr);
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			kinect_sdk->setLogFramePtr(ptr);
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::setLogFramePtr(): ERROR: Unknown Library Type.");
	}

	return;
}



void  CExNiDevice::setSensorFramePtr(CExFrame* ptr)
{
	err_message = _T("");

	pSensorFrame = ptr;

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			openni_win->setSensorFramePtr(ptr);
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			kinect_sdk->setSensorFramePtr(ptr);
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::setSensorFramePtr(): ERROR: Unknown Library Type.");
	}

	return;
}




void  CExNiDevice::setDepthFramePtr(CExFrame* ptr)
{
	err_message = _T("");

	pDepthFrame = ptr;

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			openni_win->setDepthFramePtr(ptr);
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			kinect_sdk->setDepthFramePtr(ptr);
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::setDepthFramePtr(): ERROR: Unknown Library Type.");
	}

	return;
}




void  CExNiDevice::setModelFramePtr(CExFrame* ptr)
{
	err_message = _T("");

	pModelFrame = ptr;

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			openni_win->setModelFramePtr(ptr);
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			kinect_sdk->setModelFramePtr(ptr);
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::setModelFramePtr(): ERROR: Unknown Library Type.");
	}

	return;
}




//////////////////////////////////////////////////////////////////////////////////////////

void  CExNiDevice::setViewPoint(void)
{ 
	err_message = _T("");

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			openni_win->setViewPoint();
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			kinect_sdk->nop();
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::setViewPoint(): ERROR: Unknown Library Type.");
	}

	return;

}



void  CExNiDevice::setNiJoints(CExNiJoints* joints)
{
	err_message = _T("");

	if (joints==NULL) return;
	niJoints = joints;

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			openni_win->niJoints = joints;
			joints->connectJointsData(openni_win->posVect, openni_win->rotQuat, openni_win->crdVect, 
									  openni_win->jntAngl, openni_win->posRing, openni_win->rotRing);
			joints->pViewData = (ExCmnHead**)&(openni_win->pViewData);
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			kinect_sdk->niJoints = joints;
			joints->connectJointsData(kinect_sdk->posVect, kinect_sdk->rotQuat, kinect_sdk->crdVect, 
									  kinect_sdk->jntAngl, kinect_sdk->posRing, kinect_sdk->rotRing);
			joints->pViewData = (ExCmnHead**)&(kinect_sdk->pViewData);
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::setNiJoints(): ERROR: Unknown Library Type.");
	}

	return;
}



void  CExNiDevice::setNetwork(CExNiNetwork* net)
{
	err_message = _T("");

	niNetwork = net;

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			openni_win->niNetwork = net;
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			kinect_sdk->niNetwork = net;
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::setNetwork(): ERROR: Unknown Library Type.");
	}

	return;

}



void  CExNiDevice::setSharedMem(CExNiSHMemory* mem)
{
	err_message = _T("");

	sharedMem = mem;

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			openni_win->sharedMem = mem;
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			kinect_sdk->sharedMem = mem;
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::setSharedMem(): ERROR: Unknown Library Type.");
	}

	return;
}



void  CExNiDevice::setNiFileDev(void)
{
	err_message = _T("");

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			openni_win->niFileDev = nifile_dev;
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			kinect_sdk->niFileDev = nifile_dev;
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::setNiFileDev(): ERROR: Unknown Library Type.");
	}

	return;
}





//////////////////////////////////////////////////////////////////////////////////////////

void  CExNiDevice::deleteDevice(void)
{
	err_message = _T("");

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			openni_win->deleteDevice();
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			kinect_sdk->deleteDevice();
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::deleteDevice(): ERROR: Unknown Library Type.");
	}

	return;
}




BOOL  CExNiDevice::createImageGenerator(void)
{
	err_message = _T("");

	BOOL ret = FALSE;

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			ret = openni_win->createImage();
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			ret = kinect_sdk->createImage();
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::createImage(): ERROR: Unknown Library Type.");
	}

	return ret;
}




BOOL  CExNiDevice::createDepthGenerator(void)
{
	err_message = _T("");

	BOOL ret = FALSE;

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			ret = openni_win->createDepth();
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			ret = kinect_sdk->createDepth();
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::createDepth(): ERROR: Unknown Library Type.");
	}

	return ret;
}




void  CExNiDevice::deleteImageGenerator(void)
{
	err_message = _T("");

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			openni_win->deleteImage();
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			kinect_sdk->deleteImage();
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::deleteImageGenerator(): ERROR: Unknown Library Type.");
	}

	return;
}




void  CExNiDevice::deleteDepthGenerator(void)
{
	err_message = _T("");

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			openni_win->deleteDepth();
		}	
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			kinect_sdk->deleteDepth();
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::deleteDepthGenerator(): ERROR: Unknown Library Type.");
	}

	return;
}




BOOL  CExNiDevice::hasImageGenerator(void)
{
	err_message = _T("");

	BOOL ret = FALSE;

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			if (openni_win->hasImageGen()) ret = TRUE;
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			if (kinect_sdk->hasImageGen()) ret = TRUE;
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::hasImageGenerator(): ERROR: Unknown Library Type.");
	}

	return ret;
}



BOOL  CExNiDevice::hasDepthGenerator(void)
{
	err_message = _T("");

	BOOL ret = FALSE;

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			if (openni_win->hasDepthGen()) ret = TRUE;
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			if (kinect_sdk->hasDepthGen()) ret = TRUE;
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::hasDepthGenerator(): ERROR: Unknown Library Type.");
	}

	return ret;
}





//////////////////////////////////////////////////////////////////////////////////////////

void  CExNiDevice::setUseImage(BOOL use)
{
	err_message = _T("");

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			openni_win->m_use_image = use;
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			kinect_sdk->m_use_image = use;
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::setUseImage(): ERROR: Unknown Library Type.");
	}

	return;
}



void  CExNiDevice::setEnableFaceTracker(void)
{
	err_message = _T("");

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			openni_win->m_enable_face = enableFaceDetect;
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			kinect_sdk->m_enable_face = enableFaceDetect;
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::setEnableFaceTracker(): ERROR: Unknown Library Type.");
	}

	return;
}




void  CExNiDevice::setUseFaceTracker(BOOL use)
{
	err_message = _T("");

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			if (openni_win->m_enable_face) openni_win->m_use_face = use;
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			if (kinect_sdk->m_enable_face) kinect_sdk->m_use_face = use;
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::setUseFaceTracker(): ERROR: Unknown Library Type.");
	}

	return;
}



void  CExNiDevice::setMirroring(BOOL use)
{
	err_message = _T("");

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			openni_win->setMirroring(use);
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			kinect_sdk->setMirroring(use);
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::setMirroring(): ERROR: Unknown Library Type.");
	}

	return;
}




//////////////////////////////////////////////////////////////////////////////////////////

BOOL  CExNiDevice::startDetection()
{
	err_message = _T("");

	BOOL ret = FALSE;
	clearLocalAvgFPS();

	if (nowSDKLib==NiSDK_OpenNI2) {
		#ifdef ENABLE_OPENNI2
		if (openni_win!=NULL) {
			ret = initializeNiTE2();
			if (ret) ret = openni_win->startDetection();
			else DEBUG_ERR("CExNiDevice::startDetection(): ERROR: Start Detection Error.");
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_OpenNI) {
		#ifdef ENABLE_OPENNI
		if (openni_win!=NULL) {
			ret = openni_win->startDetection();
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			ret = kinect_sdk->startDetection();
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::startDetection(): ERROR: Unknown Library Type.");
	}

	return ret;
}



BOOL  CExNiDevice::stopDetection()
{
	err_message = _T("");

	BOOL ret = FALSE;
	clearLocalAvgFPS();

	if (nowSDKLib==NiSDK_OpenNI2) {
		#ifdef ENABLE_OPENNI2
		if (openni_win!=NULL) {
			ret = openni_win->stopDetection();
			shutdownNiTE2();
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_OpenNI) {
		#ifdef ENABLE_OPENNI
		if (openni_win!=NULL) {
			ret = openni_win->stopDetection();
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			ret = kinect_sdk->stopDetection();
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::stopDetection(): ERROR: Unknown Library Type.");
	}

	return ret;
}



void  CExNiDevice::setDenyTrackingSearch(void)
{
	err_message = _T("");

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			openni_win->setDenyTrackingSearch(openni_win->tracking_user);
			openni_win->tracking_user = 0;
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			kinect_sdk->setDenyTrackingSearch(kinect_sdk->tracking_user);
			kinect_sdk->tracking_user = 0;
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::setDenyTrackingSearch(): ERROR: Unknown Library Type.");
	}

	return;
}




//////////////////////////////////////////////////////////////////////////////////////////
//
// Speech Platorm
//
BOOL  CExNiDevice::initSpeech(void)
{
	DEBUG_INFO("CExNiDevice::initSpeech(): initializing Speech");
	//
	err_message = _T("");

	BOOL ret = FALSE;

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			if (openni_win!=NULL) {
				//ret = openni_win->initSpeech();
				ret = FALSE;
				err_message = _T("CExNiDevice::initSpeech(): WARNING: Speech Platform can not be used with OpenNI/OpenNI2.");
			}
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			#ifdef ENABLE_NI_SPEECH
			ret = kinect_sdk->initSpeech();
			#endif
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::initSpeech(): ERROR: Unknown Library Type.");
	}

	enableSpeechReco = FALSE;

	DEBUG_INFO("CExNiDevice::initSpeech(): initialized  Speech");
	return ret;
}





BOOL  CExNiDevice::createSpeech(void)
{
	DEBUG_INFO("CExNiDevice::createSpeech(): creating Speech");
	//
	err_message = _T("");

	BOOL ret = FALSE;

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			err_message = _T("CExNiDevice::createSpeech WARNING: Speech Platform can not be used with OpenNI/OpenNI2.");
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			//
			CString path   = _T(RINIONS_SPEECH_GRAMMAR);
			CString locale = getResourceLocale();

			if (!locale.Compare(_T(JBXWL_LOCALE_JP))) {
				path += _T(".jp.grxml");
			}
			else {
				path += _T(".grxml");
			}
			if (!file_exist_t(path)) path = GetProgramFolder() + path;

			//
			if (!locale.Compare(_T(JBXWL_LOCALE_JP))) {	
				#ifdef ENABLE_NI_SPEECH
				ret = kinect_sdk->createSpeech(_T(NI_SPEECH_LANG_JP_KINECT), (LPCTSTR)path);
				#endif
			}
			else {
				#ifdef ENABLE_NI_SPEECH
				ret = kinect_sdk->createSpeech(_T(NI_SPEECH_LANG_US_KINECT), (LPCTSTR)path);
				#endif
			}
			
			//if (ret) enableSpeechReco = TRUE;
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::createSpeech(): ERROR: Unknown Library Type.");
	}

	enableSpeechReco = FALSE;

	DEBUG_INFO("CExNiDevice::createSpeech(): created  Speech");
	return ret;
}





BOOL  CExNiDevice::startSpeech(void)
{
	DEBUG_INFO("CExNiDevice::startSpeech(): starting Speech");
	//
	err_message = _T("");

	BOOL ret = FALSE;
	enableSpeechReco = FALSE;

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			err_message = _T("CExNiDevice::startSpeech(): WARNING: Speech Platform can not be used with OpenNI/OpenNI2.");
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			#ifdef ENABLE_NI_SPEECH
			ret = kinect_sdk->startSpeech(appParam.confdSpeech);
			#endif
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::startSpeech(): ERROR: Unknown Library Type.");
	}

	if (ret) enableSpeechReco = TRUE;

	DEBUG_INFO("CExNiDevice::startSpeech(): started  Speech");
	return ret;
}




void  CExNiDevice::stopSpeech(void)
{
	DEBUG_INFO("CExNiDevice::stopSpeech(): stopping Speech");
	//
	err_message = _T("");

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			err_message = _T("CExNiDevice::stopSpeech(): WARNING: Speech Platform can not be used with OpenNI/OpenNI2.");
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			#ifdef ENABLE_NI_SPEECH
			kinect_sdk->stopSpeech();
			#endif
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::stopSpeech(): ERROR: Unknown Library Type.");
	}

	enableSpeechReco = FALSE;

	DEBUG_INFO("CExNiDevice::stopSpeech(): stopped  Speech");
	return;
}




void  CExNiDevice::deleteSpeech(BOOL flg)
{
	DEBUG_INFO("CExNiDevice::deleteSpeech(): deleting Speech");
	//
	err_message = _T("");

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			err_message = _T("CExNiDevice::deleteSpeech(): WARNING: Speech Platform can not be used with OpenNI/OpenNI2.");
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			#ifdef ENABLE_NI_SPEECH
			kinect_sdk->deleteSpeech(flg);
			#endif
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::deleteSpeech(): ERROR: Unknown Library Type.");
	}

	enableSpeechReco = FALSE;

	DEBUG_INFO("CExNiDevice::deleteSpeech(): deleted  Speech");
	return;
}




void  CExNiDevice::setSpeechConfidence(float confd)
{
	err_message = _T("");

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			err_message = _T("CExNiDevice::setSpeechConfidence(): WARNING: Speech Platform can not be used with OpenNI/OpenNI2.");
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			#ifdef ENABLE_NI_SPEECH
			kinect_sdk->setSpeechConfidence(confd);
			#endif
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::setSpeechConfidence(): ERROR: Unknown Library Type.");
	}

	return;
}





//////////////////////////////////////////////////////////////////////////////////////////
//
// File
//

BOOL  CExNiDevice::startRecorde(void)
{
	err_message = _T("");

	BOOL ret = FALSE;

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			if (appParam.saveDataMode==FileDataONI) {
				char* mbstr = ts2mbs(openni_win->tempFilePath);
				ret = openni_win->startRecorde(mbstr, appParam.isUseImage);
				::free(mbstr);
				openni_win->dataSaving = ret;
			}
			else if (appParam.saveDataMode==FileDataBVH || appParam.saveDataMode==FileDataJTXT) {
				if (nifile_dev!=NULL) {
					ret = nifile_dev->openTempJointsFile();
					openni_win->dataSaving = ret;
					if (ret) nifile_dev->writeTempJointsFileHeader();
				}
				else {
					err_message = _T("CExNiDevice::startRecorde(): ERROR: NiFile Class is NULL.");
				}
			}
			else {
				err_message = _T("CExNiDevice::startRecorde(): ERROR: Unknown Recorde Format!!");
			}
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			if (appParam.saveDataMode==FileDataONI) {
				err_message = _T("CExNiDevice::startRecorde(): ERROR: Can not treat ONI file with Kinect SDK Lib");
			}
			else if (appParam.saveDataMode==FileDataBVH || appParam.saveDataMode==FileDataJTXT) {
				if (nifile_dev!=NULL) {
					ret = nifile_dev->openTempJointsFile();
					kinect_sdk->dataSaving = ret;
					if (ret) nifile_dev->writeTempJointsFileHeader();
				}
				else {
					err_message = _T("CExNiDevice::startRecorde(): ERROR: NiFile Class is NULL.");
				}
			}
			else {
				err_message = _T("CExNiDevice::startRecorde(): ERROR: Unknown Recorde Format!!");
			}
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::startRecorde(): ERROR: Unknown Library Type.");
	}

	return ret;
}




void  CExNiDevice::stopRecorde(void)
{
	err_message = _T("");

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			if (appParam.saveDataMode==FileDataONI) {
				openni_win->dataSaving = FALSE;
				openni_win->stopRecorde();
			}
			else if (appParam.saveDataMode==FileDataBVH || appParam.saveDataMode==FileDataJTXT) {
				if (nifile_dev!=NULL) {
					openni_win->dataSaving = FALSE;
					::Sleep(NIDEVICE_WAIT_TIME);
					nifile_dev->closeTempJointsFile();
				}
			}
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			if (appParam.saveDataMode==FileDataBVH || appParam.saveDataMode==FileDataJTXT) {
				if (nifile_dev!=NULL) {
					kinect_sdk->dataSaving = FALSE;
					::Sleep(NIDEVICE_WAIT_TIME);
					nifile_dev->closeTempJointsFile();
				}
			}
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::stopRecorde(): ERROR: Unknown Library Type.");
	}

	return;
}



int   CExNiDevice::backupRecordeTempFile(LPCTSTR backup_file, BOOL force)
{
	err_message = _T("");

	int  ret = 0;

	////////////////////////////////////////////////////////////////
	// Original Format
	if (appParam.saveDataMode==FileDataJTXT) {
		if (nifile_dev!=NULL) {
			CMessageBoxDLG* mbox = MessageBoxDLG(IDS_STR_INFO, IDS_STR_WRTNG_FILE, mWnd);
			ret = nifile_dev->saveAsJText(backup_file, force);
			if (mbox!=NULL) delete(mbox);
		}
	}
	

	////////////////////////////////////////////////////////////////
	// BVH
	else if (appParam.saveDataMode==FileDataBVH) {		
		if (nifile_dev!=NULL) {
			BVHSaveParam param;
			param.format = appParam.saveBVHFormat;
			param.fps    = appParam.saveBVHFPS;
			param.divtm	 = appParam.saveDivTime;
			param.scale  = appParam.saveSzScale;
			param.recalc = FALSE;
			//
			CMessageBoxDLG* mbox = MessageBoxDLG(IDS_STR_INFO, IDS_STR_WRTNG_FILE, mWnd);
			ret = nifile_dev->saveAsBVH(backup_file, param);
			if (mbox!=NULL) delete(mbox);
		}
	}


	////////////////////////////////////////////////////////////////
	// ONI
	else if (appParam.saveDataMode==FileDataONI) {
		if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
			#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
			if (openni_win!=NULL) {
				if (!force) {
					if (file_exist_t(backup_file)) {
						int yesno = MessageBoxDLG(IDS_STR_WARN, IDS_STR_ASK_OVERWRITE, MB_YESNO, mWnd);
						if (yesno==IDNO) return TRUE;	// エラーメッセージを出さない
					}
				}
				CMessageBoxDLG* mbox = MessageBoxDLG(IDS_STR_INFO, IDS_STR_WRTNG_FILE, mWnd);
				ret = copyFileWithCounter((LPCTSTR)openni_win->tempFilePath, backup_file);
				if (mbox!=NULL) delete(mbox);
			}
			#endif
		}
		//
		else if (nowSDKLib==NiSDK_Kinect) {
			#ifdef ENABLE_KINECT_SDK
			if (kinect_sdk!=NULL) {
				// NOP
			}
			#endif
		}
		//
		else {
			err_message = _T("CExNiDevice::backupRecordeTempFile(): ERROR: ONI FILE: Unknown Library Type.");
		}
	}

	return ret;
}



void   CExNiDevice::deleteRecordeTempFile()
{
	err_message = _T("");

	////////////////////////////////////////////////////////////////
	// Original or BVH
	if (appParam.saveDataMode==FileDataBVH || appParam.saveDataMode==FileDataJTXT) {
		if (nifile_dev!=NULL) {
			nifile_dev->deleteRecordeTempFile();
		}
	}
	

	////////////////////////////////////////////////////////////////
	// ONI
	else if (appParam.saveDataMode==FileDataONI) {
		if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
			#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
			if (openni_win!=NULL) {
				openni_win->deleteRecordeTempFile();
			}
			#endif
		}
		//
		else if (nowSDKLib==NiSDK_Kinect) {
			#ifdef ENABLE_KINECT_SDK
			if (kinect_sdk!=NULL) {
				// NOP
			}
			#endif
		}
		//
		else {
			err_message = _T("CExNiDevice::deleteRecordeTempFile(): ERROR: ONI FILE: Unknown Library Type.");
		}
	}

	return;
}




BOOL  CExNiDevice::openLoadFile(LPCTSTR fname)
{
	err_message = _T("");

	BOOL ret   = FALSE;
	loaded_oni = FALSE;

	NiFileType fd = checkFileType(fname);

	////////////////////////////////////////////////////////////////
	// BVH
	if (fd==FileDataBVH) {
		int num = 0;
		if (nifile_dev!=NULL) {
			num = nifile_dev->readBVHFile(fname);
		}

		//if (num<=0) DEBUG_INFO("read BVH File Error (%d)", num);
		if (num>0) ret = TRUE;
		else err_message = _T("CExNiDevice::openLoadFile(): ERROR: Read BVH File Error!!");
	}


	////////////////////////////////////////////////////////////////
	// MMD VMD2
	else if (fd==FileDataVMD2) {
		int num = 0;
		if (nifile_dev!=NULL) {
			num = nifile_dev->readVMDFile(fname);
		}

		if (num>0) ret = TRUE;
		else err_message = _T("CExNiDevice::openLoadFile(): ERROR: Read MMD VMD2 File Error!!");
	}


	////////////////////////////////////////////////////////////////
	// Joints Text
	else if (fd==FileDataJTXT) {
		int num = 0;
		if (nifile_dev!=NULL) {
			num = nifile_dev->readJTextFile(fname);
		}

		if (num>0) ret = TRUE;
		else err_message = _T("CExNiDevice::openLoadFile(): ERROR: Read Text Joints File Error!!");
	}


	////////////////////////////////////////////////////////////////
	// ONI
	else if (fd==FileDataONI) {
		//
		if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
			#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
			if (openni_win!=NULL) {
				ret = makeFileDevice(fname);
				if (ret) {
					openni_win->playerSetRepeat(TRUE);
					openni_win->playerSetSeek(0);
					prvInputDevice = inputDevice;
					inputDevice = NiDevice_File;
					loaded_oni = TRUE;
				}
				else {
					err_message = LoadString_byID(IDS_STR_ERR_UNKNOWN_FILE);
				}
			}
			#endif
		}
		//
		else if (nowSDKLib==NiSDK_Kinect) {
			#ifdef ENABLE_KINECT_SDK
			if (kinect_sdk!=NULL) {
				// NOP
			}
			#endif
		}
		//
		else {
			err_message = LoadString_byID(IDS_STR_ERR_LIB_LOAD_FILE);
		}
	}

	//
	else {
		err_message = LoadString_byID(IDS_STR_ERR_UNKNOWN_FILE);
	}

	if (ret) rftype = fd;

	return ret;
}





//////////////////////////////////////////////////////////////////////////////////////////
//
// Local Data Log File and Network Data Log
//

BOOL  CExNiDevice::startLogRecorde(void)
{
	err_message = _T("");

	BOOL ret = FALSE;
	if (nifile_dev!=NULL) {
		ret = nifile_dev->openLogFile(appParam.saveLogFolder);
	}
	if (!ret) return FALSE;

	//
	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			openni_win->logfSaving = TRUE;
		}
		else return FALSE;
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			kinect_sdk->logfSaving = TRUE;
		}
		else return FALSE;
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::startLogRecorde(): ERROR: Unknown Library Type.");
		return FALSE;
	}

	return TRUE;
}




void  CExNiDevice::stopLogRecorde(void)
{
	err_message = _T("");

	if (nifile_dev==NULL) return;

	//
	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			openni_win->logfSaving = FALSE;
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			kinect_sdk->logfSaving = FALSE;
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::stopLogRecorde(): ERROR: Unknown Library Type.");
		return;
	}

	::Sleep(NIDEVICE_WAIT_TIME);
	nifile_dev->closeLogFile();

	return;
}





BOOL  CExNiDevice::startLogTransfer(void)
{
	err_message = _T("");

	BOOL ret = FALSE;

	//
	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			ret = openni_win->startLogTransfer();
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			ret = kinect_sdk->startLogTransfer();
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::startLogTransfer(): ERROR: Unknown Library Type.");
	}

	return ret;
}



//
//
//
void  CExNiDevice::stopLogTransfer(void)
{
	err_message = _T("");

	//
	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			openni_win->stopLogTransfer();
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			kinect_sdk->stopLogTransfer();
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::stopLogTransfer(): ERROR: Unknown Library Type.");
	}

	return;
}





//////////////////////////////////////////////////////////////////////////////////////////
//
// USB
//

BOOL  CExNiDevice::openUSBDevice(void)
{
	err_message = _T("");

	enableUSBDev = FALSE;

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			enableUSBDev = openni_win->openUSBDevice();
		}
		#endif
	}
	//	
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			enableUSBDev = kinect_sdk->openUSBDevice();
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::openUSBDevice(): ERROR: Unknown Library Type.");
	}

	return enableUSBDev;
}



//
void  CExNiDevice::closeUSBDevice(void)
{
	err_message = _T("");

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			openni_win->closeUSBDevice();
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			kinect_sdk->closeUSBDevice();
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::closeUSBDevice(): ERROR: Unknown Library Type.");
	}

	enableUSBDev = FALSE;
	return;
}



void  CExNiDevice::setTiltMotor(int ang)
{
	err_message = _T("");

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			openni_win->setTiltMotor(ang);
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			kinect_sdk->setTiltMotor(ang);
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::setTiltMotor(): ERROR: Unknown Library Type.");
	}

	return;
}


	
void  CExNiDevice::setLEDColor(int col)
{
	err_message = _T("");

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			openni_win->setLEDColor(col);
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			kinect_sdk->nop();
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::setLEDColor(): ERROR: Unknown Library Type.");
	}

	return;
}



void  CExNiDevice::setUseMotor(BOOL use)
{
	err_message = _T("");

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			openni_win->m_use_motor = use;
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			kinect_sdk->m_use_motor = use;
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::setUseMotor(): ERROR: Unknown Library Type.");
	}

	return;
}



void  CExNiDevice::setUseLED(BOOL use)
{
	err_message = _T("");

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			openni_win->m_use_led = use;
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			kinect_sdk->m_use_led = use;
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::setUseLED(): ERROR: Unknown Library Type.");
	}

	return;
}





//////////////////////////////////////////////////////////////////////////////////////////
//
// File Device
//

BOOL  CExNiDevice::makeFileDevice(LPCTSTR fname)
{
	err_message = _T("");

	BOOL ret = FALSE;

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) {
			ret = openni_win->makeFileDevice(fname);
			if (ret) {
				image_xsize = openni_win->getXSize();
				image_ysize = openni_win->getYSize();
				image_fps   = openni_win->getFPS();
			}
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			kinect_sdk->nop();
		}
		#endif
	}
	else {
		err_message = _T("CExNiDevice::makeFileDevice(): ERROR: Unknown Library Type.");
	}

	return ret;
}




BOOL  CExNiDevice::restoreDevice(void)
{
	err_message = _T("");

	BOOL ret = FALSE;

	if (nowSDKLib==NiSDK_OpenNI || nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		if (openni_win!=NULL) { 
			ret = openni_win->restoreDevice();
			if (ret) {
				image_xsize = openni_win->getXSize();
				image_ysize = openni_win->getYSize();
				image_fps   = openni_win->getFPS();
			}
		}
		#endif
	}
	//
	else if (nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		if (kinect_sdk!=NULL) {
			kinect_sdk->nop();
		}
		#endif
	}
	//
	else {
		err_message = _T("CExNiDevice::restoreDevice(): ERROR: Unknown Library Type.");
	}

	return ret;
}







//////////////////////////////////////////////////////////////////////////////////////////
//
//
//

UINT  niDeviceEventLoop(LPVOID pParam)
{
	UINT  ret = 1;
	if (pParam==NULL) return ret;
	CExNiDevice* niDevice = (CExNiDevice*)pParam;

	ret = ExecDocFrmView(niDevice->pSensorFrame);
	if (ret>0) {
		ExecDocFrmViewError(niDevice->hWnd, ret);
		return ret;
	}

	//
	if (niDevice->nowSDKLib==NiSDK_OpenNI ||niDevice-> nowSDKLib==NiSDK_OpenNI2) {
		#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)
		ret = openniEventLoop((LPVOID)niDevice->openni_win);
        #endif
	}
	//
	else if (niDevice->nowSDKLib==NiSDK_Kinect) {
		#ifdef ENABLE_KINECT_SDK
		ret = kinectEventLoop((LPVOID)niDevice->kinect_sdk);
        #endif
	}

	if (ret>0) {
		MessageBox(niDevice->hWnd, niDevice->getErrorMessage(TRUE), _T("Error"), MB_OK);
	}

	return ret;
}