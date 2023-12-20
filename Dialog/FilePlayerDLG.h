#pragma once

#include  "ExNiLib/ExNiDevice.h"


// CFilePlayerDLG ダイアログ

class CFilePlayerDLG : public CDialog
{
	DECLARE_DYNAMIC(CFilePlayerDLG)

public:
	CFilePlayerDLG(CExNiDevice* dev, BOOL lgn, FileDevParam param, CString fname, CWnd* pParent=NULL);
	virtual ~CFilePlayerDLG() {}

	FileDevParam getParameter(void) { return dev_param;}

	// ダイアログ データ
	enum { IDD = IDD_FLPLAYER_PLAYER };

protected:
	int			 stop_frame;
	int			 control;

	BOOL		 playing;
	BOOL		 pause;

	int			 divTime;
	float		 szScale;
	float		 lengthUnit;
	CString		 fname;

	FileDevParam lap_param;
	FileDevParam dev_param;
	CExNiDevice* nidev;
	BOOL		 login;

	CButton*	 playButton;
	CButton*	 stopButton;
	CButton*	 pauseButton;
	CButton*	 exitButton;

	CButton*	 reptButton;
	CButton*	 calcButton;
	CButton*	 mirrButton;
	CButton*	 posButton;

	CButton*	 meterButton;
	CButton*	 centiButton;
	CButton*	 inchButton;
	CEdit*		 szScaleEBox;

	CSliderCtrl* tmScaleSldr;
	CEdit*		 tmScaleEBox;

	CEdit*		 allfrmEBox;
	CEdit*		 plytmEBox;
	CEdit*		 frameEBox;
	CEdit*		 stateEBox;

	CComboBox*	 bvhFmtCombo;
	CComboBox*	 bvhFPSCombo;
	CEdit*		 divTimeEBox;


public:
	void		 play_file_data(void);
	void		 terminate(void);


protected:
	BOOL		 DoLocalTerminateAction(LPCTSTR com);


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
protected:
	virtual void OnOK();
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonPlay();
	afx_msg void OnButtonStop();
	afx_msg void OnButtonPause();
	afx_msg void OnClose();
	afx_msg void OnCheckLoop();
	afx_msg void OnButtonExit();
	afx_msg void OnCheckQuat();
	afx_msg void OnChangeTimeScale();
	afx_msg void OnCheckInitpos();
	afx_msg void OnCheckMirror();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnButtonSave();
protected:
	afx_msg LRESULT OnSpeechEvent(WPARAM wParam, LPARAM lParam);
};
