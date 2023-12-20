// CameraView.cpp : 実装ファイル
//

#include "stdafx.h"

#include "Rinions.h"
#include "CameraView.h"


// CCameraView

IMPLEMENT_DYNCREATE(CCameraView, CView)


CCameraView::CCameraView()
{
	m_lock	  = FALSE;

	m_thread  = NULL;
	m_handler = NULL;
}



CCameraView::~CCameraView()
{
	DEBUG_INFO("DESTRUCTOR: CCameraView: START");

	if (pFrame!=NULL) pFrame->pView = NULL;

	DEBUG_INFO("DESTRUCTOR: CCameraView: Wait Thread");
	stopThread();
	
	DEBUG_INFO("DESTRUCTOR: CCameraView: Wait Unlock");
	while(m_lock) ::Sleep(100);
	FILE* fp = NULL;
	DEBUG_INFO("DESTRUCTOR: CCameraView: END");
}




void  CCameraView::stopThread(void)
{
	if (m_handler!=NULL) {
		m_handler->set();
		//
        if (m_thread!=NULL) {
//			m_handler->wait(m_thread->m_hThread, INFINITE);
			m_handler->wait(m_thread->m_hThread, 10000);
			deleteNull(m_thread);
		}
		deleteNull(m_handler);
    }
	//
	else {
        if (m_thread!=NULL) {
			deleteNull(m_thread);
		}
	}
}





//
BEGIN_MESSAGE_MAP(CCameraView, CView)
END_MESSAGE_MAP()


// CCameraView 描画

void CCameraView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: 描画コードをここに追加してください。
}



// CCameraView 診断

#ifdef _DEBUG
void CCameraView::AssertValid() const
{
	CView::AssertValid();
}

#ifndef _WIN32_WCE
void CCameraView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif
#endif //_DEBUG


// CCameraView メッセージ ハンドラー
