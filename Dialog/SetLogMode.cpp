// SetLogMode.cpp : 実装ファイル
//

#include "stdafx.h"

//#include "Rinions.h"
#include "resource.h"
#include "SetLogMode.h"
#include "WinTools.h"


// CSetLogMode ダイアログ


IMPLEMENT_DYNAMIC(CSetLogMode, CDialog)

CSetLogMode::CSetLogMode(NiSDK_Lib lib, CParameterSet prm, CWnd* pParent /*=NULL*/)
	: CDialog(CSetLogMode::IDD, pParent)
{
	sdk_lib = lib;
	param   = prm;

	posCBox	= NULL;
//	rotCBox	= NULL;
	qutCBox	= NULL;
	aglCBox = NULL;

	netCBox	= NULL;
	chkCBox	= NULL;
}



void CSetLogMode::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}




BEGIN_MESSAGE_MAP(CSetLogMode, CDialog)
	ON_MESSAGE(JBXWL_WM_SPEECH_EVENT, &CSetLogMode::OnSpeechEvent)
END_MESSAGE_MAP()




BOOL CSetLogMode::OnInitDialog()
{
	CDialog::OnInitDialog();

	posCBox = (CButton*)GetDlgItem(IDC_CHECK_LOG_POSITION);
//	rotCBox = (CButton*)GetDlgItem(DC_CHECK_LOG_ROT_MATRIX);
	qutCBox = (CButton*)GetDlgItem(IDC_CHECK_LOG_QUATERNION);
	aglCBox = (CButton*)GetDlgItem(IDC_CHECK_LOG_ANGLE);

	netCBox = (CButton*)GetDlgItem(IDC_CHECK_LOG_NETWORK);
	chkCBox = (CButton*)GetDlgItem(IDC_CHECK_LOG_NET_CHECK);

	if (param.printPostnMode) posCBox->SetCheck(1);
	else                      posCBox->SetCheck(0);
//	if (param.printRotMxMode) rotCBox->SetCheck(1);
//	else                      rotCBox->SetCheck(0);
	if (param.printQuateMode) qutCBox->SetCheck(1);
	else                      qutCBox->SetCheck(0);
	if (param.printAngleMode) aglCBox->SetCheck(1);
	else                      aglCBox->SetCheck(0);

	//
	if (param.printNetwkMode) netCBox->SetCheck(1);
	else				  	  netCBox->SetCheck(0);
	if (param.printNtChkMode) chkCBox->SetCheck(1);
	else                      chkCBox->SetCheck(0);

	/*
	if (sdk_lib==NiSDK_Kinect) {
		rotCBox->SetCheck(0);
		rotCBox->EnableWindow(FALSE);
	}*/

	return TRUE;
}



void CSetLogMode::OnOK()
{
	if (posCBox->GetCheck()) param.printPostnMode = TRUE;
	else                     param.printPostnMode = FALSE;
//	if (rotCBox->GetCheck()) param.printRotMxMode = TRUE;
//	else                     param.printRotMxMode = FALSE;
	if (qutCBox->GetCheck()) param.printQuateMode = TRUE;
	else                     param.printQuateMode = FALSE;
	if (aglCBox->GetCheck()) param.printAngleMode = TRUE;
	else                     param.printAngleMode = FALSE;

	if (netCBox->GetCheck()) param.printNetwkMode = TRUE;
	else                     param.printNetwkMode = FALSE;
	if (chkCBox->GetCheck()) param.printNtChkMode = TRUE;
	else                     param.printNtChkMode = FALSE;

	//
	param.printRotMxMode = FALSE;
	
	CDialog::OnOK();
}



///////////////////////////////////////////////////////////////////////////////////
//
afx_msg LRESULT CSetLogMode::OnSpeechEvent(WPARAM wParam, LPARAM lParam)
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




BOOL CSetLogMode::DoLocalTerminateAction(LPCTSTR com)
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