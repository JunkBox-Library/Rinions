// CDepthFrame フレーム

#pragma once


#include "OpenNiWin.h"

#include "ExClass.h"
#include "Rinions.h"


using namespace jbxl;
using namespace jbxwl;



class CDepthFrame : public CExFrame
{
	DECLARE_DYNCREATE(CDepthFrame)
protected:
	CDepthFrame();
	virtual ~CDepthFrame();

public:
	CRinionsApp*	thisApp;


protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnContrastButtonUpdate(CCmdUI* pCmdUI);
	afx_msg void OnContrast();

};


