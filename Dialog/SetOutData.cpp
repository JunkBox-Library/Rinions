// SetOutData.cpp : 実装ファイル
//

#include "stdafx.h"

#include "resource.h"
#include "SetOutData.h"
#include "WinTools.h"


/*
Delete
IDD_SETOUTDATA
    CONTROL         " 自動調整 （未）",IDC_CHECK_OUT_FPS_AUTO,"Button",BS_AUTOCHECKBOX | WS_TABSTOP,16,75,57,10
    CONTROL         " Automatic adjustment (not yet)",IDC_CHECK_OUT_FPS_AUTO,
                    "Button",BS_AUTOCHECKBOX | WS_TABSTOP,16,75,115,10
*/



// CSetOutData ダイアログ

IMPLEMENT_DYNAMIC(CSetOutData, CDialog)

CSetOutData::CSetOutData(CParameterSet prm, CWnd* pParent /*=NULL*/)
	: CDialog(CSetOutData::IDD, pParent)
{
	param		= prm;

	cordCBox	= NULL;
	quatCBox	= NULL;

	ctrlfpsCBox = NULL;
//	autofpsCBox = NULL;
	outfpsSldr	= NULL;
	outfpsEBox	= NULL;

	outfps = param.outDataFPS;
	if (outfps>NI_FPS_MAX) outfps = NI_FPS_MAX;
	else if (outfps<5)     outfps = 5;
}



void CSetOutData::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}




BEGIN_MESSAGE_MAP(CSetOutData, CDialog)
	ON_WM_HSCROLL()
	ON_EN_CHANGE(IDC_EDIT_OUT_FPS, OnChangeOutFPS)
	//ON_BN_CLICKED(IDC_CHECK_OUT_FPS_AUTO, &CSetOutData::OnCheckOutFPSAuto)
	ON_BN_CLICKED(IDC_CHECK_OUT_FPS_CTRL, &CSetOutData::OnCheckOutFPSCtrl)
	ON_MESSAGE(JBXWL_WM_SPEECH_EVENT, &CSetOutData::OnSpeechEvent)
END_MESSAGE_MAP()




BOOL CSetOutData::OnInitDialog()
{
	TCHAR buf[LNAME];

	CDialog::OnInitDialog();

	cordCBox	= (CButton*)GetDlgItem(IDC_CHECK_OUT_CORD);
	quatCBox	= (CButton*)GetDlgItem(IDC_CHECK_OUT_QUAT);
	
	ctrlfpsCBox = (CButton*)GetDlgItem(IDC_CHECK_OUT_FPS_CTRL);
//	autofpsCBox = (CButton*)GetDlgItem(IDC_CHECK_OUT_FPS_AUTO);
	outfpsSldr	= (CSliderCtrl*)GetDlgItem(IDC_SLIDER_OUT_FPS);
	outfpsEBox	= (CEdit*)GetDlgItem(IDC_EDIT_OUT_FPS);


	////////////////////////////////////////////////////////////////////
	// Not Yet
	param.outAutoFPS = FALSE;
	//autofpsCBox->EnableWindow(FALSE);


	if (param.outDataPostion) cordCBox->SetCheck(1);
	else					cordCBox->SetCheck(0);
	if (param.outDataQuate) quatCBox->SetCheck(1);
	else					quatCBox->SetCheck(0);

	//
	if (param.outCtrlFPS)   ctrlfpsCBox->SetCheck(1);
	else					ctrlfpsCBox->SetCheck(0);
//	if (param.outAutoFPS)   autofpsCBox->SetCheck(1);
//	else					autofpsCBox->SetCheck(0);

	//
	outfpsSldr->SetRange(5, NI_FPS_MAX);
	outfpsSldr->SetPos(outfps);
	sntprintf(buf, LNAME, _T("%d"), outfps);
	outfpsEBox->SetWindowText(buf);
	
	//
	if (param.outCtrlFPS) {
		OnCheckOutFPSAuto();
	}
	else {
		outfpsSldr->EnableWindow(FALSE);
		outfpsEBox->EnableWindow(FALSE);
		//autofpsCBox->EnableWindow(FALSE);
	}

	//
	return TRUE;
}




void CSetOutData::OnOK()
{
	TCHAR buf[LNAME];

	if (cordCBox->GetCheck()) param.outDataPostion = TRUE;
	else                      param.outDataPostion = FALSE;
	if (quatCBox->GetCheck()) param.outDataQuate = TRUE;
	else                      param.outDataQuate = FALSE;
	
	if (!param.outDataPostion && !param.outDataQuate) return;

	if (ctrlfpsCBox->GetCheck()) param.outCtrlFPS = TRUE;
	else                         param.outCtrlFPS = FALSE;
//	if (autofpsCBox->GetCheck()) param.outAutoFPS = TRUE;
//	else                         param.outAutoFPS = FALSE;

	outfpsEBox->GetWindowText(buf, LNAME);
	param.outDataFPS = ttoi(buf);
	if (param.outDataFPS>=NI_FPS_MAX) param.outDataFPS = NI_FPS_MAX;
	else if (param.outDataFPS<5)      param.outDataFPS = 5;

	CDialog::OnOK();
}





void CSetOutData::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	TCHAR buf[LNAME];

	if (outfpsSldr==(CSliderCtrl*)pScrollBar) {
		outfps = outfpsSldr->GetPos();
		sntprintf(buf, LNAME, _T("%d"), outfps);
		outfpsEBox->SetWindowText(buf);
		outfpsEBox->UpdateWindow();
	}
}



void CSetOutData::OnChangeOutFPS()
{
	TCHAR buf[LNAME];

	outfpsEBox->GetWindowText(buf, LNAME);
	outfps = ttoi(buf);

	if (outfps<0 || outfps>NI_FPS_MAX) {
		if (outfps>NI_FPS_MAX) outfps = NI_FPS_MAX;
		else if (outfps<5) outfps = 5;
		sntprintf(buf, LNAME, _T("%d"), outfps);
		outfpsEBox->SetWindowText(buf);
		outfpsEBox->UpdateWindow();
	}

	outfpsSldr->SetPos(outfps);
}



void CSetOutData::OnCheckOutFPSAuto()
{
	/*
	if (autofpsCBox->GetCheck()) {
		outfpsSldr->EnableWindow(FALSE);
		outfpsEBox->EnableWindow(FALSE);
	}
	else {
		outfpsSldr->EnableWindow(TRUE);
		outfpsEBox->EnableWindow(TRUE);
	}
	*/
}



void CSetOutData::OnCheckOutFPSCtrl()
{
	if (ctrlfpsCBox->GetCheck()) {
		outfpsSldr->EnableWindow(TRUE);
		outfpsEBox->EnableWindow(TRUE);
		//autofpsCBox->EnableWindow(TRUE); Not Yet
		OnCheckOutFPSAuto();
	}
	else {
		outfpsSldr->EnableWindow(FALSE);
		outfpsEBox->EnableWindow(FALSE);
		//autofpsCBox->EnableWindow(FALSE);
	}
}





///////////////////////////////////////////////////////////////////////////////////
//
afx_msg LRESULT CSetOutData::OnSpeechEvent(WPARAM wParam, LPARAM lParam)
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




BOOL CSetOutData::DoLocalTerminateAction(LPCTSTR com)
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