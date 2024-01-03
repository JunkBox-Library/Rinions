#pragma once


#include  "ParameterSet.h"



// CSetAnimation ダイアログ

class CSetAnimation : public CDialog
{
	DECLARE_DYNAMIC(CSetAnimation)

public:
	CSetAnimation(CParameterSet prm, BOOL login=FALSE, CWnd* pParent=NULL);   // 標準コンストラクタ
	virtual ~CSetAnimation() {}

	CParameterSet getParameter(void) { return param;}

	// ダイアログ データ
	enum { IDD = IDD_SETANIMATION };


protected:
	CParameterSet param; 
	BOOL		  isLogin;

	CEdit*		  animUUIDEBox;


protected:
	BOOL		 DoLocalTerminateAction(LPCTSTR com);


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
	virtual void OnOK();
public:
	virtual BOOL OnInitDialog();
protected:
	afx_msg LRESULT OnSpeechEvent(WPARAM wParam, LPARAM lParam);
};


