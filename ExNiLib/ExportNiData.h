#pragma once



#include  "tools++.h"
#include  "MFCTool.h"

#include  "ExNiNetwork.h"



using namespace jbxwl;





class  CExportNiData
{
public:
	CExportNiData(void)  { init();}
	virtual ~CExportNiData(void) {}


public:
	unsigned short	outTimer;
	unsigned short	frameTimer;

	int				modTime;
	int				outPeriod;

	float			frameRate;
	int				frameNum;

	BOOL			ctrlFPS;
	BOOL			sendLogNet;


public:
	void	init(void); 
	void	clear(void);

	BOOL	exportJointsData(Vector<float>* posVect, Quaternion<float>* rotQuat, float* jntAngl, CExNiNetwork* net, NiSDK_Lib lib, int joint_num);

//	void	exportJointsLog (Vector<double> currentPos, double* jointAngle, CExNiNetwork* net, NiSDK_Lib lib, int joint_num);
	void	exportJointsLogStart(CExNiNetwork* net, NiSDK_Lib lib, int joint_num);
	void	exportJointsLogStop (CExNiNetwork* net);


public:
	BOOL	checkOutputPeriod(void);
	void	calcLocalFPS(void);
	void	exportData(Vector<float>* posVect, Quaternion<float>* rotQuat, float* jntAngl, CExNiNetwork* net, NiSDK_Lib lib, int joint_num);
};