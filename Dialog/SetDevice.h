#pragma once


#include  "ExNiLib/ExNiDevice.h"
#include  "ParameterSet.h"

// CSetDevice ダイアログ




class CSetDevice : public CDialog
{
	DECLARE_DYNAMIC(CSetDevice)

public:
//	CSetDevice(NiSDK_Lib lib, CParameterSet prm, CWnd* pParent=NULL);   // 標準コンストラクタ
	CSetDevice(CExNiDevice* dev, CParameterSet prm, CWnd* pParent=NULL);   // 標準コンストラクタ
	virtual ~CSetDevice() {}

	CParameterSet getParameter(void) { return param;}

	// ダイアログ データ
	enum { IDD = IDD_SETDEVICE };


protected:
	CParameterSet param;
	CExNiDevice*	nidev;
//	NiSDK_Lib	sdk_lib;

	CEdit*		crntLibEitBox;
	CButton*	openniButton;
	CButton*	kinectButton;

	CButton*	useLEDButton;
	CButton*	useMotorButton;

protected:
	BOOL		DoLocalTerminateAction(LPCTSTR com);


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
protected:
	virtual void OnOK();
public:
protected:
	afx_msg LRESULT OnSpeechEvent(WPARAM wParam, LPARAM lParam);
};
