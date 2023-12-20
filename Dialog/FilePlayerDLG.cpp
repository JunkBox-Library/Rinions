// FilePlayerDLG.cpp : 実装ファイル
//

#include "stdafx.h"

#include "resource.h"
#include "FilePlayerDLG.h"


// CFilePlayerDLG ダイアログ


IMPLEMENT_DYNAMIC(CFilePlayerDLG, CDialog)

CFilePlayerDLG::CFilePlayerDLG(CExNiDevice* dev, BOOL lgn, FileDevParam param, CString fn, CWnd* pParent)
	: CDialog(CFilePlayerDLG::IDD, pParent)
{
	//
	dev_param = param;
	lap_param = param;
	lap_param.start_frame = 0;
	lap_param.controler   = &control;
	lap_param.counter     = NULL;

	fname		= get_file_name_t(fn);

	stop_frame  = 0;
	control		= NI_FILE_PLAYER_START;
	playing		= FALSE;
	pause		= FALSE;

	nidev		= dev;
	login		= lgn;

//	divTime		= nidev->appParam.saveDivTime;
//	szScale		= nidev->appParam.saveSzScale;
	divTime     = 0;
	szScale		= 1.0f;
	lengthUnit  = 1.0f;

	playButton  = NULL;
	stopButton  = NULL;
	pauseButton = NULL;
	exitButton  = NULL;

	reptButton	= NULL;
	calcButton	= NULL;
	mirrButton	= NULL;
	posButton	= NULL;

	meterButton	= NULL;
	centiButton	= NULL;
	inchButton	= NULL;
	szScaleEBox	= NULL;

	tmScaleSldr = NULL;
	tmScaleEBox = NULL;

	allfrmEBox  = NULL;
	plytmEBox   = NULL;
	frameEBox   = NULL;
	stateEBox	= NULL;

	bvhFmtCombo = NULL;
	bvhFPSCombo = NULL;
	divTimeEBox	= NULL;
}



void CFilePlayerDLG::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}




BEGIN_MESSAGE_MAP(CFilePlayerDLG, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_FLPLAYER_PLAY,  &CFilePlayerDLG::OnButtonPlay)
	ON_BN_CLICKED(IDC_BUTTON_FLPLAYER_STOP,  &CFilePlayerDLG::OnButtonStop)
	ON_BN_CLICKED(IDC_BUTTON_FLPLAYER_PAUSE, &CFilePlayerDLG::OnButtonPause)
	ON_BN_CLICKED(IDC_BUTTON_FLPLAYER_EXIT,  &CFilePlayerDLG::OnButtonExit)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_CHECK_FLPLAYER_REPEAT,   &CFilePlayerDLG::OnCheckLoop)
	ON_BN_CLICKED(IDC_CHECK_FLPLAYER_CALCQUAT, &CFilePlayerDLG::OnCheckQuat)
	ON_BN_CLICKED(IDC_CHECK_FLPLAYER_INITPOS,  &CFilePlayerDLG::OnCheckInitpos)
	ON_BN_CLICKED(IDC_CHECK_FLPLAYER_MIRROR,   &CFilePlayerDLG::OnCheckMirror)
	ON_EN_CHANGE(IDC_EDIT_FLPLAYER_TIMESCALE,  &CFilePlayerDLG::OnChangeTimeScale)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUTTON_FLPLAYER_SAVE, &CFilePlayerDLG::OnButtonSave)
	ON_MESSAGE(JBXWL_WM_SPEECH_EVENT, &CFilePlayerDLG::OnSpeechEvent)
END_MESSAGE_MAP()




void CFilePlayerDLG::terminate(void)
{
	control = NI_FILE_PLAYER_STOP;

	::Sleep(NIDEVICE_WAIT_TIME);
}



BOOL CFilePlayerDLG::OnInitDialog()
{
	TCHAR buf[LNAME];

	CDialog::OnInitDialog();

	playButton  = (CButton*)GetDlgItem(IDC_BUTTON_FLPLAYER_PLAY);
	stopButton  = (CButton*)GetDlgItem(IDC_BUTTON_FLPLAYER_STOP);
	pauseButton = (CButton*)GetDlgItem(IDC_BUTTON_FLPLAYER_PAUSE);
	exitButton  = (CButton*)GetDlgItem(IDC_BUTTON_FLPLAYER_EXIT);

	reptButton	= (CButton*)GetDlgItem(IDC_CHECK_FLPLAYER_REPEAT);
	calcButton	= (CButton*)GetDlgItem(IDC_CHECK_FLPLAYER_CALCQUAT);
	mirrButton	= (CButton*)GetDlgItem(IDC_CHECK_FLPLAYER_MIRROR);
	posButton	= (CButton*)GetDlgItem(IDC_CHECK_FLPLAYER_INITPOS);

	meterButton = (CButton*)GetDlgItem(IDC_RADIO_FLPLAYER_MT);
	centiButton = (CButton*)GetDlgItem(IDC_RADIO_FLPLAYER_CM);
	inchButton  = (CButton*)GetDlgItem(IDC_RADIO_FLPLAYER_INCH);
	szScaleEBox = (CEdit*)GetDlgItem(IDC_EDIT_SAVE_SZSCALE);

	tmScaleSldr = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_FLPLAYER_TIMESCALE);
	tmScaleEBox = (CEdit*)GetDlgItem(IDC_EDIT_FLPLAYER_TIMESCALE);

	allfrmEBox  = (CEdit*)GetDlgItem(IDC_INFO_FLPLAYER_ALLFRAME);
	plytmEBox   = (CEdit*)GetDlgItem(IDC_INFO_FLPLAYER_ALLTIME);
	frameEBox   = (CEdit*)GetDlgItem(IDC_INFO_FLPLAYER_FRAME);
	stateEBox   = (CEdit*)GetDlgItem(IDC_INFO_FLPLAYER_STATE);

	// BVH
	bvhFmtCombo = (CComboBox*)GetDlgItem(IDC_COMB_SAVE_BVH_FMT);
	bvhFPSCombo = (CComboBox*)GetDlgItem(IDC_COMB_SAVE_BVH_FPS);
	divTimeEBox = (CEdit*)GetDlgItem(IDC_EDIT_SAVE_DIVTIME);

	//
	lap_param.counter = frameEBox;
	playButton->GetFocus();

	//
	if (lap_param.repeat)    reptButton->SetCheck(1);
	else                     reptButton->SetCheck(0);
	if (lap_param.calc_quat) calcButton->SetCheck(1);
	else                     calcButton->SetCheck(0);
	if (lap_param.mirroring) mirrButton->SetCheck(1);
	else                     mirrButton->SetCheck(0);
	if (lap_param.init_pos)  posButton->SetCheck(1);
	else                     posButton->SetCheck(0);


	//
	tmScaleSldr->SetRange(-100, 100);
	double pos = - log(lap_param.time_scale)/log(2.0)*10.0;
	tmScaleSldr->SetPos(100);		// for bug when pos==0
	tmScaleSldr->SetPos((int)pos);

	int val = - (int)lap_param.time_scale;
	if (val==0 && lap_param.time_scale!=0.0) {
		val = (int)(1.0/lap_param.time_scale);
	}

	//
	if (nidev->rftype==FileDataBVH) {
		meterButton->SetCheck(0);
		centiButton->SetCheck(0);
		inchButton-> SetCheck(1);
	}
	else {
		meterButton->SetCheck(1);
		centiButton->SetCheck(0);
		inchButton-> SetCheck(0);
	}

	sntprintf(buf, LNAME, _T("%6.3f"), szScale);
	szScaleEBox->SetWindowText(buf);

	//
	if (val==0 || val==-1) val = 1;
	sntprintf(buf, LNAME, _T("%d"), val);
	tmScaleEBox->SetWindowText(buf);

	// Information
	sntprintf(buf, LNAME, _T(" %d"), nidev->getFileFrameNum());
	allfrmEBox->SetWindowText(buf);

	sntprintf(buf, LNAME, _T(" %-9.3f"), nidev->getFilePlayTime()*lap_param.time_scale);
	plytmEBox->SetWindowText(buf);

	frameEBox->SetWindowText(_T(" 0"));
	stateEBox->SetWindowText(_T(" STOP"));

	stopButton->EnableWindow(FALSE);
	pauseButton->EnableWindow(FALSE);

	int fmt = nidev->appParam.saveBVHFormat;
	int fps = nidev->appParam.saveBVHFPS;
	if (fps<10) fps = 10;
	else if (fps>60) fps = 60;

	bvhFmtCombo->SetCurSel(fmt);
	bvhFPSCombo->SetCurSel(fps/10-1);

	sntprintf(buf, LNAME, _T("%d"), divTime);
	divTimeEBox->SetWindowText(buf);

	SetWindowText(_T("FilePlayer : ") + fname);
	return TRUE;
}



void CFilePlayerDLG::OnOK()
{
	// for press Enter key
}




void CFilePlayerDLG::play_file_data(void)
{
	if (lap_param.start_frame<0) {
		dev_param.start_frame = lap_param.start_frame = stop_frame;
	}
	if (lap_param.start_frame<0) return;

	TCHAR buf[LNAME];
	stateEBox->SetWindowText(_T(" PLAY"));
	sntprintf(buf, LNAME, _T(" %d"), lap_param.start_frame);
	frameEBox->SetWindowText(buf);

	//
	if (centiButton->GetCheck())     lengthUnit = 0.01f;		// 1cm   = 0.01m
	else if (inchButton->GetCheck()) lengthUnit = 0.0254f;		// 1inch = 0.0254m
	else                             lengthUnit = 1.0f;

	szScaleEBox->GetWindowText(buf, LNAME);
	szScale = (float)ttof(buf);
	divTimeEBox->GetWindowText(buf, LNAME);
	divTime = ttoi(buf);
	lap_param.size_scale = szScale*lengthUnit;

	do {
		if (lap_param.start_frame==0) {
			dev_param = lap_param;
		}
		stop_frame = nidev->execLoadedData(login, dev_param);
		if (lap_param.repeat && (control==NI_FILE_PLAYER_START || control==NI_FILE_PLAYER_GO)) lap_param.start_frame = 0;
	} while(lap_param.repeat && (control==NI_FILE_PLAYER_START || control==NI_FILE_PLAYER_GO));

	nidev->clearLocalAvgFPS();
	
	if (stop_frame<0) {
		::MessageBox(nidev->hWnd, nidev->getErrorMessage(), _T("Error"), MB_OK);
	}

	// no repeat
	if (control==NI_FILE_PLAYER_START || control==NI_FILE_PLAYER_GO) {
		control = NI_FILE_PLAYER_STOP;
	}
	playing = FALSE;
	if (!pause) {
		playButton->EnableWindow(TRUE);
		stopButton->EnableWindow(FALSE);
	}
	if (control==NI_FILE_PLAYER_STOP) {
		OnButtonStop();
	}

	return;
}



void CFilePlayerDLG::OnButtonPlay()
{
	if (playing || pause) return;

	control = NI_FILE_PLAYER_START;
	playing = TRUE;

	playButton->EnableWindow(FALSE);
	stopButton->EnableWindow(TRUE);
	pauseButton->EnableWindow(TRUE);

	play_file_data();
}



void CFilePlayerDLG::OnButtonPause()
{
	if (pause) {
		control = NI_FILE_PLAYER_GO;
		pause   = FALSE;
		playing = TRUE;
		play_file_data();
	}
	else if (playing) {
		control = NI_FILE_PLAYER_PAUSE;
		pause	= TRUE;
		playing = FALSE;
		lap_param.start_frame = -1;
		stateEBox->SetWindowText(_T(" PAUSE"));
	}
}



void CFilePlayerDLG::OnButtonStop()
{
	control = NI_FILE_PLAYER_STOP;
	pause	= FALSE;
	playing = FALSE;
	lap_param.start_frame = 0;

	stateEBox->SetWindowText(_T(" STOP"));

	playButton->EnableWindow(TRUE);
	stopButton->EnableWindow(FALSE);
	pauseButton->EnableWindow(FALSE);
}





void CFilePlayerDLG::OnClose()
{
	terminate();
	CDialog::OnClose();
}



void CFilePlayerDLG::OnButtonExit()
{
	TCHAR buf[LNAME];

	int fmt = bvhFmtCombo->GetCurSel();
	int fps = (bvhFPSCombo->GetCurSel() + 1)*10;
	if      (fps<10) fps = 10;
	else if (fps>60) fps = 60;

	szScaleEBox->GetWindowText(buf, LNAME);
	szScale = (float)ttof(buf);
	divTimeEBox->GetWindowText(buf, LNAME);
	divTime = ttoi(buf);

	nidev->appParam.saveBVHFormat = fmt;
	nidev->appParam.saveBVHFPS    = fps;
	//nidev->appParam.saveDivTime   = divTime;
	//nidev->appParam.saveSzScale   = szScale;

	terminate();
	CDialog::OnOK();
}



void CFilePlayerDLG::OnCheckQuat()
{
	if (calcButton->GetCheck()) lap_param.calc_quat = TRUE;
	else                        lap_param.calc_quat = FALSE;
}



void CFilePlayerDLG::OnCheckLoop()
{
	if (reptButton->GetCheck()) lap_param.repeat = TRUE;
	else                        lap_param.repeat = FALSE;
}



void CFilePlayerDLG::OnCheckMirror()
{
	if (mirrButton->GetCheck()) lap_param.mirroring = TRUE;
	else                        lap_param.mirroring = FALSE;
}



void CFilePlayerDLG::OnCheckInitpos()
{
	if (posButton->GetCheck()) lap_param.init_pos = TRUE;
	else                       lap_param.init_pos = FALSE;
}




void CFilePlayerDLG::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	TCHAR buf[LNAME];

	if (tmScaleSldr==(CSliderCtrl*)pScrollBar) {
		int pos = tmScaleSldr->GetPos();
		lap_param.time_scale = (float)(power(2.0f, -pos/10.0f));

		int val = - (int)lap_param.time_scale;
		if (val==0 && lap_param.time_scale!=0.0) {
			val = (int)(1.0/lap_param.time_scale);
		}
		if (val==0 || val==-1) val = 1;
		sntprintf(buf, LNAME, _T("%d"), val);
		tmScaleEBox->SetWindowText(buf);
		tmScaleEBox->UpdateWindow();

		sntprintf(buf, LNAME, _T(" %-9.3f"), nidev->getFilePlayTime()*lap_param.time_scale);
		plytmEBox->SetWindowText(buf);
	}
}



void CFilePlayerDLG::OnChangeTimeScale()
{
	TCHAR buf[LNAME];

	tmScaleEBox->GetWindowText(buf, LNAME);
	int val = ttoi(buf);

	if (val>1024 || val<-1024) {
		if      (val>1024)  val = 1024;
		else if (val<-1024) val = -1024;
		sntprintf(buf, LNAME, _T("%d"), val);
		tmScaleEBox->SetWindowText(buf);
		tmScaleEBox->UpdateWindow();
	}

	if (val==0) val = 1;
	if (val>0) {
		lap_param.time_scale = 1.0f/val;
	}
	else {
		lap_param.time_scale = - (float)val;
	}

	int pos = - (int)(log(lap_param.time_scale)/log(2.0)*10.0);
	tmScaleSldr->SetPos((int)pos);

	sntprintf(buf, LNAME, _T(" %-9.3f"), nidev->getFilePlayTime()*lap_param.time_scale);
	plytmEBox->SetWindowText(buf);
}




void CFilePlayerDLG::OnButtonSave()
{
	BVHSaveParam param;
	TCHAR buf[LNAME];

	int fmt =bvhFmtCombo->GetCurSel();
	int fps =(bvhFPSCombo->GetCurSel() + 1)*10;

	szScaleEBox->GetWindowText(buf, LNAME);
	szScale = (float)ttof(buf);
	divTimeEBox->GetWindowText(buf, LNAME);
	divTime = ttoi(buf);

	param.format = fmt;
	param.fps    = fps;
	param.divtm  = divTime;
	param.scale  = szScale;
	param.recalc = (BOOL)calcButton->GetCheck();

	if (nidev!=NULL) {
		//
		CString mesg  = LoadString_byID(IDS_STR_SPECIFY_SAVE_FILE);
		CString fname = EasyGetSaveFileName((LPCTSTR)mesg, _T(""), m_hWnd);
		if (!fname.IsEmpty()) {
			nidev->saveFramesDataAsBVH(fname, param);
		}
	}
}





///////////////////////////////////////////////////////////////////////////////////
//
afx_msg LRESULT CFilePlayerDLG::OnSpeechEvent(WPARAM wParam, LPARAM lParam)
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




BOOL CFilePlayerDLG::DoLocalTerminateAction(LPCTSTR com)
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