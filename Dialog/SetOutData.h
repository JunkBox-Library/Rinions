#pragma once


#include  "ParameterSet.h"



/*
Delete
IDD_SETOUTDATA
    CONTROL         " 自動調整 （未）",IDC_CHECK_OUT_FPS_AUTO,"Button",BS_AUTOCHECKBOX | WS_TABSTOP,16,75,57,10
    CONTROL         " Automatic adjustment (not yet)",IDC_CHECK_OUT_FPS_AUTO,
                    "Button",BS_AUTOCHECKBOX | WS_TABSTOP,16,75,115,10
*/



// CSetOutData ダイアログ

class CSetOutData : public CDialog
{
	DECLARE_DYNAMIC(CSetOutData)

public:
	CSetOutData(CParameterSet prm, CWnd* pParent=NULL);   // 標準コンストラクタ
	virtual ~CSetOutData() {}

	CParameterSet getParameter(void) { return param;}

	// ダイアログ データ
	enum { IDD = IDD_SETOUTDATA };


protected:
	CParameterSet param;
	CButton*	 cordCBox;
	CButton*	 quatCBox;

	CButton*     ctrlfpsCBox;
//	CButton*     autofpsCBox;
	CSliderCtrl* outfpsSldr;
	CEdit*		 outfpsEBox;

	int			 outfps;


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
	afx_msg void OnChangeOutFPS();
	afx_msg void OnCheckOutFPSAuto();
	afx_msg void OnCheckOutFPSCtrl();
protected:
	afx_msg LRESULT OnSpeechEvent(WPARAM wParam, LPARAM lParam);
};
