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

protected:  // コントロール バー用メンバ
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

// アトリビュート
public:
	CRinionsApp*  pApp;

// オペレーション
public:

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// インプリメンテーション
public:
	virtual ~CMainFrame();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// 生成されたメッセージ マップ関数
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnInitMenu(CMenu* pMenu);
	afx_msg void OnViewToolbar();
		// メモ - ClassWizard はこの位置にメンバ関数を追加または削除します。
		//        この位置に生成されるコードを編集しないでください。
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
