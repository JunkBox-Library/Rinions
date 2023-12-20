#pragma once


#include "DxDirectView.h"
//#include "Rinions.h"


// CCameraView �r���[

class CCameraView : public CDxDirectView
{
	DECLARE_DYNCREATE(CCameraView)

protected:
	CCameraView();           // ���I�����Ŏg�p����� protected �R���X�g���N�^�[
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
	virtual void OnDraw(CDC* pDC);      // ���̃r���[��`�悷�邽�߂ɃI�[�o�[���C�h����܂��B
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	DECLARE_MESSAGE_MAP()
};


