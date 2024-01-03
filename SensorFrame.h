// CSensorFrame フレーム

#pragma once


#include "OpenNiWin.h"

#include "ExClass.h"
#include "Rinions.h"


using namespace jbxl;
using namespace jbxwl;



class CSensorFrame : public CExFrame
{
	DECLARE_DYNCREATE(CSensorFrame)
protected:
	CSensorFrame();
	virtual ~CSensorFrame();

public:
	CRinionsApp*	thisApp;


protected:
	BOOL	DoLocalEventAction(LPCTSTR com);


protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnStartButtonUpdate(CCmdUI* pCmdUI);
	afx_msg void OnStopButtonUpdate (CCmdUI* pCmdUI);
	afx_msg void OnNextButtonUpdate (CCmdUI* pCmdUI);
	afx_msg void OnCalibrationStart();
	afx_msg void OnCalibrationStop ();
	afx_msg void OnCalibrationNext ();
	afx_msg void OnRecStartButtonUpdate(CCmdUI* pCmdUI);
	afx_msg void OnRecStopButtonUpdate (CCmdUI* pCmdUI);
	afx_msg void OnRecStart();
	afx_msg void OnRecStop ();
	afx_msg void OnMotorUpButtonUpdate    (CCmdUI* pCmdUI);
	afx_msg void OnMotorCenterButtonUpdate(CCmdUI* pCmdUI);
	afx_msg void OnMotorDownButtonUpdate  (CCmdUI* pCmdUI);
	afx_msg void OnMotorUp();
	afx_msg void OnMotorCenter();
	afx_msg void OnMotorDown();
protected:
	afx_msg LRESULT OnEventSpeech(WPARAM wParam, LPARAM lParam);
};


