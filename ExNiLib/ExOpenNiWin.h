#pragma once


#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)


#include  "OpenNiWin.h"
#include  "ExClass.h"
#include  "ExNiJoints.h"
#include  "ExNiFileDev.h"




#define		OPENNI_RECORDE_TEMP_FILE	".openni_recorde.oni.$$$"



using namespace jbxwl;





class  CExOpenNiWin : public COpenNiWin
{
public:
	CExOpenNiWin(NiSDK_Lib lib);
	virtual ~CExOpenNiWin(void);


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
	BOOL			isDetectFace;
	BOOL			dataSaving;
	BOOL			logfSaving;
	BOOL			logfSending;
	BOOL			hasAngleData;

	Vector<double>	vect_fwrd;


public:
	CString	tempFilePath;
	CString	makeTempFilePath(void);
	void    deleteRecordeTempFile(void) { tunlink(tempFilePath);}

	void	setSensorFramePtr(CExFrame* pfrm) { pSensorFrame = pfrm;}
	void	setDepthFramePtr(CExFrame* pfrm)  { pDepthFrame  = pfrm;}
	void	setModelFramePtr(CExFrame* pfrm)  { pModelFrame  = pfrm;}
	void	setLogFramePtr(CExTextFrame* pfrm);
	void	setParameter(CParameterSet param);

	BOOL	makeFileDevice  (LPCTSTR fname);
	BOOL	remakeFileDevice(LPCTSTR fname);

//	BOOL	startRecord(void) { dataSaving = TRUE;}
//	BOOL	stopRecord(void)  { dataSaving = FALSE;}

	BOOL	startLogTransfer(void);
	void	stopLogTransfer(void);


public:
	void	convertRot2JointsData(void);
	void	convertPos2JointsData(void);
	void	convertUpperPos2JointsData(void);


public:
	virtual void saveJointsData(void);
	virtual void loggingJointsData(void);

	virtual void checkGroundLevel(void) { if (appParam.useJointConst) niJoints->CheckGroundLevel(m_ground_level);}
	virtual void checkBoneLength (void) { if (appParam.useJointConst) niJoints->CheckBoneLength();}

	virtual void convertJointsData(void);
	virtual void drawAddition(int col, int line);
	
	virtual void lostTrackingUser(int uid);  // { DEBUG_INFO("LOST   TRACKING USER (%d)", uid);}
	virtual void detectTrackingUser(int uid);// { DEBUG_INFO("DETECT TRACKING USER (%d)", uid);}
	

public:
	// TEST
	bool	setStartBoneLength(void);

	double	stbnLen[OPENNI_JOINT_NUM];
};






////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 

UINT  openniEventLoop(LPVOID pParam);


#endif