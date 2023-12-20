// MainFrm.cpp : CMainFrame クラスの動作の定義を行います。
//

#include "stdafx.h"

#include "MainFrm.h"
#include "Rinions.h"

#include "ExNiLib/ExNiNetwork.h"



using namespace jbxl;
using namespace jbxwl;



 // ステータス ライン インジケータ
static UINT indicators[] =
{
	IDS_STR_STATUSBAR_INIT,
	IDS_STR_STATUSBAR_LCLFPS,
	IDS_STR_STATUSBAR_NETFPS,
	IDS_STR_STATUSBAR_NETBPS,
	IDS_STR_STATUSBAR_SPEECH,
	//ID_SEPARATOR,
	//ID_INDICATOR_CAPS,
	//ID_SEPARATOR,          
	//ID_INDICATOR_CAPS,
	//ID_INDICATOR_NUM,
	//ID_INDICATOR_SCRL,
};



/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
	ON_WM_INITMENU()
	ON_WM_SIZE()
	ON_WM_MOVE()
	ON_MESSAGE(NISHM_WM_LCLFPS_UPDATE,  OnUpdateLocalFPS)
	ON_MESSAGE(NINET_WM_NETFBPS_UPDATE, OnUpdateNetworkSpeed)
	ON_MESSAGE(JBXWL_WM_NETWORK_CLOSE,  OnCloseNetwork)
	ON_MESSAGE(JBXWL_WM_SPEECH_EVENT,   OnEventSpeech)
END_MESSAGE_MAP()




/////////////////////////////////////////////////////////////////////////////
// Message

LRESULT CMainFrame::OnUpdateLocalFPS(WPARAM wParam, LPARAM lParam)
{
	if (pApp!=NULL && pApp->niDevice!=NULL) {
		SetStausBarLocalFPS(pApp->niDevice->getLocalAvgFPS());
	}

	return	TRUE;
}




LRESULT CMainFrame::OnUpdateNetworkSpeed(WPARAM wParam, LPARAM lParam)
{
	if (pApp!=NULL && pApp->niNetwork!=NULL) {
		SetStausBarNetworkFPS(pApp->niNetwork->getNetFPS());
		SetStausBarNetworkBPS(pApp->niNetwork->getNetBPS());
	}

	return	TRUE;
}



LRESULT CMainFrame::OnCloseNetwork(WPARAM wParam, LPARAM lParam)
{
	if (pApp!=NULL) {
		pApp->closeNetwork(FALSE);
	}

	return	TRUE;
}




/////////////////////////////////////////////////////////////////////////////
//
LRESULT CMainFrame::OnEventSpeech(WPARAM wParam, LPARAM lParam)
{
	if (pApp!=NULL && pApp->niDevice!=NULL) {
		if (lParam!=NULL) {
			SetStausBarSpeech((LPCTSTR)lParam);

			//DEBUG_INFO("CMainFrame::OnEventSpeech");
			/*
			CWnd* wnd = GetForegroundWindow();
			CWnd* pnt = wnd->GetParent();
			HWND  hpn = NULL;
			if (pnt!=NULL) hpn = pnt->m_hWnd;
			DEBUG_INFO("m_hWnd=%08x : F=%08x, A=%08x, FP=%08x", m_hWnd, wnd->m_hWnd, GetActiveWindow( )->m_hWnd, hpn);
			*/

			if (wParam!=NULL && *(WORD*)wParam) {
				BOOL  ret = FALSE;
				//
				CWnd* wnd = GetForegroundWindow();	// GetActiveWindow(), GetLastActivePopup()
				if (wnd!=NULL) {
					if (wnd->m_hWnd==m_hWnd) {	// 自ウィンドウ
						ret = DoSystemKeyAction((LPCTSTR)lParam);
						if (!ret) ret = DoLocalMenuKeyAction((LPCTSTR)lParam);
					}
					else {
						CWnd* pnt = wnd->GetParent();
						if (pnt!=NULL && pnt->m_hWnd==m_hWnd) {	// モーダルダイアログ
							wnd->SendMessage(JBXWL_WM_SPEECH_EVENT, wParam, lParam);
							::Sleep(30);			// 処理が完了するまで待つ？
							ret = TRUE;
						}
					}
				}

				if (!ret) {
					// センサー（カメラ）ウィンドウ
					if (pApp->pSensorView!=NULL && pApp->pSensorView->pFrame!=NULL) {
						CWnd* snsr = FromHandle(pApp->pSensorView->pFrame->m_hWnd);
						if (snsr!=NULL) {
							snsr->SendMessage(JBXWL_WM_SPEECH_EVENT, wParam, lParam);
							::Sleep(30);	// 処理が完了するまで待つ？
						}
					}
				}
				//
			}
		}
	}

	return	TRUE;
}




// 1 string command
BOOL  CMainFrame::DoLocalMenuKeyAction(LPCTSTR com)
{
	BOOL ret = TRUE;

	// Main Menu
	if      (!tcscmp(com, _T("F"))) {	// File
		SendKeyActionTapAlt('F');
	}
	else if (!tcscmp(com, _T("W"))) {	// Window
		SendKeyActionTapAlt('W');
	}
	else if (!tcscmp(com, _T("T"))) {	// Setting
		SendKeyActionTapAlt('T');
	}
	else if (!tcscmp(com, _T("H"))) {	// Help
		SendKeyActionTapAlt('H');
	}

	// Sub Menu 
	else {
		char* pp = ::ts2mbs(com);
		if (strlen(pp)==1) {
			SendKeyActionTap((WORD)*pp);
		}
		else ret = FALSE;
		::free(pp);
	}

	return ret;
}





/////////////////////////////////////////////////////////////////////////////
// CMainFrame クラスの構築/消滅

CMainFrame::CMainFrame()
{
	pApp = NULL;
	memset(&windowSize, 0, sizeof(RECT));

	// メニューの有効，無効の切り替えを可能にする
	m_bAutoMenuEnable = FALSE;
}




CMainFrame::~CMainFrame()
{
	DEBUG_INFO("DESTRUCTOR: CMainFrame: START");

	if (pApp!=NULL) {
		pApp->deleteLockFile();
		if (pApp->pMainFrame!=NULL) {
			//pApp->appParam.saveConfigFile(pApp->pMainFrame->windowSize);
			pApp->appParam.saveWindowSize(pApp->pMainFrame->windowSize);
			pApp->pMainFrame = NULL;
		}
	}
	DEBUG_INFO("DESTRUCTOR: CMainFrame: END");
}




int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1) return -1;
	
	if (!m_wndStatusBar.Create(this))
	{
		DEBUG_INFO("CMainFrame::OnCreate(): ERROR: Failed to create status bar");
		return -1;      // 作成に失敗
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));

	return 0;
}




BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWnd::PreCreateWindow(cs) ) return FALSE;

	cs.style |= WS_CLIPCHILDREN;
	cs.style &= ~(FWS_PREFIXTITLE | FWS_ADDTOTITLE); 
	cs.lpszName = _T(RINIONS_VERSION); 

	return TRUE;
}





/////////////////////////////////////////////////////////////////////////////
// ステータスバーへの設定

void  CMainFrame::SetStausBarText(CString mesg)
{
	m_wndStatusBar.SetPaneText(0, mesg);
	return;
}



void  CMainFrame::SetStausBarLocalFPS(float fps)
{
	TCHAR buf[32];
	sntprintf(buf, 32, _T("Local FPS: %-4.1f"), fps);
	m_wndStatusBar.SetPaneText(1, (LPCTSTR)buf);
	return;
}



void  CMainFrame::SetStausBarNetworkFPS(float fps)
{
	TCHAR buf[32];
	sntprintf(buf, 32, _T("Net FPS: %-4.1f"), fps);
	m_wndStatusBar.SetPaneText(2,(LPCTSTR)buf);
	return;
}



void  CMainFrame::SetStausBarNetworkBPS(float bps)
{
	TCHAR buf[32];
	sntprintf(buf, 32, _T("Net: %-4.1fkbps"), bps);
	m_wndStatusBar.SetPaneText(3, (LPCTSTR)buf);
	return;
}





/////////////////////////////////////////////////////////////////////////////
// CMainFrame クラスの診断

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}



void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG






/////////////////////////////////////////////////////////////////////////////
// CMainFrame メッセージ ハンドラ

void  CMainFrame::OnInitMenu(CMenu* pMenu)
{
	if (pApp!=NULL) pApp->updateMenuBar(pMenu);

	CMDIFrameWnd::OnInitMenu(pMenu);
}




void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
	CMDIFrameWnd::OnSize(nType, cx, cy);

	GetWindowRect(&windowSize);
}




void CMainFrame::OnMove(int x, int y)
{
	CMDIFrameWnd::OnMove(x, y);

	GetWindowRect(&windowSize);
}
