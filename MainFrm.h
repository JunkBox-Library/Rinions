#pragma once

#pragma warning(disable:4995)


#include "ExToolBar.h"


class CRinionsApp;


using namespace jbxwl;



class CMainFrame : public CMDIFrameWnd
{
	DECLARE_DYNAMIC(CMainFrame)

public:
	CMainFrame();

protected:  // �R���g���[�� �o�[�p�����o
	CStatusBar  m_wndStatusBar;
	CExToolBar  m_wndToolBar;

public:
	void  SetStausBarText(CString mesg);		// 0
	void  SetStausBarLocalFPS(float fps);		// 1
	void  SetStausBarNetworkFPS(float fps);		// 2
	void  SetStausBarNetworkBPS(float bps);		// 3
	void  SetStausBarSpeech(LPCTSTR tstr) { m_wndStatusBar.SetPaneText(4, tstr);}	// 4

	BOOL  DoLocalMenuKeyAction(LPCTSTR com);
	//
	RECT  windowSize;

// �A�g���r���[�g
public:
	CRinionsApp*  pApp;

// �I�y���[�V����
public:

// �I�[�o�[���C�h
	// ClassWizard �͉��z�֐��̃I�[�o�[���C�h�𐶐����܂��B
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// �C���v�������e�[�V����
public:
	virtual ~CMainFrame();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// �������ꂽ���b�Z�[�W �}�b�v�֐�
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnInitMenu(CMenu* pMenu);
	afx_msg void OnViewToolbar();
		// ���� - ClassWizard �͂��̈ʒu�Ƀ����o�֐���ǉ��܂��͍폜���܂��B
		//        ���̈ʒu�ɐ��������R�[�h��ҏW���Ȃ��ł��������B
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMove(int x, int y);
	afx_msg LRESULT OnUpdateLocalFPS(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUpdateNetworkSpeed(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCloseNetwork(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEventSpeech(WPARAM wParam, LPARAM lParam);
};





/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
