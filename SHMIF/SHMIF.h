#pragma once

/** 
 * SHMIF.h    v1.0
 *
 *		Copyright (c) 2012, Fumi.Iseki
 * 
 */

#include "tools++.h"
#include "buffer.h"
#include "Vector.h"
#include "Rotation.h"

#define	  SHMIF_JOINT_NUM		34

#define   SHMIF_CHANNEL_NUM		22	// Local 1 + Network (Oneself 1 + Other 20)
#define   SHMIF_DATA_SIZE		64	// ジョイントのデータサイズ  8+8x3+8x4   see also ANM_COM_LEN_DATA in anm_data.h
#define   SHMIF_INDEX_LEN		40	// 36 + 4
#define   SHMIF_UUID_LEN		36

#define	  SHMIF_INDEX_NAME		"AnimationIndex"
#define   SHMIF_ZERO_UUID		"00000000-0000-0000-0000-000000000000"
#define   SHMIF_DEFAULT_ANIM	"f9073e22-6148-ea6f-8906-8378bfe6d44b"


//
//		  共有メモリインデックス	共有メモリ名		see	_SHMJointName[]
//
#define   SHMIF_PELVIS		0		//*"mPelvis"
#define   SHMIF_TORSO		1		//*"mTorso"
#define   SHMIF_CHEST		2		//*"mChest"
#define   SHMIF_NECK		3		//*"mNeck"
#define   SHMIF_HEAD		4		//*"mHead"
#define   SHMIF_SKULL		5		//*"mSkull"

#define   SHMIF_L_EYE		6		//*"mEyeLeft"
#define   SHMIF_R_EYE		7		//*"mEyeRight"
#define   SHMIF_L_BUST		8		// "mBustLeft"
#define   SHMIF_R_BUST		9		// "mBustRight"

#define   SHMIF_L_COLLAR	10		//*"mCollarLeft"
#define   SHMIF_L_SHLDR		11		//*"mShoulderLeft"
#define   SHMIF_L_ELBOW		12		//*"mElbowLeft"
#define   SHMIF_L_WRIST		13		//*"mWristLeft"
#define   SHMIF_L_FINGERTIP	14		// "mFingertipLeft"

#define   SHMIF_R_COLLAR	15		//*"mCollarRight"
#define   SHMIF_R_SHLDR		16		//*"mShoulderRight"
#define   SHMIF_R_ELBOW		17		//*"mElbowRight"
#define   SHMIF_R_WRIST		18		//*"mWristRight"
#define   SHMIF_R_FINGERTIP	19		// "mFingertipRight"

#define   SHMIF_L_HIP		20		//*"mHipLeft"
#define   SHMIF_L_KNEE		21		//*"mKneeLeft"
#define   SHMIF_L_ANKLE		22		//*"mAnkleLeft"
#define   SHMIF_L_FOOT		23		//*"mFootLeft"
#define   SHMIF_L_TOE		24		//*"mToeLeft"

#define   SHMIF_R_HIP		25		//*"mHipRight"
#define   SHMIF_R_KNEE		26		//*"mKneeRight"
#define   SHMIF_R_ANKLE		27		//*"mAnkleRight"
#define   SHMIF_R_FOOT		28		//*"mFootRight"
#define   SHMIF_R_TOE		29		//*"mToeRight"

#define   SHMIF_L_HAND		30		// "L_Hand"
#define   SHMIF_R_HAND		31		// "R_Hand"
#define   SHMIF_FACE		32		// "Expression"

#define   SHMIF_AVATAR		33		// "Avatar"


using namespace jbxl;


std::string  SHMJointName(int n);


///////////////////////////////////////////////////////////////////////////////////
//  CSHMInterFace クラス 
//
//   主にクライアントでの共有メモリの読み出しに用いる．
//

class  CSHMInterFace
{
public:
	Buffer		uuid;
	int			channel;

	BOOL		isSHMemory;

	BOOL		isTracking;
	BOOL		isProfUpper;

	void*		indexHandle;
	char*		ptrIndex;

	void*		mapHandle[SHMIF_JOINT_NUM];
	double*		ptrShm[SHMIF_JOINT_NUM];

public:
	CSHMInterFace()  {}
	CSHMInterFace(char* id) { init(id);}
	virtual	~CSHMInterFace() { closeShm();}

	void		init(char* id);

	void		createAllJointsShm(void);
	void 		createJointShm(int n);
	void 		openJointShm(int n);
	void		closeShm(void);

	void		createIndexShm(void);
	void		openIndexShm(void);
	void		closeIndexShm(void);
	int			getChannelIndexShm(Buffer uuid);
	void 		checkChannelIndexShm(void);

	void		getJointStatus(double* data);

	Quaternion<double> getAnimationRotation(std::string joint_name);
	Vector<double>     getAnimationPosition(std::string joint_name);
	Vector<double>     getAnimationScale   (std::string joint_name);
};

