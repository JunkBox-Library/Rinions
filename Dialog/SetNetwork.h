#pragma once

#include  "ParameterSet.h"


// CSetNetwork ダイアログ



/*
Delete
IDD_SETNETWORK
　　CONTROL         " 自動調整  （未）",IDC_CHECK_AUTO_IN_FPS,"Button",BS_AUTOCHECKBOX | WS_TABSTOP,20,115,57,10
    CONTROL         " Automatic adjustment (not yet)",IDC_CHECK_AUTO_IN_FPS,
                    "Button",BS_AUTOCHECKBOX | WS_TABSTOP,20,115,111,10
*/


class CSetNetwork : public CDialog
{
	DECLARE_DYNAMIC(CSetNetwork)

public:
	CSetNetwork(CParameterSet prm, BOOL login, CWnd* pParent=NULL);   // 標準コンストラクタ
	virtual ~CSetNetwork() {}

	CParameterSet getParameter(void) { return param;}

	// ダイアログ データ
	enum { IDD = IDD_SETNETWORK };


protected:
	CParameterSet param;

	BOOL		isLogin;

	CEdit*		userNameEBox;

	CButton*	outputNetMemButton;
	CButton*	outputNetworkButton;
	CButton*	outputNetLogCBox;
//	CButton*	fastNetworkCBox;
	
	CEdit*		animServerEBox;
	CEdit*		serverPortEBox;
	CEdit*		clientPortEBox;
	CEdit*		groupKeyEBox;

//	CButton*     autobpsCBox;
	CSliderCtrl* maxbpsSldr;
	CEdit*		 maxbpsEBox;

	int			 MaxBPS;
	int			 maxbps;


protected:
	BOOL		 DoLocalTerminateAction(LPCTSTR com);


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
protected:
	virtual void OnOK();
public:
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnChangeInFPS();
	afx_msg void OnCheckAutoInFPS();
protected:
	afx_msg LRESULT OnSpeechEvent(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnBnClickedRadioNetwork();
	afx_msg void OnBnClickedRadioNetSmem();
};
