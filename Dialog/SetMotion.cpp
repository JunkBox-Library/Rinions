// SetMotion.cpp : 実装ファイル
//

#include "stdafx.h"

#include "resource.h"
#include "SetMotion.h"
#include "WinTools.h"



//////////////////////////////////////////////////////////////////////////////////////////
// 
// CSetMotion ダイアログ
//

IMPLEMENT_DYNAMIC(CSetMotion, CDialog)

CSetMotion::CSetMotion(CParameterSet prm, UINT id, CWnd* pParent /*=NULL*/)
	: CDialog(id, pParent)
{
	param = prm;
}





//////////////////////////////////////////////////////////////////////////////////////////
// 
// CSetOpenNIMotion ダイアログ
//
// for OpenNI/OpenNI2
//

IMPLEMENT_DYNAMIC(CSetOpenNIMotion, CSetMotion)

CSetOpenNIMotion::CSetOpenNIMotion(CParameterSet prm, CWnd* pParent /*=NULL*/)
	: CSetMotion(prm, CSetOpenNIMotion::IDD, pParent)
{
//	posButton	= NULL;
//	rotButton	= NULL;

	cnfdncSldr	= NULL;
	cnfdncEBox	= NULL;

	jointCBox	= NULL;

	maSmthBtn	= NULL;
	maTypeCombo	= NULL;
	maNumCombo	= NULL;
	
	niteSmthBtn = NULL;
	smoothSldr	= NULL;
	smoothEBox	= NULL;

	YaxisSldr	= NULL;
	YaxisEBox	= NULL;

	partsCombo	= NULL;
	faceCBox	= NULL;

	confidence  = param.confidence;
	smoothNITE  = param.smoothNITE;
	YaxisCrrct  = (float)(param.YaxisCorrect/PI*180.);

	if      (confidence<0.5) confidence = 0.5f;
	else if (confidence>1.0) confidence = 1.0f;

	if      (smoothNITE<0.0) smoothNITE = 0.0f;
	else if (smoothNITE>1.0) smoothNITE = 1.0f;

	if      (YaxisCrrct<-15.0) YaxisCrrct = -15.0f;
	else if (YaxisCrrct> 15.0) YaxisCrrct =  15.0f;
}



void CSetOpenNIMotion::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}



BEGIN_MESSAGE_MAP(CSetOpenNIMotion, CDialog)
	ON_EN_CHANGE(IDC_EDIT_CNFDNC, OnChangeConfidence)
	ON_EN_CHANGE(IDC_EDIT_SMOOTH, OnChangeSmooth)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_CHECK_NITE_SMOOTH, OnCheckNiteSmooth)
	ON_BN_CLICKED(IDC_CHECK_MVAV_SMOOTH, OnCheckMvavSmooth)
	ON_EN_CHANGE(IDC_EDIT_YROT, &CSetOpenNIMotion::OnChangeYRot)
	ON_MESSAGE(JBXWL_WM_SPEECH_EVENT, &CSetOpenNIMotion::OnSpeechEvent)
END_MESSAGE_MAP()




// CSetOpenNIMotion メッセージ ハンドラ

BOOL CSetOpenNIMotion::OnInitDialog()
{
	TCHAR buf[LNAME];
//	memset(buf, 0, LNAME);

	CDialog::OnInitDialog();

//	posButton	= (CButton*)GetDlgItem(IDC_RADIO_MOTION_POS);
//	rotButton	= (CButton*)GetDlgItem(IDC_RADIO_MOTION_ROT);

	cnfdncSldr	= (CSliderCtrl*)GetDlgItem(IDC_SLIDER_CNFDNC);
	cnfdncEBox	= (CEdit*)GetDlgItem(IDC_EDIT_CNFDNC);

	jointCBox	= (CButton*)GetDlgItem(IDC_CHECK_JOINT_CONSTRAINT);

	maSmthBtn	= (CButton*)GetDlgItem(IDC_CHECK_MVAV_SMOOTH);
	maTypeCombo	= (CComboBox*)GetDlgItem(IDC_COMBO_MVAV_TYPE);
	maNumCombo	= (CComboBox*)GetDlgItem(IDC_COMBO_MVAV_NUM);

	niteSmthBtn = (CButton*)GetDlgItem(IDC_CHECK_NITE_SMOOTH);
	smoothSldr	= (CSliderCtrl*)GetDlgItem(IDC_SLIDER_SMOOTH);
	smoothEBox	= (CEdit*)GetDlgItem(IDC_EDIT_SMOOTH);
	
	YaxisSldr   = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_YROT);
	YaxisEBox	= (CEdit*)GetDlgItem(IDC_EDIT_YROT);

	partsCombo	= (CComboBox*)GetDlgItem(IDC_COMBO_DETECT);
	faceCBox	= (CButton*)GetDlgItem(IDC_CHECK_FACE_DETECT);

	/*
	if (param.useRotData) {
		posButton->SetCheck(0);
		rotButton->SetCheck(1);
	}
	else {
		posButton->SetCheck(1);
		rotButton->SetCheck(0);
	}*/

	cnfdncSldr->SetRange(50, 100);
	cnfdncSldr->SetPos((int)(confidence*100));
	sntprintf(buf, LNAME, _T("%4.2f"), confidence);
	cnfdncEBox->SetWindowText(buf);

	if (param.useJointConst) {
		jointCBox->SetCheck(1);
	}
	else {
		jointCBox->SetCheck(0);
	}

	//
	maTypeCombo->SetCurSel(param.mvavType-1);
	maNumCombo->SetCurSel(param.mvavNum-1);

	if (param.useMvavSmooth) {
		maSmthBtn->SetCheck(1);
		maTypeCombo->EnableWindow(TRUE);
		maNumCombo->EnableWindow(TRUE);
	}
	else {
		maSmthBtn->SetCheck(0);
		maTypeCombo->EnableWindow(FALSE);
		maNumCombo->EnableWindow(FALSE);
	}

	//
	smoothSldr->SetRange(0, 100);
	smoothSldr->SetPos((int)(smoothNITE*100));
	sntprintf(buf, LNAME, _T("%4.2f"), smoothNITE);
	smoothEBox->SetWindowText(buf);

	if (param.useNiteSmooth) {
		niteSmthBtn->SetCheck(1);
		smoothSldr->EnableWindow(TRUE);
		smoothEBox->EnableWindow(TRUE);
	}
	else {
		niteSmthBtn->SetCheck(0);
		smoothSldr->EnableWindow(FALSE);
		smoothEBox->EnableWindow(FALSE);
	}
	
	//
	YaxisSldr->SetRange(-150, 150);
	YaxisSldr->SetPos(100);
	YaxisSldr->SetPos((int)(YaxisCrrct*10));
	sntprintf(buf, LNAME, _T("%+4.1f"), YaxisCrrct);
	YaxisEBox->SetWindowText(buf);

	//
	partsCombo->SetCurSel(param.detectParts-2);

	if (param.useFaceDetect) {
		faceCBox->SetCheck(1);
	}
	else {
		faceCBox->SetCheck(0);
	}
	
	return TRUE;
}



void CSetOpenNIMotion::OnOK()
{
	TCHAR buf[LNAME];

	/*
	if (rotButton->GetCheck()) {
		param.usePosData = FALSE;
		param.useRotData = TRUE;
	}
	else  {
		param.usePosData = TRUE;
		param.useRotData = FALSE;
	}*/
	param.usePosData = TRUE;
	param.useRotData = FALSE;


	//
	cnfdncEBox->GetWindowText(buf, LNAME);
	param.confidence = (float)ttof(buf);
	if (param.confidence<0.5) param.confidence = 0.5;

	if (jointCBox->GetCheck()) {
		param.useJointConst = TRUE;
	}
	else {
		param.useJointConst = FALSE;
	}

	//
	if (maSmthBtn->GetCheck()) {
		param.useMvavSmooth = TRUE;
	}
	else {
		param.useMvavSmooth = FALSE;
	}
	param.mvavType = (NiMvAvType)(maTypeCombo->GetCurSel() + 1);
	param.mvavNum  = maNumCombo->GetCurSel() + 1;

	//
	smoothEBox->GetWindowText(buf, LNAME);
	param.smoothNITE = (float)ttof(buf);
	//
	if (niteSmthBtn->GetCheck()) {
		param.useNiteSmooth = TRUE;
	}
	else {
		param.useNiteSmooth = FALSE;
	}

	//
	YaxisEBox->GetWindowText(buf, LNAME);
	YaxisCrrct = (float)ttof(buf);
	if      (YaxisCrrct<-15.0) YaxisCrrct = -15.0f;
	else if (YaxisCrrct> 15.0) YaxisCrrct =  15.0f;
	param.YaxisCorrect = (float)(YaxisCrrct/180.*PI);

	//
	param.detectParts = partsCombo->GetCurSel() + 2;	// OpenNIに合わせる
	if (faceCBox->GetCheck()) {
		param.useFaceDetect = TRUE;
	}
	else {
		param.useFaceDetect = FALSE;
	}
	CDialog::OnOK();
}



void CSetOpenNIMotion::OnChangeConfidence() 
{
	TCHAR buf[LNAME];
//	memset(buf, 0, LNAME);

	cnfdncEBox->GetWindowText(buf, LNAME);
	confidence = (float)ttof(buf);

	if (confidence<0.0 || confidence>1.0) {
		if      (confidence<0.0) confidence = 0.0;
		else if (confidence>1.0) confidence = 1.0;
		sntprintf(buf, LNAME, _T("%4.2f"), confidence);
		cnfdncEBox->SetWindowText(buf);
		cnfdncEBox->UpdateWindow();
	}

	cnfdncSldr->SetPos((int)(confidence*100));
}



void CSetOpenNIMotion::OnChangeSmooth() 
{
	TCHAR buf[LNAME];

	smoothEBox->GetWindowText(buf, LNAME);
	smoothNITE = (float)ttof(buf);

	if (smoothNITE<0.0 || smoothNITE>1.0) {
		if      (smoothNITE<0.0) smoothNITE = 0.0;
		else if (smoothNITE>1.0) smoothNITE = 1.0;
		sntprintf(buf, LNAME, _T("%4.2f"), smoothNITE);
		smoothEBox->SetWindowText(buf);
		smoothEBox->UpdateWindow();
	}

	smoothSldr->SetPos((int)(smoothNITE*100));
}



void CSetOpenNIMotion::OnChangeYRot()
{
	TCHAR buf[LNAME];

	YaxisEBox->GetWindowText(buf, LNAME);
	YaxisCrrct = (float)ttof(buf);

	if (YaxisCrrct<-15.0 || YaxisCrrct>15.0) {
		if      (YaxisCrrct<-15.0) YaxisCrrct = -15.0f;
		else if (YaxisCrrct> 15.0) YaxisCrrct =  15.0f;
		sntprintf(buf, LNAME, _T("%+4.1f"), YaxisCrrct);
		YaxisEBox->SetWindowText(buf);
		YaxisEBox->UpdateWindow();
	}

	YaxisSldr->SetPos(100);
	YaxisSldr->SetPos((int)(YaxisCrrct*10));
}



void CSetOpenNIMotion::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	TCHAR buf[LNAME];

	if (cnfdncSldr==(CSliderCtrl*)pScrollBar) {
		int cnfd = cnfdncSldr->GetPos();
		sntprintf(buf, LNAME, _T("%4.2f"), cnfd/100.);
		cnfdncEBox->SetWindowText(buf);
		cnfdncEBox->UpdateWindow();
	}
	else if (smoothSldr==(CSliderCtrl*)pScrollBar) {
		int smth = smoothSldr->GetPos();
		sntprintf(buf, LNAME, _T("%4.2f"), smth/100.);
		smoothEBox->SetWindowText(buf);
		smoothEBox->UpdateWindow();
	}
	else if (YaxisSldr==(CSliderCtrl*)pScrollBar) {
		int yaxis = YaxisSldr->GetPos();
		sntprintf(buf, LNAME, _T("%+4.1f"), yaxis/10.);
		YaxisEBox->SetWindowText(buf);
		YaxisEBox->UpdateWindow();
	}
}




void CSetOpenNIMotion::OnCheckNiteSmooth()
{
	if (niteSmthBtn->GetCheck()) {
		smoothSldr->EnableWindow(TRUE);
		smoothEBox->EnableWindow(TRUE);
	}
	else {
		smoothSldr->EnableWindow(FALSE);
		smoothEBox->EnableWindow(FALSE);
	}
}




void CSetOpenNIMotion::OnCheckMvavSmooth()
{
	if (maSmthBtn->GetCheck()) {
		maTypeCombo->EnableWindow(TRUE);
		maNumCombo->EnableWindow(TRUE);
	}
	else {
		maTypeCombo->EnableWindow(FALSE);
		maNumCombo->EnableWindow(FALSE);
	}
}



///////////////////////////////////////////////////////////////////////////////////
//
afx_msg LRESULT CSetOpenNIMotion::OnSpeechEvent(WPARAM wParam, LPARAM lParam)
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




BOOL CSetOpenNIMotion::DoLocalTerminateAction(LPCTSTR com)
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





//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
// CSetKinectMotion ダイアログ
//
// for Kinect SDK
//

IMPLEMENT_DYNAMIC(CSetKinectMotion, CSetMotion)

CSetKinectMotion::CSetKinectMotion(CParameterSet prm, CWnd* pParent /*=NULL*/)
	: CSetMotion(prm, CSetKinectMotion::IDD, pParent)
{
//	posButton	= NULL;
//	rotButton	= NULL;

	jointCBox	= NULL;

	maSmthBtn	= NULL;
	maTypeCombo	= NULL;
	maNumCombo	= NULL;
	
	knctSmthBtn = NULL;
	smoothSldr	= NULL;
	smoothEBox	= NULL;
	crrctnSldr	= NULL;
	crrctnEBox	= NULL;

	YaxisSldr	= NULL;
	YaxisEBox	= NULL;

	partsCombo	= NULL;
	faceCBox	= NULL;

	correction  = param.correction;
	smoothKNCT  = param.smoothKNCT;
	YaxisCrrct  = (float)(param.YaxisCorrect/PI*180.);

	if      (correction<0.0) correction = 0.0f;
	else if (correction>1.0) correction = 1.0f;

	if      (smoothKNCT<0.0) smoothKNCT = 0.0f;
	else if (smoothKNCT>1.0) smoothKNCT = 1.0f;

	if      (YaxisCrrct<-15.0) YaxisCrrct = -15.0f;
	else if (YaxisCrrct> 15.0) YaxisCrrct =  15.0f;
}



void CSetKinectMotion::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSetKinectMotion, CDialog)
	ON_EN_CHANGE(IDC_EDIT_CRRCTN, OnChangeCorrection)
	ON_EN_CHANGE(IDC_EDIT_SMOOTH, OnChangeSmooth)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_CHECK_KINECT_SMOOTH, OnCheckKinectSmooth)
	ON_BN_CLICKED(IDC_CHECK_MVAV_SMOOTH, OnCheckMvavSmooth)
	ON_EN_CHANGE(IDC_EDIT_YROT, &CSetKinectMotion::OnChangeYRot)
	ON_MESSAGE(JBXWL_WM_SPEECH_EVENT, &CSetKinectMotion::OnSpeechEvent)
END_MESSAGE_MAP()




// CSetKinectMotion メッセージ ハンドラ

BOOL CSetKinectMotion::OnInitDialog()
{
	TCHAR buf[LNAME];
//	memset(buf, 0, LNAME);

	CDialog::OnInitDialog();

//	posButton	= (CButton*)GetDlgItem(IDC_RADIO_MOTION_POS);
//	rotButton	= (CButton*)GetDlgItem(IDC_RADIO_MOTION_ROT);

	jointCBox	= (CButton*)GetDlgItem(IDC_CHECK_JOINT_CONSTRAINT);

	maSmthBtn	= (CButton*)GetDlgItem(IDC_CHECK_MVAV_SMOOTH);
	maTypeCombo	= (CComboBox*)GetDlgItem(IDC_COMBO_MVAV_TYPE);
	maNumCombo	= (CComboBox*)GetDlgItem(IDC_COMBO_MVAV_NUM);

	knctSmthBtn = (CButton*)GetDlgItem(IDC_CHECK_KINECT_SMOOTH);
	crrctnSldr	= (CSliderCtrl*)GetDlgItem(IDC_SLIDER_CRRCTN);
	crrctnEBox	= (CEdit*)GetDlgItem(IDC_EDIT_CRRCTN);
	smoothSldr	= (CSliderCtrl*)GetDlgItem(IDC_SLIDER_SMOOTH);
	smoothEBox	= (CEdit*)GetDlgItem(IDC_EDIT_SMOOTH);

	YaxisSldr   = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_YROT);
	YaxisEBox	= (CEdit*)GetDlgItem(IDC_EDIT_YROT);

	partsCombo	= (CComboBox*)GetDlgItem(IDC_COMBO_DETECT);
	faceCBox	= (CButton*)GetDlgItem(IDC_CHECK_FACE_DETECT);


	if (param.useJointConst) {
		jointCBox->SetCheck(1);
	}
	else {
		jointCBox->SetCheck(0);
	}

	//
	maTypeCombo->SetCurSel(param.mvavType-1);
	maNumCombo->SetCurSel(param.mvavNum-1);

	if (param.useMvavSmooth) {
		maSmthBtn->SetCheck(1);
		maTypeCombo->EnableWindow(TRUE);
		maNumCombo->EnableWindow(TRUE);
	}
	else {
		maSmthBtn->SetCheck(0);
		maTypeCombo->EnableWindow(FALSE);
		maNumCombo->EnableWindow(FALSE);
	}

	//
	smoothSldr->SetRange(0,  100);
	smoothSldr->SetPos((int)(smoothKNCT*100));
	sntprintf(buf, LNAME, _T("%4.2f"), smoothKNCT);
	smoothEBox->SetWindowText(buf);

	//
	crrctnSldr->SetRange(0, 100);
	crrctnSldr->SetPos((int)(correction*100));
	sntprintf(buf, LNAME, _T("%4.2f"), correction);
	crrctnEBox->SetWindowText(buf);

	if (param.useKnctSmooth) {
		knctSmthBtn->SetCheck(1);
		smoothSldr->EnableWindow(TRUE);
		smoothEBox->EnableWindow(TRUE);
		crrctnSldr->EnableWindow(TRUE);
		crrctnEBox->EnableWindow(TRUE);
	}
	else {
		knctSmthBtn->SetCheck(0);
		smoothSldr->EnableWindow(FALSE);
		smoothEBox->EnableWindow(FALSE);
		crrctnSldr->EnableWindow(FALSE);
		crrctnEBox->EnableWindow(FALSE);
	}

	//
	YaxisSldr->SetRange(-150, 150);
	YaxisSldr->SetPos(100);
	YaxisSldr->SetPos((int)(YaxisCrrct*10));
	sntprintf(buf, LNAME, _T("%+4.1f"), YaxisCrrct);
	YaxisEBox->SetWindowText(buf);
	
	//
	partsCombo->SetCurSel(param.detectParts - 2);	// OpenNIに合わせる

	if (param.useFaceDetect) {
		faceCBox->SetCheck(1);
	}
	else {
		faceCBox->SetCheck(0);
	}

	return TRUE;
}



void CSetKinectMotion::OnOK()
{
	TCHAR buf[LNAME];

	param.usePosData = TRUE;
	param.useRotData = FALSE;

	if (jointCBox->GetCheck()) {
		param.useJointConst = TRUE;
	}
	else {
		param.useJointConst = FALSE;
	}

	//
	if (knctSmthBtn->GetCheck()) {
		param.useKnctSmooth = TRUE;
	}
	else {
		param.useKnctSmooth = FALSE;
	}
	//
	smoothEBox->GetWindowText(buf, LNAME);
	param.smoothKNCT = (float)ttof(buf);

	crrctnEBox->GetWindowText(buf, LNAME);
	param.correction = (float)ttof(buf);


	//
	if (maSmthBtn->GetCheck()) {
		param.useMvavSmooth = TRUE;
	}
	else {
		param.useMvavSmooth = FALSE;
	}
	param.mvavType = (NiMvAvType)(maTypeCombo->GetCurSel() + 1);
	param.mvavNum  = maNumCombo->GetCurSel() + 1;

	//
	YaxisEBox->GetWindowText(buf, LNAME);
	YaxisCrrct = (float)ttof(buf);
	if      (YaxisCrrct<-15.0) YaxisCrrct = -15.0f;
	else if (YaxisCrrct> 15.0) YaxisCrrct =  15.0f;
	param.YaxisCorrect = (float)(YaxisCrrct/180.*PI);
		
	param.detectParts = partsCombo->GetCurSel() + 2;
	if (faceCBox->GetCheck()) {
		param.useFaceDetect = TRUE;
	}
	else {
		param.useFaceDetect = FALSE;
	}

	CDialog::OnOK();
}



void CSetKinectMotion::OnChangeCorrection() 
{
	TCHAR buf[LNAME];

	crrctnEBox->GetWindowText(buf, LNAME);
	correction = (float)ttof(buf);

	if (correction<0.0 || correction>1.0) {
		if      (correction<0.0) correction = 0.0;
		else if (correction>1.0) correction = 1.0;
		sntprintf(buf, LNAME, _T("%4.2f"), correction);
		crrctnEBox->SetWindowText(buf);
		crrctnEBox->UpdateWindow();
	}

	crrctnSldr->SetPos((int)(correction*100));
}



void CSetKinectMotion::OnChangeSmooth() 
{
	TCHAR buf[LNAME];
	memset(buf, 0, LNAME);

	smoothEBox->GetWindowText(buf, LNAME);
	smoothKNCT = (float)ttof(buf);

	if (smoothKNCT<0.0 || smoothKNCT>1.0) {
		if      (smoothKNCT<0.0) smoothKNCT = 0.0;
		else if (smoothKNCT>1.0) smoothKNCT = 1.0;
		sntprintf(buf, LNAME, _T("%4.2f"), smoothKNCT);
		smoothEBox->SetWindowText(buf);
		smoothEBox->UpdateWindow();
	}

	smoothSldr->SetPos((int)(smoothKNCT*100));
}




void CSetKinectMotion::OnChangeYRot()
{
	TCHAR buf[LNAME];

	YaxisEBox->GetWindowText(buf, LNAME);
	YaxisCrrct = (float)ttof(buf);

	if (YaxisCrrct<-15.0 || YaxisCrrct>15.0) {
		if      (YaxisCrrct<-15.0) YaxisCrrct = -15.0f;
		else if (YaxisCrrct> 15.0) YaxisCrrct =  15.0f;
		sntprintf(buf, LNAME, _T("%+4.1f"), YaxisCrrct);
		YaxisEBox->SetWindowText(buf);
		YaxisEBox->UpdateWindow();
	}

	YaxisSldr->SetPos(100);
	YaxisSldr->SetPos((int)(YaxisCrrct*10));
}



void CSetKinectMotion::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	TCHAR buf[LNAME];
	memset(buf, 0, LNAME);

	if (crrctnSldr==(CSliderCtrl*)pScrollBar) {
		int cnfd = crrctnSldr->GetPos();
		sntprintf(buf, LNAME, _T("%4.2f"), cnfd/100.);
		crrctnEBox->SetWindowText(buf);
		crrctnEBox->UpdateWindow();
	}
	else if (smoothSldr==(CSliderCtrl*)pScrollBar) {
		int smth = smoothSldr->GetPos();
		sntprintf(buf, LNAME, _T("%4.2f"), smth/100.);
		smoothEBox->SetWindowText(buf);
		smoothEBox->UpdateWindow();
	}
	else if (YaxisSldr==(CSliderCtrl*)pScrollBar) {
		int yaxis = YaxisSldr->GetPos();
		sntprintf(buf, LNAME, _T("%+4.1f"), yaxis/10.);
		YaxisEBox->SetWindowText(buf);
		YaxisEBox->UpdateWindow();
	}
}




void CSetKinectMotion::OnCheckKinectSmooth()
{
	if (knctSmthBtn->GetCheck()) {
		smoothSldr->EnableWindow(TRUE);
		smoothEBox->EnableWindow(TRUE);
		crrctnSldr->EnableWindow(TRUE);
		crrctnEBox->EnableWindow(TRUE);
	}
	else {
		smoothSldr->EnableWindow(FALSE);
		smoothEBox->EnableWindow(FALSE);
		crrctnSldr->EnableWindow(FALSE);
		crrctnEBox->EnableWindow(FALSE);
	}
}




void CSetKinectMotion::OnCheckMvavSmooth()
{
	if (maSmthBtn->GetCheck()) {
		maTypeCombo->EnableWindow(TRUE);
		maNumCombo->EnableWindow(TRUE);
	}
	else {
		maTypeCombo->EnableWindow(FALSE);
		maNumCombo->EnableWindow(FALSE);
	}
}




///////////////////////////////////////////////////////////////////////////////////
//
afx_msg LRESULT CSetKinectMotion::OnSpeechEvent(WPARAM wParam, LPARAM lParam)
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




BOOL CSetKinectMotion::DoLocalTerminateAction(LPCTSTR com)
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