#pragma once


#include "DxDirectView.h"
//#include "Rinions.h"


// CCameraView ビュー

class CCameraView : public CDxDirectView
{
	DECLARE_DYNCREATE(CCameraView)

protected:
	CCameraView();           // 動的生成で使用される protected コンストラクター
	virtual ~CCameraView();


public:
	BOOL		  m_lock;

	CWinThread*	  m_thread;
	EventHandler* m_handler;


public:
	void	stopThread(void);

	void	lock(void)   { m_lock = TRUE; }
	void	unlock(void) { m_lock = FALSE;}


//
public:
	virtual void OnDraw(CDC* pDC);      // このビューを描画するためにオーバーライドされます。
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	DECLARE_MESSAGE_MAP()
};


