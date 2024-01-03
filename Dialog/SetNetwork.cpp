// SetNetwork.cpp : 実装ファイル
//

#include "stdafx.h"

#include "resource.h"
#include "anm_data.h"
#include "SetNetwork.h"
#include "ExNiLib/ExNiSHMemory.h"
#include "WinTools.h"




/*
Delete
IDD_SETNETWORK
　　CONTROL         " 自動調整  （未）",IDC_CHECK_AUTO_IN_FPS,"Button",BS_AUTOCHECKBOX | WS_TABSTOP,20,115,57,10
    CONTROL         " Automatic adjustment (not yet)",IDC_CHECK_AUTO_IN_FPS,
                    "Button",BS_AUTOCHECKBOX | WS_TABSTOP,20,115,111,10
*/



// CSetNetwork ダイアログ

IMPLEMENT_DYNAMIC(CSetNetwork, CDialog)

CSetNetwork::CSetNetwork(CParameterSet prm, BOOL login, CWnd* pParent /*=NULL*/)
	: CDialog(CSetNetwork::IDD, pParent)
{
	param   = prm;
	isLogin	= login;

	userNameEBox = NULL;

	outputNetMemButton  = NULL;
	outputNetworkButton = NULL;
	outputNetLogCBox	= NULL; 
//	fastNetworkCBox		= NULL;

	animServerEBox = NULL;
	serverPortEBox = NULL;
	clientPortEBox = NULL;
	groupKeyEBox   = NULL;

//	autobpsCBox	   = NULL;
	maxbpsSldr	   = NULL;
	maxbpsEBox	   = NULL;

	MaxBPS = (SHMIF_CHANNEL_NUM-1)*NI_FPS_MAX*3;
	MaxBPS = (MaxBPS/500 + 1)*500;
	maxbps = param.inMaxBPS;
	if (maxbps>MaxBPS) maxbps = MaxBPS;
	else if (maxbps<1) maxbps = 1;
}



void CSetNetwork::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}



BEGIN_MESSAGE_MAP(CSetNetwork, CDialog)
	ON_WM_HSCROLL()
	ON_EN_CHANGE(IDC_EDIT_IN_FPS, OnChangeInFPS)
	//ON_BN_CLICKED(IDC_CHECK_AUTO_IN_FPS, &CSetNetwork::OnCheckAutoInFPS)
	ON_MESSAGE(JBXWL_WM_SPEECH_EVENT, &CSetNetwork::OnSpeechEvent)
	ON_BN_CLICKED(IDC_RADIO_NETWORK, &CSetNetwork::OnBnClickedRadioNetwork)
	ON_BN_CLICKED(IDC_RADIO_NET_SMEM, &CSetNetwork::OnBnClickedRadioNetSmem)
END_MESSAGE_MAP()




// CSetNetwork メッセージ ハンドラ

BOOL CSetNetwork::OnInitDialog()
{
	TCHAR buf[LNAME];
//	memset(buf, 0, LNAME);

	CDialog::OnInitDialog();

	userNameEBox = (CEdit*)GetDlgItem(IDC_EDIT_RINIONS_NAME);

	outputNetMemButton  = (CButton*)GetDlgItem(IDC_RADIO_NET_SMEM);
	outputNetworkButton = (CButton*)GetDlgItem(IDC_RADIO_NETWORK);
	outputNetLogCBox    = (CButton*)GetDlgItem(IDC_CHECK_NET_LOGONLY);
//	fastNetworkCBox     = (CButton*)GetDlgItem(IDC_CHECK_FASTNETWORK);
	
	animServerEBox = (CEdit*)GetDlgItem(IDC_EDIT_SERVER_NAME);
	serverPortEBox = (CEdit*)GetDlgItem(IDC_EDIT_SERVER_PORT);
	clientPortEBox = (CEdit*)GetDlgItem(IDC_EDIT_CLIENT_PORT);
	groupKeyEBox   = (CEdit*)GetDlgItem(IDC_EDIT_GROUP_KEY);

	//autobpsCBox	= (CButton*)GetDlgItem(IDC_CHECK_AUTO_IN_FPS);
	maxbpsSldr	= (CSliderCtrl*)GetDlgItem(IDC_SLIDER_IN_FPS);
	maxbpsEBox	= (CEdit*)GetDlgItem(IDC_EDIT_IN_FPS);


	////////////////////////////////////////////////////////////////////
	// Not Yet
	param.inAutoBPS = FALSE;
	//autobpsCBox->EnableWindow(FALSE);

	userNameEBox->SetWindowText(param.userName);

	//
	if (param.netLogOnly)  outputNetLogCBox->SetCheck(1);
	else                   outputNetLogCBox->SetCheck(0);
//	if (param.netFastMode) fastNetworkCBox->SetCheck(1);
//	else                   fastNetworkCBox->SetCheck(0);

	if (param.netOutMode==NETandLOCAL) { 
		outputNetMemButton->SetCheck(1);
		outputNetworkButton->SetCheck(0);
		outputNetLogCBox->EnableWindow(TRUE);
	}
	else {
		outputNetMemButton->SetCheck(0);
		outputNetworkButton->SetCheck(1);
		outputNetLogCBox->EnableWindow(FALSE);
	}

	//
	sntprintf(buf, LNAME, _T("%d"), param.clientPort);
	clientPortEBox->SetWindowText(buf);

//	if (param.inAutoBPS) autobpsCBox->SetCheck(1);
//	else                 autobpsCBox->SetCheck(0);

	maxbpsSldr->SetRange(1, MaxBPS);
	maxbpsSldr->SetPos(maxbps);
	sntprintf(buf, LNAME, _T("%d"), maxbps);
	maxbpsEBox->SetWindowText(buf);

	animServerEBox->SetWindowText(param.animationSrvr);

	sntprintf(buf, LNAME, _T("%d"), param.serverPort);
	serverPortEBox->SetWindowText(buf);

	groupKeyEBox->SetWindowText(param.groupID);

	//
	OnCheckAutoInFPS();

	if (isLogin) {
		animServerEBox->EnableWindow(FALSE);
		serverPortEBox->EnableWindow(FALSE);
		clientPortEBox->EnableWindow(FALSE);
		groupKeyEBox->EnableWindow(FALSE);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
}



void CSetNetwork::OnOK()
{
	TCHAR buf[LNAME];
//	memset(buf, 0, LNAME);

	userNameEBox->GetWindowText(buf, LNAME);
	for (int i=ANM_COM_LEN_NAME; i<LNAME; i++) buf[i] = TCHAR(0);
	param.userName = buf;

	if (outputNetworkButton->GetCheck()) param.netOutMode = NETonly;
	else                                 param.netOutMode = NETandLOCAL;
	if (outputNetLogCBox->GetCheck())	 param.netLogOnly = TRUE;
	else								 param.netLogOnly = FALSE;

//	if (fastNetworkCBox->GetCheck())	 param.netFastMode = TRUE;
//	else								 param.netFastMode = FALSE;
//	if (autobpsCBox->GetCheck())		 param.inAutoBPS = TRUE;
//	else								 param.inAutoBPS = FALSE;

	clientPortEBox->GetWindowText(buf, LNAME);
	param.clientPort = ttoi(buf);

	maxbpsEBox->GetWindowText(buf, LNAME);
	param.inMaxBPS = ttoi(buf);

	animServerEBox->GetWindowText(buf, LNAME);
	param.animationSrvr = buf;

	serverPortEBox->GetWindowText(buf, LNAME);
	param.serverPort = ttoi(buf);
	
	groupKeyEBox->GetWindowText(buf, LNAME);
	for (int i=ANM_COM_LEN_IDKEY; i<LNAME; i++) buf[i] = TCHAR(0);
	param.groupID = buf;

	CDialog::OnOK();
}




void CSetNetwork::OnBnClickedRadioNetwork()
{
	outputNetLogCBox->EnableWindow(FALSE);
}





void CSetNetwork::OnBnClickedRadioNetSmem()
{
	outputNetLogCBox->EnableWindow(TRUE);
}





void CSetNetwork::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	TCHAR buf[LNAME];
//	memset(buf, 0, LNAME);

	if (maxbpsSldr==(CSliderCtrl*)pScrollBar) {
		maxbps = maxbpsSldr->GetPos();
		sntprintf(buf, LNAME, _T("%d"), maxbps);
		maxbpsEBox->SetWindowText(buf);
		maxbpsEBox->UpdateWindow();
	}
}



void CSetNetwork::OnChangeInFPS()
{
	TCHAR buf[LNAME];
//	memset(buf, 0, LNAME);

	maxbpsEBox->GetWindowText(buf, LNAME);
	maxbps = ttoi(buf);

	if (maxbps<0 || maxbps>MaxBPS) {
		if (maxbps>MaxBPS) maxbps = MaxBPS;
		else if (maxbps<1) maxbps = 1;
		sntprintf(buf, LNAME, _T("%d"), maxbps);
		maxbpsEBox->SetWindowText(buf);
		maxbpsEBox->UpdateWindow();
	}

	maxbpsSldr->SetPos(maxbps);
}





void CSetNetwork::OnCheckAutoInFPS()
{
	/*
	if (autobpsCBox->GetCheck()) {
		maxbpsSldr->EnableWindow(FALSE);
		maxbpsEBox->EnableWindow(FALSE);
	}
	else {
		maxbpsSldr->EnableWindow(TRUE);
		maxbpsEBox->EnableWindow(TRUE);
	}
	*/
}




///////////////////////////////////////////////////////////////////////////////////
//
afx_msg LRESULT CSetNetwork::OnSpeechEvent(WPARAM wParam, LPARAM lParam)
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




BOOL CSetNetwork::DoLocalTerminateAction(LPCTSTR com)
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

