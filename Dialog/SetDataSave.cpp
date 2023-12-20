// setDataSave.cpp : 実装ファイル
//

#include "stdafx.h"

#include "Rinions.h"
#include "SetDataSave.h"
#include "WinTools.h"


// CSetDataSave ダイアログ

IMPLEMENT_DYNAMIC(CSetDataSave, CDialog)

CSetDataSave::CSetDataSave(NiSDK_Lib lib, CParameterSet prm, CWnd* pParent /*=NULL*/)
	: CDialog(CSetDataSave::IDD, pParent)
{
	param   = prm;
	sdk_lib	= lib;

	if      (param.saveBVHFPS<10) param.saveBVHFPS = 10;
	else if (param.saveBVHFPS>60) param.saveBVHFPS = 60;

	orgCBox	= NULL;
	oniCBox	= NULL;
	bvhCBox	= NULL;
	nsvCBox = NULL;

	bvhFmtCombo = NULL;
	bvhFPSCombo = NULL;

	divTimeEBox = NULL;
	szScaleEBox	= NULL;

//	divTime		= param.saveDivTime;
	divTime		= 0;
	szScale		= param.saveSzScale;

	netLogCBox  = NULL;
	lclLogCBox  = NULL;
	fldrLogEBox = NULL;
	referButton = NULL;
}



void CSetDataSave::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSetDataSave, CDialog)
	ON_BN_CLICKED(IDC_RADIO_SAVE_ORG, &CSetDataSave::OnBnClickedRadioSaveOrg)
	ON_BN_CLICKED(IDC_RADIO_SAVE_ONI, &CSetDataSave::OnBnClickedRadioSaveOni)
	ON_BN_CLICKED(IDC_RADIO_SAVE_BVH, &CSetDataSave::OnBnClickedRadioSaveBvh)
	ON_MESSAGE(JBXWL_WM_SPEECH_EVENT, &CSetDataSave::OnSpeechEvent)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_LOG_REFER, &CSetDataSave::OnBnClickedButtonSaveLogRefer)
	ON_EN_CHANGE(IDC_EDIT_SAVE_LOG_FLDR, &CSetDataSave::OnEnChangeEditSaveLogFldr)
	ON_BN_CLICKED(IDC_CHECK_SAVE_LOG_LCL, &CSetDataSave::OnBnClickedCheckSaveLogLcl)
	ON_BN_CLICKED(IDC_RADIO_NOT_SAVE, &CSetDataSave::OnBnClickedRadioNotSave)
END_MESSAGE_MAP()


// CSetDataSave メッセージ ハンドラ



BOOL CSetDataSave::OnInitDialog()
{
	TCHAR buf[LNAME];

	CDialog::OnInitDialog();

	orgCBox = (CButton*)GetDlgItem(IDC_RADIO_SAVE_ORG);
	oniCBox = (CButton*)GetDlgItem(IDC_RADIO_SAVE_ONI);
	bvhCBox = (CButton*)GetDlgItem(IDC_RADIO_SAVE_BVH);
	nsvCBox = (CButton*)GetDlgItem(IDC_RADIO_NOT_SAVE);

	bvhFmtCombo = (CComboBox*)GetDlgItem(IDC_COMB_SAVE_BVH_FMT);
	bvhFPSCombo = (CComboBox*)GetDlgItem(IDC_COMB_SAVE_BVH_FPS);
	szScaleEBox = (CEdit*)GetDlgItem(IDC_EDIT_SAVE_SZSCALE);
	divTimeEBox = (CEdit*)GetDlgItem(IDC_EDIT_SAVE_DIVTIME);

	//
	bvhFmtCombo->SetCurSel(param.saveBVHFormat);
	bvhFPSCombo->SetCurSel(param.saveBVHFPS/10-1);

	sntprintf(buf, LNAME, _T("%6.3f"), szScale);
	szScaleEBox->SetWindowText(buf);
	sntprintf(buf, LNAME, _T("%d"), divTime);
	divTimeEBox->SetWindowText(buf);

	//bvhFmtCombo->EnableWindow(FALSE);
	//bvhFPSCombo->EnableWindow(FALSE);
	//szScaleEBox->EnableWindow(FALSE);
	//divTimeEBox->EnableWindow(FALSE);

	//
	netLogCBox  = (CButton*)GetDlgItem(IDC_CHECK_SAVE_LOG_NET);
	lclLogCBox  = (CButton*)GetDlgItem(IDC_CHECK_SAVE_LOG_LCL);
	fldrLogEBox = (CEdit*)GetDlgItem(IDC_EDIT_SAVE_LOG_FLDR);
	referButton = (CButton*)GetDlgItem(IDC_BUTTON_SAVE_LOG_REFER);

	//
	if (param.saveDataMode==FileDataBVH) {
		OnBnClickedRadioSaveBvh();
	}
	else if (param.saveDataMode==FileDataONI) {
		OnBnClickedRadioSaveOni();
	}	
	else if (param.saveDataMode==FileDataJTXT) {
		OnBnClickedRadioSaveOrg();
	}
	else {
		OnBnClickedRadioNotSave();
	}

	if (param.saveNoData) {
		OnBnClickedRadioNotSave();
	}

	//
	if (sdk_lib==NiSDK_Kinect) {
		if (param.saveDataMode==FileDataONI) OnBnClickedRadioSaveOrg();
		oniCBox->EnableWindow(FALSE);
	}

	//
	if (param.sendLogNet)	netLogCBox->SetCheck(1);
	else					netLogCBox->SetCheck(0);
	if (param.saveLogLocal)	lclLogCBox->SetCheck(1);
	else					lclLogCBox->SetCheck(0);
	fldrLogEBox->SetWindowText(param.saveLogFolder);

	return TRUE;
}




void CSetDataSave::OnOK()
{
	TCHAR buf[LNAME];

	if (bvhCBox->GetCheck()) param.saveDataMode = FileDataBVH;
	else if (oniCBox->GetCheck()) param.saveDataMode = FileDataONI;
	else if (orgCBox->GetCheck()) param.saveDataMode = FileDataJTXT;
	else param.saveDataMode = FileDataNone;

	if (nsvCBox->GetCheck()) param.saveNoData = TRUE;
	else                     param.saveNoData = FALSE;

	//
	param.saveBVHFormat = bvhFmtCombo->GetCurSel();
	param.saveBVHFPS = (bvhFPSCombo->GetCurSel() + 1)*10;
	if      (param.saveBVHFPS<10) param.saveBVHFPS = 10;
	else if (param.saveBVHFPS>60) param.saveBVHFPS = 60;

	szScaleEBox->GetWindowText(buf, LNAME);
	szScale = (float)ttof(buf);
	divTimeEBox->GetWindowText(buf, LNAME);
	divTime = ttoi(buf);

	param.saveSzScale = szScale;
	param.saveDivTime = divTime;

	//
	if (netLogCBox->GetCheck())  param.sendLogNet     = TRUE;
	else                         param.sendLogNet     = FALSE;
	if (lclLogCBox->GetCheck())  param.saveLogLocal   = TRUE;
	else                         param.saveLogLocal   = FALSE;


	fldrLogEBox->GetWindowText(buf, SHMIF_INDEX_LEN);
	param.saveLogFolder = buf;

	//
	CDialog::OnOK();
}




void CSetDataSave::OnBnClickedRadioSaveOrg()
{
	orgCBox->SetCheck(1);
	oniCBox->SetCheck(0);
	bvhCBox->SetCheck(0);
	nsvCBox->SetCheck(0);

	bvhFmtCombo->EnableWindow(FALSE);
	bvhFPSCombo->EnableWindow(FALSE);
	szScaleEBox->EnableWindow(FALSE);
	divTimeEBox->EnableWindow(FALSE);
}



void CSetDataSave::OnBnClickedRadioSaveBvh()
{
	orgCBox->SetCheck(0);
	oniCBox->SetCheck(0);
	bvhCBox->SetCheck(1);
	nsvCBox->SetCheck(0);

	bvhFmtCombo->EnableWindow(TRUE);
	bvhFPSCombo->EnableWindow(TRUE);
	szScaleEBox->EnableWindow(TRUE);
	divTimeEBox->EnableWindow(TRUE);
}



void CSetDataSave::OnBnClickedRadioSaveOni()
{
	orgCBox->SetCheck(0);
	oniCBox->SetCheck(1);
	bvhCBox->SetCheck(0);
	nsvCBox->SetCheck(0);

	bvhFmtCombo->EnableWindow(FALSE);
	bvhFPSCombo->EnableWindow(FALSE);
	szScaleEBox->EnableWindow(FALSE);
	divTimeEBox->EnableWindow(FALSE);
}



void CSetDataSave::OnBnClickedRadioNotSave()
{
	orgCBox->SetCheck(0);
	oniCBox->SetCheck(0);
	bvhCBox->SetCheck(0);
	nsvCBox->SetCheck(1);

	bvhFmtCombo->EnableWindow(FALSE);
	bvhFPSCombo->EnableWindow(FALSE);
	szScaleEBox->EnableWindow(FALSE);
	divTimeEBox->EnableWindow(FALSE);

	/*
	if (nsvCBox->GetCheck()) {
		orgCBox->EnableWindow(FALSE);
		oniCBox->EnableWindow(FALSE);
		bvhCBox->EnableWindow(FALSE);
		//
		bvhFmtCombo->EnableWindow(FALSE);
		bvhFPSCombo->EnableWindow(FALSE);
		szScaleEBox->EnableWindow(FALSE);
		divTimeEBox->EnableWindow(FALSE);	
	}
	else {
		orgCBox->EnableWindow(TRUE);
		oniCBox->EnableWindow(TRUE);
		bvhCBox->EnableWindow(TRUE);
		//
		if      (orgCBox->GetCheck()) OnBnClickedRadioSaveOrg();
		else if (bvhCBox->GetCheck()) OnBnClickedRadioSaveBvh();
		else if (oniCBox->GetCheck()) OnBnClickedRadioSaveOni();
	}*/
}



//
void CSetDataSave::OnBnClickedCheckSaveLogLcl()
{
	if (lclLogCBox->GetCheck()) {
//		nsvCBox->EnableWindow(TRUE);
		fldrLogEBox->EnableWindow(TRUE);
		referButton->EnableWindow(TRUE);
	}
	else {
//		nsvCBox->EnableWindow(FALSE);
		fldrLogEBox->EnableWindow(FALSE);
		referButton->EnableWindow(FALSE);
		//nsvCBox->SetCheck(0);
		//OnBnClickedRadioNotSave();
	}
}




void CSetDataSave::OnBnClickedButtonSaveLogRefer()
{
	TCHAR buf[LNAME];
	fldrLogEBox->GetWindowText(buf, SHMIF_INDEX_LEN);

	CString folder = EasyGetSaveFolderName(buf);
	fldrLogEBox->SetWindowText(folder);
}


void CSetDataSave::OnEnChangeEditSaveLogFldr()
{

}





///////////////////////////////////////////////////////////////////////////////////
//
afx_msg LRESULT CSetDataSave::OnSpeechEvent(WPARAM wParam, LPARAM lParam)
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




BOOL CSetDataSave::DoLocalTerminateAction(LPCTSTR com)
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



