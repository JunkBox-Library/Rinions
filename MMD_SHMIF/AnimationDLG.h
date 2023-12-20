#pragma once


#include  "resource.h"



// CAnimationDLG ダイアログ

class CAnimationDLG : public CDialog
{
	DECLARE_DYNAMIC(CAnimationDLG)

public:
	CAnimationDLG(CString uuid, CWnd* pParent=NULL);   // 標準コンストラクタ
	virtual ~CAnimationDLG() {}

	// ダイアログ データ
	enum { IDD = IDD_SETANIMATION };


public:
	CString getAnimationUUID() { return animationUUID;}


protected:
	CString		  animationUUID;
	CEdit*		  animUUIDEBox;


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
	virtual void OnOK();
public:
	virtual BOOL OnInitDialog();
};


