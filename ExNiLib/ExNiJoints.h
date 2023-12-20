#pragma once


#include  "Vector.h"
#include  "Rotation.h"
#include  "RingBuffer.h"
#include  "ExGdata.h"

#include  "ParameterSet.h"
#include  "NiDevice.h"
#include  "NiJointsTool.h"




// for OpenCV
#ifdef ENABLE_OPENCV
#include  "OpenCVTool.h"
#endif


#define	  OPENCV_FACE_FILE		"data/haarcascades/haarcascade_frontalface_alt.xml"
#define	  OPENCV_EYES_FILE		"data/haarcascades/haarcascade_eye.xml"
#define	  OPENCV_MOUTH_FILE		"data/haarcascades/haarcascade_mcs_mouth.xml"
/*
#define   OPENCV_FACE_FILE		"data/lbpcascades/lbpcascade_frontalface.xml"
#define   OPENCV_EYES_FILE		"data/haarcascades/haarcascade_eye_tree_eyeglasses.xml"
*/


// for Moving Avarage
#define   NI_MVAV_MAX_NUM		15		// should be smaller than NI_RING_BUFFER_SIZE in NiToolWin.h
#define	  NI_MVAV_EXP_DEC		 2		// weight of last data is 10^(-THIS) when use EMA


//
#define	  NI_LOG_POS_TITLE		"Lap Time, X Pos, Y Pos, Z Pos"

#define	  JOINT_RSTRCTN_TXT		"joint_restriction.txt"




namespace jbxwl {

using namespace jbxl;





//
//  拡張共通ジョイントクラス
//		ジョイントデータの補正
//

class  CExNiJoints : public CNiJoints
{
public:
	CExNiJoints(void);
	CExNiJoints(NiSDK_Lib lib);
	virtual ~CExNiJoints(void)	{ }


public:
	//
	CParameterSet appParam;

	//
	ExCmnHead**	pViewData;	// 参照用．メモリ管理なし．データをフリーしない事．

	rectangle	faceRect;
	rectangle   eyesRect[2];
	rectangle	mouthRect;

	bool		enableOpencvFace;
	bool		enableOpencvEyes;
	bool		enableOpencvMouth;

	BOOL		isDetectFace;
	BOOL		isDetectEyes;
	BOOL		isDetectMouth;


public:
	//
	NiMvAvType	mvav_type;
	float	mvav_weight[NI_MVAV_MAX_NUM];
	int		mvav_num;

	float	confidence;
	float	noise_min;

//	float	bone_length_min;
//	float	bone_length_max;

//	FILE*	pLogFile;
//	unsigned long  log_ttl_time;
//	unsigned short log_lap_time;



public:
	void	init(void);
	void	clear(void);

	//
	void	execFaceTracking(void);
	void	drawFaceTracking(int color, int line);

	//
	void	setParameter(CParameterSet param);
	void	initMvavWeight(void);

	// Smoothing
	void	PosMovingAverage(NiSDK_Lib sdk_lib);
	void	RotMovingAverage(NiSDK_Lib sdk_lib);

	// Constraints
	void	PosVibNoiseCanceler(void);
	void	RotVibNoiseCanceler(void);

	void	CheckJointsRotation(void);
	void	CheckGroundLevel(float ground_level);
	void	CheckBoneLength(void);

	// Log File
//	BOOL	openLogFile(LPCTSTR path);
//	void	closeLogFile(void);
//	void	writeLogFile(Vector<float> pos);


// OpenCV

public:
	BOOL    init_opencv(void);
	void	clear_opencv(void);

#ifdef ENABLE_OPENCV

	cv::CascadeClassifier opencvFace;
	cv::CascadeClassifier opencvEyes;
	cv::CascadeClassifier opencvMouth;
	

	void	execOpencvFaceTracking(void);
	void	drawOpencvFaceTracking(int color, int line);

#endif

};








/////////////////////////////////////////////////////

void   setRBoundJointsRotation(void);
void   readRBoundJointsRotation(char* fname);

void   initRBoundJointsRotation(void);
void   d2rRBoundJointsRotation(void);






}		// name space