// DepthFrame.cpp : 実装ファイル
//

#include  "stdafx.h"
#include  "DepthFrame.h"
#include  "MessageBoxDLG.h"

#include  "ContrastDLG.h"



using namespace jbxl;
using namespace jbxwl;



// CDepthFrame

IMPLEMENT_DYNCREATE(CDepthFrame, CExFrame)

CDepthFrame::CDepthFrame()
{
	thisApp = NULL;
}




CDepthFrame::~CDepthFrame()
{
	DEBUG_INFO("DESTRUCTOR: CDepthFrame");
}




BEGIN_MESSAGE_MAP(CDepthFrame, CExFrame)
	ON_UPDATE_COMMAND_UI(ID_DEPTH_CONTRAST, &CDepthFrame::OnContrastButtonUpdate)
	ON_COMMAND(ID_DEPTH_CONTRAST, OnContrast)
	ON_WM_CREATE()
END_MESSAGE_MAP()




// CDepthFrame メッセージ ハンドラ

int  CDepthFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CExFrame::OnCreate(lpCreateStruct) == -1) return -1;

	toolBar = new CExToolBar();
	if (!toolBar->CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP 
			| CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC | WS_CLIPCHILDREN) ||
		!toolBar->LoadToolBar(IDR_DEPTH_TOOLBAR))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;
	}

	return 0;
}






/////////////////////////////////////////////////////////////////////////////////////////
//
// ToolBar Click
//

void  CDepthFrame::OnContrast()
{
	CContrastDLG* cntDLG = new CContrastDLG(pView, FALSE);
	if (cntDLG!=NULL) {
		cntDLG->DoModal();
		delete(cntDLG);
	}
}




//////////////////////////////////////////////////////////////////////////////////
// Button Enable

void  CDepthFrame::OnContrastButtonUpdate(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);

	return;
}




