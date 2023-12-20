#pragma once


#include  "ParameterSet.h"



// CSetViewMode �_�C�A���O

class CSetViewMode : public CDialog
{
	DECLARE_DYNAMIC(CSetViewMode)

public:
	CSetViewMode(CParameterSet prm, NiInputDevice dev, CWnd* pParent=NULL);   // �W���R���X�g���N�^
	virtual ~CSetViewMode() {}

	CParameterSet getParameter(void) { return param;}

	// �_�C�A���O �f�[�^
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
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
protected:
	virtual void OnOK();
	afx_msg LRESULT OnSpeechEvent(WPARAM wParam, LPARAM lParam);
};
