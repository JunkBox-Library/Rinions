// SetDevice.cpp : 実装ファイル
//

#include "stdafx.h"

#include "resource.h"
#include "SetDevice.h"
#include "WinTools.h"


// CSetDevice ダイアログ

IMPLEMENT_DYNAMIC(CSetDevice, CDialog)

//CSetDevice::CSetDevice(NiSDK_Lib lib, CParameterSet prm, CWnd* pParent /*=NULL*/)
CSetDevice::CSetDevice(CExNiDevice* dev, CParameterSet prm, CWnd* pParent /*=NULL*/)
	: CDialog(CSetDevice::IDD, pParent)
{
//	sdk_lib = lib;
	nidev	= dev;
	param   = prm;

	crntLibEitBox	= NULL;
	openniButton	= NULL;
	kinectButton	= NULL;
	useLEDButton    = NULL;
	useMotorButton  = NULL;
}



void CSetDevice::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSetDevice, CDialog)
	ON_MESSAGE(JBXWL_WM_SPEECH_EVENT, &CSetDevice::OnSpeechEvent)
END_MESSAGE_MAP()




// CSetDevice メッセージ ハンドラ

BOOL CSetDevice::OnInitDialog()
{
	CDialog::OnInitDialog();

	crntLibEitBox  = (CEdit*)  GetDlgItem(IDC_INFO_CRNT_LIB);
	openniButton   = (CButton*)GetDlgItem(IDC_RADIO_LIB_OPENNI);
	kinectButton   = (CButton*)GetDlgItem(IDC_RADIO_LIB_KINECT);
	useLEDButton   = (CButton*)GetDlgItem(IDC_CHECK_LED_CONTROL);
	useMotorButton = (CButton*)GetDlgItem(IDC_CHECK_MOTOR_CONTROL);

	if (nidev->nowSDKLib==NiSDK_OpenNI) {
		crntLibEitBox->SetWindowText(_T("OpenNI"));
	}
	else if (nidev->nowSDKLib==NiSDK_OpenNI2) {
		crntLibEitBox->SetWindowText(_T("OpenNI2"));
	}
	else if (nidev->nowSDKLib==NiSDK_Kinect) {
		crntLibEitBox->SetWindowText(_T("Kinect SDK"));
	}
	else {
		crntLibEitBox->SetWindowText(_T(""));
	}


	if (param.nextSDKLib==NiSDK_OpenNI || param.nextSDKLib==NiSDK_OpenNI2) {
		openniButton->SetCheck(1);
		kinectButton->SetCheck(0);
	}
	else if (param.nextSDKLib==NiSDK_Kinect) {
		openniButton->SetCheck(0);
		kinectButton->SetCheck(1);
	}

	if (param.useDevLED)   useLEDButton->SetCheck(1);
	else                   useLEDButton->SetCheck(0);
	if (param.useDevMotor) useMotorButton->SetCheck(1);
	else                   useMotorButton->SetCheck(0);


	//
	if (!nidev->enableUSBDev || nidev->nowSDKLib==NiSDK_None) {
		useLEDButton->SetCheck(0);
		useLEDButton->EnableWindow(FALSE);
		useMotorButton->SetCheck(0);
		//useMotorButton->EnableWindow(FALSE);
	}
	else if (nidev->nowSDKLib==NiSDK_Kinect) {
		useLEDButton->SetCheck(0);
		useLEDButton->EnableWindow(FALSE);
	}

	return TRUE;
}



void CSetDevice::OnOK()
{
	if (openniButton->GetCheck()) {
#ifdef ENABLE_OPENNI2
		param.nextSDKLib = NiSDK_OpenNI2;
#else
		param.nextSDKLib = NiSDK_OpenNI;
#endif
	}
	else if (kinectButton->GetCheck()) {
		param.nextSDKLib = NiSDK_Kinect;
	}

	if (useLEDButton->GetCheck())   param.useDevLED   = TRUE;
	else                            param.useDevLED   = FALSE;	
	if (useMotorButton->GetCheck()) param.useDevMotor = TRUE;
	else                            param.useDevMotor = FALSE;

	CDialog::OnOK();
}





///////////////////////////////////////////////////////////////////////////////////
//
afx_msg LRESULT CSetDevice::OnSpeechEvent(WPARAM wParam, LPARAM lParam)
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




BOOL CSetDevice::DoLocalTerminateAction(LPCTSTR com)
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