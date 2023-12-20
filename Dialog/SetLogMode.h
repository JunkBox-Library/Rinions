#pragma once

#include  "ParameterSet.h"



// CSetLogMode ダイアログ

class CSetLogMode : public CDialog
{
	DECLARE_DYNAMIC(CSetLogMode)

public:
	CSetLogMode(NiSDK_Lib lib, CParameterSet prm, CWnd* pParent=NULL);
	virtual ~CSetLogMode() {}

	CParameterSet getParameter(void) { return param;}

	// ダイアログ データ
	enum { IDD = IDD_SETLOGMODE };


protected:
	NiSDK_Lib	  sdk_lib; 
	CParameterSet param;

	CButton*	posCBox;
//	CButton*	rotCBox;
	CButton*	qutCBox;
	CButton*	aglCBox;

	CButton*	netCBox;
	CButton*	chkCBox;


protected:
	BOOL		DoLocalTerminateAction(LPCTSTR com);


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
protected:
	virtual void OnOK();
public:
	virtual BOOL OnInitDialog();
protected:
	afx_msg LRESULT OnSpeechEvent(WPARAM wParam, LPARAM lParam);
};
