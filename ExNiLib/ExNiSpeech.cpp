#include "StdAfx.h"

#include  "WinTools.h"
#include  "ExNiSpeech.h"




using namespace jbxwl;




void  CExNiSpeech::map2action(LPCTSTR tag, float confd)
{
	//char* str = ::ts2mbs(tag);
	//DEBUG_ERR("CExNiSpeech ====> %s, %f:%f", str, m_confidence, confd);
	//::free(str);

	if (confd>=m_confidence) {
		SendWinMessage(WM_USER_SPEECH_UPDATE, (LPARAM)tag);
		map2command(tag);
	}
	else {
		SendWinMessage(WM_USER_SPEECH_UPDATE, (LPARAM)_T("N/A"));
	}
}



void  CExNiSpeech::map2command(LPCTSTR tag)
{
	char* com = ::ts2mbs(tag);

	if      (!strcmp(com, "UP")) {
		SendKeyActionTap(VK_UP);
	}
	else if (!strcmp(com, "DOWN")) {
		SendKeyActionTap(VK_DOWN);
	}
	else if (!strcmp(com, "NEXT")) {
		SendKeyActionTap(VK_RIGHT);
	}
	else if (!strcmp(com, "BACK")) {
		SendKeyActionTap(VK_LEFT);
	}
	else if (!strcmp(com, "ENTER")) {
		SendKeyActionTap(VK_RETURN);
	}
	else if (!strcmp(com, "CHANGE")) {
		SendKeyActionTapCtrl(VK_F6);
	}
	else if (!strcmp(com, "CLOSE")) {
		SendKeyActionTapCtrl(VK_F4);
	}
	else if (!strcmp(com, "ALT")) {
		SendKeyActionTap(VK_LMENU);
	}

	//	
	else if (!strcmp(com, "OK")) {
		SendWinMessage(WM_USER_SPEECH_OK);
	}
	else if (!strcmp(com, "CANCEL")) {
		SendWinMessage(WM_USER_SPEECH_CANCEL);
	}
	else if (!strcmp(com, "START")) {
		SendWinMessage(WM_USER_SPEECH_START);
	}
	else if (!strcmp(com, "STOP")) {
		SendWinMessage(WM_USER_SPEECH_STOP);
	}
	else if (!strcmp(com, "RECORD")) {
		SendWinMessage(WM_USER_SPEECH_RECORD);
	}
	else if (!strcmp(com, "END")) {
		SendWinMessage(WM_USER_SPEECH_END);
	}
	else if (!strcmp(com, "USER")) {
		SendWinMessage(WM_USER_SPEECH_USER);
	}
	else if (!strcmp(com, "MOTER_UP")) {
		SendWinMessage(WM_USER_SPEECH_UP);
	}
	else if (!strcmp(com, "MOTER_DOWN")) {
		SendWinMessage(WM_USER_SPEECH_DOWN);
	}
	else if (!strcmp(com, "MOTER_CENTER")) {
		SendWinMessage(WM_USER_SPEECH_CENTER);
	}

	// Menu Bar
	else if (!strcmp(com, "F")) {
		SendKeyActionTapAlt('F');
	}
	else if (!strcmp(com, "W")) {
		SendKeyActionTapAlt('W');
	}
	else if (!strcmp(com, "T")) {
		SendKeyActionTapAlt('T');
	}
	else if (!strcmp(com, "H")) {
		SendKeyActionTapAlt('H');
	}

	//
	else {
		SendKeyActionTap((WORD)*com);
	}

	::free(com);

}

