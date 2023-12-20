#pragma once


#include  "ParameterSet.h"



// CSetViewMode ダイアログ

class CSetViewMode : public CDialog
{
	DECLARE_DYNAMIC(CSetViewMode)

public:
	CSetViewMode(CParameterSet prm, NiInputDevice dev, CWnd* pParent=NULL);   // 標準コンストラクタ
	virtual ~CSetViewMode() {}

	CParameterSet getParameter(void) { return param;}

	// ダイアログ データ
	enum { IDD = IDD_SETVIEWMODE };


protected:
	CParameterSet param;
	NiInputDevice device;

	CButton*	mrrCBox;
	CButton*	imgCBox;
//	CButton*    pseCBox;
	CComboBox*	lineCombo;

protected:
	BOOL		 DoLocalTerminateAction(LPCTSTR com);


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
protected:
	virtual void OnOK();
	afx_msg LRESULT OnSpeechEvent(WPARAM wParam, LPARAM lParam);
};
