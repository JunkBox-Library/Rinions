#pragma once


#include  "ParameterSet.h"





// CSetSpeech ダイアログ

class CSetSpeech : public CDialog
{
	DECLARE_DYNAMIC(CSetSpeech)

public:
	CSetSpeech(NiSDK_Lib lib, CParameterSet prm, CWnd* pParent=NULL);   // 標準コンストラクタ
	virtual ~CSetSpeech() {}

	CParameterSet getParameter(void) { return param;}

	// ダイアログ データ
	enum { IDD = IDD_SETSPEECH };


protected:
	CParameterSet param;

	NiSDK_Lib	  sdk_lib; 

	CButton*	  speechCBox;
	CSliderCtrl*  confdSldr;
	CEdit*		  confdEBox;
	CComboBox*	  langCombo;

	float		  confidence;


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
	afx_msg void OnChangeSpeechConfd();
	afx_msg void OnCheckSpeechCtrl();
	afx_msg void OnNMCustomdrawSliderSpeechConfd(NMHDR *pNMHDR, LRESULT *pResult);
protected:
	afx_msg LRESULT OnSpeechEvent(WPARAM wParam, LPARAM lParam);
};
