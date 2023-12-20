#pragma once


#include  "resource.h"



// CAnimationDLG �_�C�A���O

class CAnimationDLG : public CDialog
{
	DECLARE_DYNAMIC(CAnimationDLG)

public:
	CAnimationDLG(CString uuid, CWnd* pParent=NULL);   // �W���R���X�g���N�^
	virtual ~CAnimationDLG() {}

	// �_�C�A���O �f�[�^
	enum { IDD = IDD_SETANIMATION };


public:
	CString getAnimationUUID() { return animationUUID;}


protected:
	CString		  animationUUID;
	CEdit*		  animUUIDEBox;


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g

	DECLARE_MESSAGE_MAP()
	virtual void OnOK();
public:
	virtual BOOL OnInitDialog();
};


