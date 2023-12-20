#pragma once

#ifdef ENABLE_KINECT_SDK


#include  "KinectWin.h"
#include  "ExNiJoints.h"
#include  "ExNiFileDev.h"

//#include  "ExNiSpeech.h"




class  CExKinectWin : public CKinectWin
{
public:
	CExKinectWin(void);
	virtual ~CExKinectWin(void);

public:
	CExNiFileDev*	niFileDev;
	CExportNiData*	niExportData;

	CExNiJoints*	niJoints;
	CExNiNetwork*	niNetwork;
	CExNiSHMemory*	sharedMem;

	CParameterSet	appParam;

	CExTextFrame*	pLogFrame;
	CExFrame*		pSensorFrame;
	CExFrame*		pDepthFrame;
	CExFrame*		pModelFrame;
	CLogWndDoc*		pLogDoc;



public:
//	BOOL			isDetectFace; // already defined at KinectWin.h
	BOOL			dataSaving;
	BOOL			logfSaving;
	BOOL			logfSending;
	BOOL			hasAngleData;

	Vector<float>	vect_fwrd;


public:
	void	setSensorFramePtr(CExFrame* pfrm)   { pSensorFrame = pfrm;}
	void	setDepthFramePtr(CExFrame* pfrm)    { pDepthFrame  = pfrm;}
	void	setModelFramePtr(CExFrame* pfrm)    { pModelFrame  = pfrm;}
	void	setLogFramePtr(CExTextFrame* pfrm);
	void	setParameter(CParameterSet param);
	
	void	nop(void) { m_err_mesg = _T("No Operation with Kinect SDK!!");}

	BOOL	startLogTransfer(void);
	void	stopLogTransfer(void);


public:
	void	convertPos2JointsData(void);
	void	convertUpperPos2JointsData(void);


public:
//	virtual CNiSpeech* makeSpeech(void) { CExNiSpeech* spch = new CExNiSpeech(); return spch;}

	virtual void saveJointsData(void);
	virtual void loggingJointsData(void);
	
	virtual void checkGroundLevel(void) { if (appParam.useJointConst) niJoints->CheckGroundLevel(m_ground_level);}
	virtual void checkBoneLength(void)  { if (appParam.useJointConst) niJoints->CheckBoneLength();}

	virtual void convertJointsData(void);
	virtual void drawAddition(int col, int line);

	virtual void lostTrackingUser(int uid);  //   { DEBUG_INFO("LOST   TRACKING USER (%d)", uid);}
	virtual void detectTrackingUser(int uid);  // { DEBUG_INFO("DETECT TRACKING USER (%d)", uid);}
};






UINT  kinectEventLoop(LPVOID pParam);



#endif