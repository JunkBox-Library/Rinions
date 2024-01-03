#pragma once


#include  "ParameterSet.h"




//////////////////////////////////////////////////////////////////////////////////////////
// 
// CSetMotion ダイアログ
//

class CSetMotion : public CDialog
{
	DECLARE_DYNAMIC(CSetMotion)


public:
	CSetMotion(CParameterSet prm, UINT id, CWnd* pParent=NULL);
	virtual ~CSetMotion() {}

	CParameterSet getParameter(void) { return param;}


protected:
	CParameterSet param;
};





//////////////////////////////////////////////////////////////////////////////////////////
// 
// CSetOpenNIMotion ダイアログ
//

class CSetOpenNIMotion : public CSetMotion
{
	DECLARE_DYNAMIC(CSetOpenNIMotion)

public:
	CSetOpenNIMotion(CParameterSet prm, CWnd* pParent=NULL);   // 標準コンストラクタ
	virtual ~CSetOpenNIMotion() {}

	// ダイアログ データ
	enum { IDD = IDD_SETMOTION_OPENNI };


protected:
//	CButton*	 posButton;
//	CButton*	 rotButton;

	CSliderCtrl* cnfdncSldr;
	CEdit*		 cnfdncEBox;

	CButton*	 jointCBox;

	CButton*	 niteSmthBtn;
	CSliderCtrl* smoothSldr;
	CEdit*		 smoothEBox;

	CButton*	 maSmthBtn;
	CComboBox*	 maTypeCombo;
	CComboBox*	 maNumCombo;

	CSliderCtrl* YaxisSldr;
	CEdit*		 YaxisEBox;

	CComboBox*	 partsCombo;
	CButton*	 faceCBox;

	float		 confidence;
	float		 smoothNITE;
	float		 YaxisCrrct; 


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
	afx_msg void OnChangeConfidence();
	afx_msg void OnChangeSmooth();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnCheckNiteSmooth();
	afx_msg void OnCheckMvavSmooth();
	afx_msg void OnChangeYRot();
protected:
	afx_msg LRESULT OnSpeechEvent(WPARAM wParam, LPARAM lParam);
};






//////////////////////////////////////////////////////////////////////////////////////////
// 
// CSetKinectMotion ダイアログ
//

class CSetKinectMotion : public CSetMotion
{
	DECLARE_DYNAMIC(CSetKinectMotion)

public:
	CSetKinectMotion(CParameterSet prm, CWnd* pParent=NULL);   // 標準コンストラクタ
	virtual ~CSetKinectMotion() {}

	// ダイアログ データ
	enum { IDD = IDD_SETMOTION_KINECT };


protected:
	CButton*	 jointCBox;

	CButton*	 knctSmthBtn;
	CSliderCtrl* smoothSldr;
	CEdit*		 smoothEBox;
	CSliderCtrl* crrctnSldr;
	CEdit*		 crrctnEBox;

	CButton*	 maSmthBtn;
	CComboBox*	 maTypeCombo;
	CComboBox*	 maNumCombo;

	CSliderCtrl* YaxisSldr;
	CEdit*		 YaxisEBox;

	CComboBox*	 partsCombo;
	CButton*	 faceCBox;

	float		 correction;
	float		 smoothKNCT;
	float		 YaxisCrrct; 


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
	afx_msg void OnChangeCorrection();
	afx_msg void OnChangeSmooth();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnCheckKinectSmooth();
	afx_msg void OnCheckMvavSmooth();
	afx_msg void OnChangeYRot();
protected:
	afx_msg LRESULT OnSpeechEvent(WPARAM wParam, LPARAM lParam);
};


