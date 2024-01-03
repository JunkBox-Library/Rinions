// SetSpeech.cpp : 実装ファイル
//

#include "stdafx.h"

#include "resource.h"
#include "SetSpeech.h"
#include "WinTools.h"



// CSetSpeech ダイアログ

IMPLEMENT_DYNAMIC(CSetSpeech, CDialog)

CSetSpeech::CSetSpeech(NiSDK_Lib lib, CParameterSet prm, CWnd* pParent /*=NULL*/)
	: CDialog(CSetSpeech::IDD, pParent)
{
	param		= prm;
	sdk_lib		= lib;

	speechCBox  = NULL;
	confdSldr	= NULL;
	confdEBox	= NULL;
	langCombo	= NULL;

	confidence  = param.confdSpeech;

	if (confidence>1.0)      confidence = 1.0;
	else if (confidence<0.0) confidence = 0.0;
}



void CSetSpeech::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}




BEGIN_MESSAGE_MAP(CSetSpeech, CDialog)
	ON_WM_HSCROLL()
	ON_EN_CHANGE(IDC_EDIT_SPEECH_CONFD,  OnChangeSpeechConfd)
	ON_BN_CLICKED(IDC_CHECK_SPEECH_CTRL, &CSetSpeech::OnCheckSpeechCtrl)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_SPEECH_CONFD, &CSetSpeech::OnNMCustomdrawSliderSpeechConfd)
	ON_MESSAGE(JBXWL_WM_SPEECH_EVENT, &CSetSpeech::OnSpeechEvent)
END_MESSAGE_MAP()




BOOL CSetSpeech::OnInitDialog()
{
	TCHAR buf[LNAME];

	CDialog::OnInitDialog();
	
	speechCBox = (CButton*)GetDlgItem(IDC_CHECK_SPEECH_CTRL);
	confdSldr  = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_SPEECH_CONFD);
	confdEBox  = (CEdit*)GetDlgItem(IDC_EDIT_SPEECH_CONFD);
	langCombo  = (CComboBox*)GetDlgItem(IDC_COMBO_SPEECH_LANG);

	//
	if (param.useSpeechReco) speechCBox->SetCheck(1);
	else					 speechCBox->SetCheck(0);

	//
	confdSldr->SetRange(0, 10);
	confdSldr->SetPos((int)(confidence*10));
	sntprintf(buf, LNAME, _T("%4.2f"), confidence);
	confdEBox->SetWindowText(buf);
	//

	// 1: Japanese
	// other: English
	if (!param.langSpeech.Compare(_T("Japanese"))) {
		langCombo->SetCurSel(1);
	}
	else {
		langCombo->SetCurSel(0);
	}


	OnCheckSpeechCtrl();

	//
	if (sdk_lib==NiSDK_OpenNI || sdk_lib==NiSDK_OpenNI2) {
		//speechCBox->EnableWindow(FALSE);
		confdSldr->EnableWindow(FALSE);
		confdEBox->EnableWindow(FALSE);
		langCombo->EnableWindow(FALSE);
	}

	//
	return TRUE;
}




void CSetSpeech::OnOK()
{
	TCHAR buf[LNAME];

	if (speechCBox->GetCheck()) param.useSpeechReco = TRUE;
	else                        param.useSpeechReco = FALSE;

	confdEBox->GetWindowText(buf, LNAME);
	param.confdSpeech = (float)ttof(buf);
	if (param.confdSpeech>1.0)      param.confdSpeech = 1.0;
	else if (param.confdSpeech<0.0) param.confdSpeech = 0.0;

	if (langCombo->GetCurSel()==1) {
		param.langSpeech = _T("Japanese");
	}
	else {
		param.langSpeech = _T("English");
	}

	CDialog::OnOK();
}




void CSetSpeech::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	TCHAR buf[LNAME];

	if (confdSldr==(CSliderCtrl*)pScrollBar) {
		confidence = confdSldr->GetPos()/10.0f;
		sntprintf(buf, LNAME, _T("%4.2f"), confidence);
		confdEBox->SetWindowText(buf);
		confdEBox->UpdateWindow();
	}
}



void CSetSpeech::OnChangeSpeechConfd()
{
	TCHAR buf[LNAME];

	confdEBox->GetWindowText(buf, LNAME);
	confidence = (float)ttof(buf);

	if (confidence<0.0 || confidence>1.0) {
		if (confidence>1.0)      confidence = 1.0;
		else if (confidence<0.0) confidence = 0.0;
		sntprintf(buf, LNAME, _T("%4.2f"), confidence);
		confdEBox->SetWindowText(buf);
		confdEBox->UpdateWindow();
	}

	confdSldr->SetPos((int)(confidence*10));
}




void CSetSpeech::OnCheckSpeechCtrl()
{
	if (speechCBox->GetCheck()) {
		confdSldr->EnableWindow(TRUE);
		confdEBox->EnableWindow(TRUE);
		langCombo->EnableWindow(TRUE);
	}
	else {
		confdSldr->EnableWindow(FALSE);
		confdEBox->EnableWindow(FALSE);
		langCombo->EnableWindow(FALSE);
	}
}




void CSetSpeech::OnNMCustomdrawSliderSpeechConfd(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	*pResult = 0;
}





///////////////////////////////////////////////////////////////////////////////////
//
afx_msg LRESULT CSetSpeech::OnSpeechEvent(WPARAM wParam, LPARAM lParam)
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




BOOL CSetSpeech::DoLocalTerminateAction(LPCTSTR com)
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