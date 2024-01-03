#pragma once


// CSetDataSave ダイアログ

class CSetDataSave : public CDialog
{
	DECLARE_DYNAMIC(CSetDataSave)

public:
	CSetDataSave(NiSDK_Lib lib, CParameterSet prm, CWnd* pParent=NULL);   // 標準コンストラクタ
	virtual ~CSetDataSave() {}

	CParameterSet getParameter(void) { return param;}

	// ダイアログ データ
	enum { IDD = IDD_SETDATASAVE };


protected:
	CParameterSet param;

	NiSDK_Lib	sdk_lib; 

	int			divTime;
	float		szScale;

	CButton*	orgCBox;
	CButton*	oniCBox;
	CButton*	bvhCBox;
	CButton*	nsvCBox;

	CButton*	netLogCBox;
	CButton*	lclLogCBox;
	CEdit*		fldrLogEBox;
	CButton*	referButton;

	CComboBox*	bvhFmtCombo;
	CComboBox*	bvhFPSCombo;
	CEdit*		divTimeEBox;
	CEdit*		szScaleEBox;


protected:
	BOOL		DoLocalTerminateAction(LPCTSTR com);


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedRadioSaveOrg();
	afx_msg void OnBnClickedRadioSaveOni();
	afx_msg void OnBnClickedRadioSaveBvh();
	afx_msg void OnBnClickedRadioNotSave();
	virtual BOOL OnInitDialog();
protected:
	virtual void OnOK();
	afx_msg LRESULT OnSpeechEvent(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnBnClickedButtonSaveLogRefer();
	afx_msg void OnEnChangeEditSaveLogFldr();
	afx_msg void OnBnClickedCheckSaveLogLcl();
};
