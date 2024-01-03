// SetViewMode.cpp : 実装ファイル
//

#include "stdafx.h"

#include "resource.h"
#include "SetViewMode.h"
#include "WinTools.h"


// CSetViewMode ダイアログ

IMPLEMENT_DYNAMIC(CSetViewMode, CDialog)

CSetViewMode::CSetViewMode(CParameterSet prm, NiInputDevice dev, CWnd* pParent /*=NULL*/)
	: CDialog(CSetViewMode::IDD, pParent)
{
	param   = prm;
	device  = dev;

	mrrCBox = NULL;
	imgCBox = NULL;
//	pseCBox = NULL;
}



void CSetViewMode::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}




BEGIN_MESSAGE_MAP(CSetViewMode, CDialog)
	ON_MESSAGE(JBXWL_WM_SPEECH_EVENT, &CSetViewMode::OnSpeechEvent)
END_MESSAGE_MAP()




BOOL CSetViewMode::OnInitDialog()
{
	CDialog::OnInitDialog();

	mrrCBox = (CButton*)GetDlgItem(IDC_CHECK_VIEW_MIRRORING);
	imgCBox = (CButton*)GetDlgItem(IDC_CHECK_VIEW_USEIMAGE);
//	pseCBox = (CButton*)GetDlgItem(IDC_CHECK_FORCE_POSE);
	lineCombo = (CComboBox*)GetDlgItem(IDC_COMBO_SKL_LINE);

//	if (state!=NI_STATE_DETECT_STOPPED) {
//		pseCBox->EnableWindow(FALSE);
//	}

	if (param.isMirroring) mrrCBox->SetCheck(1);
	else                   mrrCBox->SetCheck(0);
	if (param.isUseImage)  imgCBox->SetCheck(1);
	else                   imgCBox->SetCheck(0);

	if      (param.lineSkeleton<1) param.lineSkeleton = 1;
	else if (param.lineSkeleton>5) param.lineSkeleton = 5;
	lineCombo->SetCurSel(param.lineSkeleton-1);

	//
	if (device==NiDevice_None) {
		mrrCBox->EnableWindow(FALSE);
	}

	return TRUE;
}




void CSetViewMode::OnOK()
{
	if (mrrCBox->GetCheck()) param.isMirroring = TRUE;
	else                     param.isMirroring = FALSE;
	if (imgCBox->GetCheck()) param.isUseImage  = TRUE;
	else                     param.isUseImage  = FALSE;

	param.lineSkeleton = lineCombo->GetCurSel() + 1;

	CDialog::OnOK();
}





///////////////////////////////////////////////////////////////////////////////////
//
afx_msg LRESULT CSetViewMode::OnSpeechEvent(WPARAM wParam, LPARAM lParam)
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




BOOL CSetViewMode::DoLocalTerminateAction(LPCTSTR com)
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
