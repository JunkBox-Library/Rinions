#pragma once


#include  "SHMIF/SHMJoints.h"
#include  "Rotation.h"

#include  "ParameterSet.h"



#define	  NISHM_WM_LCLFPS_UPDATE		JBXWL_WM_USER + 200





using namespace jbxl;





class CExNiSHMemory
{
public:
	CExNiSHMemory(void);
	virtual ~CExNiSHMemory(void);


public:
	HANDLE	indexHandle;
	char*	ptrAnimIndex;

	HANDLE	mapHandle [SHMIF_JOINT_NUM];
	double*	ptrMapData[SHMIF_JOINT_NUM];

	BOOL	isCreated;
	BOOL	isTracking;


public:
	CParameterSet appParam;
	void	setParameter(CParameterSet param) { appParam = param;}


public:

	BOOL	createSharedMemory(void);
	void	closeSharedMemory (void);
	void	clearSharedMemory (void);

	void	setAnimationIndex(char* uuid, int n=0);

	void	setLocalAnimationIndex(void);
	void	clearLocalAnimationIndex(void);

	int		getNetworkAnimationIndexNum(void);
	int		checkNetworkAnimationIndex(char* uuid);
	int		clearNetworkAnimationIndex(char* uuid);
	void	clearNetworkAnimationIndex(void);

	void    updateLocalAnimationData(Vector<float>* posVect, Quaternion<float>* rotQuat, NiSDK_Lib lib, int joints_num);
	
	void	clearLocalAnimationData(void);
	void	clearNetworkAnimationData(void);
	void	clearAnimationData(void);

	// Index + Data
//	void	updateNetworkAnimation(char* ptr, char* uuid, int joints, int size);
	void	updateNetworkAnimation(char* ptr, char* uuid, int joints);
	void	clearNetworkAnimation(char* uuid);
};



