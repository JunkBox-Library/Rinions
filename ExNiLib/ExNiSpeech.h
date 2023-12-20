#pragma once


#include  "NiSpeech.h"



#define  RINIONS_SPEECH_GRAMAR_XML  "speech_gramar"


#define  WM_USER_SPEECH_UPDATE	WM_USER + 1100

#define  WM_USER_SPEECH_OK		WM_USER + 1110
#define  WM_USER_SPEECH_CANCEL	WM_USER + 1111

#define  WM_USER_SPEECH_START	WM_USER + 1120
#define  WM_USER_SPEECH_STOP	WM_USER + 1121
#define  WM_USER_SPEECH_RECORD	WM_USER + 1122
#define  WM_USER_SPEECH_END		WM_USER + 1123
#define  WM_USER_SPEECH_USER	WM_USER + 1124

#define  WM_USER_SPEECH_UP		WM_USER + 1125
#define  WM_USER_SPEECH_DOWN	WM_USER + 1126
#define  WM_USER_SPEECH_CENTER	WM_USER + 1127



using namespace jbxwl;



class  CExNiSpeech : public CNiSpeech
{
public:
	CExNiSpeech(void) {}
	virtual ~CExNiSpeech(void) {}



protected:
	virtual	void  map2action(LPCTSTR tag, float confd);


private:
	void	map2command(LPCTSTR tag);
};





