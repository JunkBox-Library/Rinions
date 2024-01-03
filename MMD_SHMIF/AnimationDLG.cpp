// SetAnimation.cpp : 実装ファイル
//

#include "stdafx.h"

#include "AnimationDLG.h"
#include "SHMIF.h"
#include "WinTools.h"



// CAnimationDLG ダイアログ

IMPLEMENT_DYNAMIC(CAnimationDLG, CDialog)

CAnimationDLG::CAnimationDLG(CString uuid, CWnd* pParent /*=NULL*/)
	: CDialog(CAnimationDLG::IDD, pParent)
{
	animationUUID = uuid;     
	animUUIDEBox  = NULL;
}




void CAnimationDLG::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}



BEGIN_MESSAGE_MAP(CAnimationDLG, CDialog)
END_MESSAGE_MAP()




// CAnimationDLG メッセージ ハンドラ

void CAnimationDLG::OnOK()
{
	TCHAR str[SHMIF_INDEX_LEN];
	animUUIDEBox->GetWindowText(str, SHMIF_INDEX_LEN);

	if (tcslen(str)!=SHMIF_UUID_LEN) {
		animUUIDEBox->SetWindowText(animationUUID);
		return;
	}
	animationUUID = str;

	CDialog::OnOK();
}




BOOL CAnimationDLG::OnInitDialog()
{
	CDialog::OnInitDialog();

	animUUIDEBox = (CEdit*)GetDlgItem(IDC_EDIT_ANIMATION_UUID);
	animUUIDEBox->SetWindowText(animationUUID);

	return TRUE;  // return TRUE unless you set the focus to a control
}
