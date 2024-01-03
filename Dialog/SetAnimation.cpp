// SetAnimation.cpp : 実装ファイル
//

#include "stdafx.h"

#include "Rinions.h"
#include "SetAnimation.h"
#include "WinTools.h"
#include "MessageBoxDLG.h"


// CSetAnimation ダイアログ

IMPLEMENT_DYNAMIC(CSetAnimation, CDialog)

CSetAnimation::CSetAnimation(CParameterSet prm, BOOL login, CWnd* pParent /*=NULL*/)
	: CDialog(CSetAnimation::IDD, pParent)
{
	param   = prm;
	isLogin	= login;

	animUUIDEBox = NULL;
}




void CSetAnimation::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}



BEGIN_MESSAGE_MAP(CSetAnimation, CDialog)
	ON_MESSAGE(JBXWL_WM_SPEECH_EVENT, &CSetAnimation::OnSpeechEvent)
END_MESSAGE_MAP()




// CSetAnimation メッセージ ハンドラ


BOOL CSetAnimation::OnInitDialog()
{
	CDialog::OnInitDialog();

	animUUIDEBox = (CEdit*)GetDlgItem(IDC_EDIT_ANIMATION_UUID);
	animUUIDEBox->SetWindowText(param.animationUUID);

	if (isLogin) animUUIDEBox->EnableWindow(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
}



void CSetAnimation::OnOK()
{
	TCHAR str[SHMIF_INDEX_LEN];
	animUUIDEBox->GetWindowText(str, SHMIF_INDEX_LEN);

//	for (int i=ANM_COM_LEN_UUID; i<SHMIF_INDEX_LEN; i++) str[i] = TCHAR(0);
	if (tcslen(str)!=ANM_COM_LEN_UUID) {
		animUUIDEBox->SetWindowText(param.animationUUID);
		return;
	}
	param.animationUUID = str;

	CDialog::OnOK();
}




///////////////////////////////////////////////////////////////////////////////////
//
afx_msg LRESULT CSetAnimation::OnSpeechEvent(WPARAM wParam, LPARAM lParam)
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




BOOL CSetAnimation::DoLocalTerminateAction(LPCTSTR com)
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