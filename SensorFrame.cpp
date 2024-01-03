// SensorFrame.cpp : 実装ファイル
//

#include  "stdafx.h"

#include  "SensorFrame.h"
#include  "MessageBoxDLG.h"



using namespace jbxl;
using namespace jbxwl;



// CSensorFrame

IMPLEMENT_DYNCREATE(CSensorFrame, CExFrame)

CSensorFrame::CSensorFrame()
{
	thisApp = NULL;
}



CSensorFrame::~CSensorFrame()
{
	DEBUG_INFO("DESTRUCTOR: CSensorFrame");

	if (thisApp->niDevice->pDepthFrame!=NULL) {
		delete(thisApp->niDevice->pDepthFrame);
		thisApp->niDevice->pDepthFrame = NULL;
	}

	OnCalibrationStop();
	deleteNull(pView);
}




BEGIN_MESSAGE_MAP(CSensorFrame, CExFrame)
	ON_UPDATE_COMMAND_UI(ID_CALIB_START, &CSensorFrame::OnStartButtonUpdate)
	ON_UPDATE_COMMAND_UI(ID_CALIB_STOP,  &CSensorFrame::OnStopButtonUpdate)
	ON_UPDATE_COMMAND_UI(ID_CALIB_NEXT,  &CSensorFrame::OnNextButtonUpdate)
	ON_COMMAND(ID_CALIB_START, OnCalibrationStart)
	ON_COMMAND(ID_CALIB_STOP,  OnCalibrationStop)
	ON_COMMAND(ID_CALIB_NEXT,  OnCalibrationNext)
	ON_UPDATE_COMMAND_UI(ID_REC_START, &CSensorFrame::OnRecStartButtonUpdate)
	ON_UPDATE_COMMAND_UI(ID_REC_STOP,  &CSensorFrame::OnRecStopButtonUpdate)
	ON_COMMAND(ID_REC_START, OnRecStart)
	ON_COMMAND(ID_REC_STOP,  OnRecStop)
	ON_UPDATE_COMMAND_UI(ID_MOTOR_UP,     &CSensorFrame::OnMotorUpButtonUpdate)
	ON_UPDATE_COMMAND_UI(ID_MOTOR_CENTER, &CSensorFrame::OnMotorCenterButtonUpdate)
	ON_UPDATE_COMMAND_UI(ID_MOTOR_DOWN,   &CSensorFrame::OnMotorDownButtonUpdate)
	ON_COMMAND(ID_MOTOR_UP,     OnMotorUp)
	ON_COMMAND(ID_MOTOR_CENTER, OnMotorCenter)
	ON_COMMAND(ID_MOTOR_DOWN,   OnMotorDown)
	ON_WM_CREATE()
	ON_MESSAGE(JBXWL_WM_SPEECH_EVENT, &CSensorFrame::OnEventSpeech)
END_MESSAGE_MAP()




// CSensorFrame メッセージ ハンドラ

int  CSensorFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CExFrame::OnCreate(lpCreateStruct) == -1) return -1;

//	toolBar = new CExToolBar(this);
	toolBar = new CExToolBar();

	if (!toolBar->CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP 
			| CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC | WS_CLIPCHILDREN) ||
		!toolBar->LoadToolBar(IDR_CALIB_TOOLBAR))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;
	}

	return 0;
}





/////////////////////////////////////////////////////////////////////////////////////////
//
// ToolBar Click
//

//
// Start Calibration
//
void  CSensorFrame::OnCalibrationStart()
{
	if (thisApp->niDevice->getDevState()==NI_STATE_DETECT_STOPPED) {
		thisApp->updateMenuBar();

		thisApp->niDevice->clearAvatarDetected();
			
		if (thisApp->niDevice->startDetection()) {
			if (thisApp->isLoggedIn) {
				thisApp->niNetwork->openSendSocket();
				if (thisApp->appParam.netOutMode==NETandLOCAL) thisApp->sharedMem->setLocalAnimationIndex();
			}
		}
		else {
			OnCalibrationStop();
			MessageBoxDLG(IDS_STR_ERROR, thisApp->niDevice->getErrorMessage(TRUE), MB_OK, this);
		}
	}
}



void  CSensorFrame::OnCalibrationStop()
{
	if (thisApp->niDevice->getDevState()==NI_STATE_DETECT_EXEC) {
		OnRecStop();

		if (!thisApp->niDevice->stopDetection()) {	// set NI_STATE_DETECT_STOPPED
			MessageBoxDLG(IDS_STR_ERROR, thisApp->niDevice->getErrorMessage(), MB_OK, this);
		}

		thisApp->sharedMem->isTracking = FALSE;
		thisApp->sharedMem->clearLocalAnimationData();
		if (thisApp->isLoggedIn) {
			if (thisApp->appParam.netOutMode==NETandLOCAL) thisApp->sharedMem->clearLocalAnimationIndex();
			thisApp->niNetwork->closeSendSocket();
		}

		thisApp->updateMenuBar();
	}
}



void  CSensorFrame::OnCalibrationNext()
{
	if (thisApp->niDevice->getDevState()==NI_STATE_DETECT_EXEC) {
		thisApp->niDevice->setDenyTrackingSearch();
		thisApp->updateMenuBar();
	}
}





void  CSensorFrame::OnRecStart()
{
	if (thisApp->niDevice->getDevState()==NI_STATE_DETECT_EXEC) {
//		if (!thisApp->appParam.saveLogLocal || !thisApp->appParam.saveNoData) {	// !(saveLogLocal && saveNoData)
		if (!thisApp->appParam.saveNoData) {
			if (!thisApp->dataSaving) {
				/*
				CString mesg;
				NiFileType sv_mode = thisApp->niDevice->appParam.saveDataMode;
				if      (sv_mode==FileDataBVH)  mesg.LoadString(IDS_STR_REC_MODE_BVH);
				else if (sv_mode==FileDataONI)  mesg.LoadString(IDS_STR_REC_MODE_ONI);
				else if (sv_mode==FileDataJTXT) mesg.LoadString(IDS_STR_REC_MODE_ORG);
				else {
					MessageBoxDLG(IDS_STR_ERROR, _T("OnRecStart(): Unknown File Type"), MB_OK, this);
					return;
				}
				int ret = MessageBoxDLG(IDS_STR_CNFRM, mesg, MB_YESNO, this);
				if (ret==IDNO) return;
				*/
				//
				thisApp->dataSaving = thisApp->niDevice->startRecorde();
				if (thisApp->dataSaving) {
					thisApp->savedData = FALSE;		
				}
				else {
					MessageBoxDLG(IDS_STR_ERROR, thisApp->niDevice->getErrorMessage(), MB_OK, this);
				}
			}
		}

		//
		if (thisApp->appParam.saveLogLocal && !thisApp->logfSaving) {
			thisApp->logfSaving = thisApp->niDevice->startLogRecorde();
			if (thisApp->logfSaving) DEBUG_INFO("CSensorFrame::OnRecStart(): Start to Record Log File");
			else DEBUG_ERR("CSensorFrame::OnRecStart(): Fail to Start to Record Log File");
		}
		//
		if (thisApp->isLoggedIn && thisApp->appParam.sendLogNet && !thisApp->logfSending) {
			thisApp->logfSending = thisApp->niDevice->startLogTransfer();
			thisApp->niNetwork->resetLapTimer();
			if (thisApp->logfSending) DEBUG_INFO("CSensorFrame::OnRecStart(): Start to Send Log File to the Server");
			else DEBUG_ERR("CSensorFrame::OnRecStart(): Fail to Start to Send Log File to the Server");
		}
	}
}



void  CSensorFrame::OnRecStop()
{
	if (thisApp->niDevice->getDevState()==NI_STATE_DETECT_EXEC) {
		if (thisApp->dataSaving) {
			thisApp->niDevice->stopRecorde();
			thisApp->dataSaving = FALSE;
			thisApp->savedData  = TRUE;
		}
		//
		if (thisApp->logfSaving) {
			thisApp->niDevice->stopLogRecorde();
			thisApp->logfSaving = FALSE;
			DEBUG_INFO("CSensorFrame::OnRecStart(): Stop to Record Log File");
		}
		//
		if (thisApp->logfSending) {
			thisApp->niDevice->stopLogTransfer();
			thisApp->logfSending = FALSE; 
			DEBUG_INFO("CSensorFrame::OnRecStart(): Stop to Send Log File to the Server");
		}
	}
}




// Motor
void  CSensorFrame::OnMotorUp()
{
	if (thisApp->niDevice->enableUSBDev && thisApp->appParam.useDevMotor) {
		thisApp->motorTilt += 2;
		if (thisApp->motorTilt>25) thisApp->motorTilt = 25;
		thisApp->niDevice->setTiltMotor(thisApp->motorTilt);
	}
}



void  CSensorFrame::OnMotorCenter()
{
	if (thisApp->niDevice->enableUSBDev && thisApp->appParam.useDevMotor) {
		thisApp->motorTilt = 0;
		thisApp->niDevice->setTiltMotor(thisApp->motorTilt);
	}
}



void  CSensorFrame::OnMotorDown()
{
	if (thisApp->niDevice->enableUSBDev && thisApp->appParam.useDevMotor) {
		thisApp->motorTilt -=2;
		if (thisApp->motorTilt<-25) thisApp->motorTilt = -25;
		thisApp->niDevice->setTiltMotor(thisApp->motorTilt);
	}
}





//////////////////////////////////////////////////////////////////////////////////
// Button Enable

void  CSensorFrame::OnStartButtonUpdate(CCmdUI* pCmdUI)
{
	if (thisApp->niDevice->getDevState()==NI_STATE_DETECT_STOPPED) {
		pCmdUI->Enable(TRUE);
	}
	else {
		pCmdUI->Enable(FALSE);
	}
	return;
}



void  CSensorFrame::OnStopButtonUpdate(CCmdUI* pCmdUI)
{
	if (thisApp->niDevice->getDevState()==NI_STATE_DETECT_EXEC) {
		pCmdUI->Enable(TRUE);
	}
	else {
		pCmdUI->Enable(FALSE);
	}
	return;
}



void  CSensorFrame::OnNextButtonUpdate(CCmdUI* pCmdUI)
{
	if (thisApp->niDevice->getDevState()==NI_STATE_DETECT_EXEC) {
		pCmdUI->Enable(TRUE);
	}
	else {
		pCmdUI->Enable(FALSE);
	}
	return;
}



// Record
void  CSensorFrame::OnRecStartButtonUpdate(CCmdUI* pCmdUI)
{
	if (thisApp->niDevice->getDevState()==NI_STATE_DETECT_EXEC 
				&& !thisApp->dataSaving && !thisApp->logfSaving && !thisApp->logfSending) {
		pCmdUI->Enable(TRUE);
	}
	else {
		pCmdUI->Enable(FALSE);
	}
	return;
}



void  CSensorFrame::OnRecStopButtonUpdate(CCmdUI* pCmdUI)
{
	if (thisApp->niDevice->getDevState()==NI_STATE_DETECT_EXEC 
					&& (thisApp->dataSaving || thisApp->logfSaving || thisApp->logfSending)) {
		pCmdUI->Enable(TRUE);
	}
	else {
		pCmdUI->Enable(FALSE);
	}
	return;
}



/*
void  CSensorFrame::OnSensorLEDButtonUpdate(CCmdUI* pCmdUI)
{
	//if (thisApp->niDevice->getDevState()==NI_STATE_DETECT_EXEC 
	//					&& (thisApp->dataSaving || thisApp->logfSaving || thisApp->logfSending)) {
	if (thisApp->niDevice->inputDevice==NiDevice_NI) {
		pCmdUI->Enable(TRUE);
	}
	else {
		pCmdUI->Enable(FALSE);
	}
	return;
}
*/



// Motor
void  CSensorFrame::OnMotorUpButtonUpdate(CCmdUI* pCmdUI)
{
	if (thisApp->niDevice->enableUSBDev && thisApp->appParam.useDevMotor) {
		pCmdUI->Enable(TRUE);
	}
	else {
		pCmdUI->Enable(FALSE);
	}
	return;
}



void  CSensorFrame::OnMotorCenterButtonUpdate(CCmdUI* pCmdUI)
{
	if (thisApp->niDevice->enableUSBDev && thisApp->appParam.useDevMotor) {
		pCmdUI->Enable(TRUE);
	}
	else {
		pCmdUI->Enable(FALSE);
	}
	return;
}



void  CSensorFrame::OnMotorDownButtonUpdate(CCmdUI* pCmdUI)
{
	if (thisApp->niDevice->enableUSBDev && thisApp->appParam.useDevMotor) {
		pCmdUI->Enable(TRUE);
	}
	else {
		pCmdUI->Enable(FALSE);
	}
	return;
}





//////////////////////////////////////////////////////////////////////////////////
// Speech Event

afx_msg LRESULT CSensorFrame::OnEventSpeech(WPARAM wParam, LPARAM lParam)
{
	if (wParam!=NULL && *(WORD*)wParam) {
		DoLocalEventAction((LPCTSTR)lParam);
	}

	return 0;
}






BOOL CSensorFrame::DoLocalEventAction(LPCTSTR com)
{
	if (!tcscmp(com, _T("START"))) {
		OnCalibrationStart();
	}
	else if (!tcscmp(com, _T("RECORD"))) {
		OnRecStart();
	}
	else if (!tcscmp(com, _T("STOP"))) {
		if (!thisApp->dataSaving && !thisApp->logfSaving && !thisApp->logfSending) {
			OnCalibrationStop();
		}
		else {
			OnRecStop();
		}
	}
	//
	else if (!tcscmp(com, _T("USER"))) {
		OnCalibrationNext();
	}
	//
	else if (!tcscmp(com, _T("MOTOR_UP"))) {
		OnMotorUp();
	}
	else if (!tcscmp(com, _T("MOTOR_DOWN"))) {
		OnMotorDown();
	}
	else if (!tcscmp(com, _T("MOTOR_CENTER"))) {
		OnMotorCenter();
	}
	//
	else {
		return FALSE;
	}

	toolBar->SetFocus();
	return TRUE;
}