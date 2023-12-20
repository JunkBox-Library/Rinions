
#include "stdafx.h"

#include "SHMJoints.h"





/*
‹¤—Lƒƒ‚ƒŠ–¼
    mPelvis(0), mTorso(1), mChest(2), mNeck(3), mHead(4), mSkull(5), 
	mEyeLeft(6), mEyeRight(7), mBustLeft(8), mBustRight(9),
    mCollarLeft (10), mShoulderLeft (11), mElbowLeft (12), mWristLeft (13), mFingertipLeft (14),
    mCollarRight(15), mShoulderRight(16), mElbowRight(17), mWristRight(18), mFingertipRight(19),
    mHipLeft (20), mKneeLeft (21), mAnkleLeft (22), mFootLeft (23), mToeLeft (24),
    mHipRight(25), mKneeRight(26), mAnkleRight(27), mFootRight(28), mToeRight(29), 
    L_Hand(30), R_Hand(31), Expression(32), Avatar(33)
*/


////////////////////////////////////////////////////////////////////////////////////////
//
// OpenNI
//

/*                        ˆÊ’u  ‰ñ“]  
 0: PELVIS					›  ›  => SHMIF_PELVIS(0)
 1: XN_SKEL_HEAD	        ›  ›  => SHMIF_HEAD(4)
 2: XN_SKEL_NECK			›  ›  => SHMIF_NECK(3)                   1
 3: XN_SKEL_TORSO			›  ›  => SHMIF_TORSO(1)                  |
 4: XN_SKEL_WAIST			~  ~  => none              9 -  7 -  6 - 2 - 12 - 13 - 15
 5: XN_SKEL_LEFT_COLLAR		~  ~  => none                            |
 6: XN_SKEL_LEFT_SHOULDER	›  ›  => SHMIF_R_SHLDR(16)               3
 7: XN_SKEL_LEFT_ELBOW		›  ›  => SHMIF_R_ELBOW(17)               |
 8: XN_SKEL_LEFT_WRIST		~  ~  => none                            0    
 9: XN_SKEL_LEFT_HAND		›  ~  =>[SHMIF_R_WRIST(18)]          17      22
10: XN_SKEL_LEFT_FINGERTIP	~  ~  => none                        |       |
11: XN_SKEL_RIGHT_COLLAR	~  ~  => none                        18      22
12: XN_SKEL_RIGHT_SHOULDER	›  ›  => SHMIF_L_SHLDR(11)           |       |
13: XN_SKEL_RIGHT_ELBOW		›  ›  => SHMIF_L_ELBOW(12)           20      24
14: XN_SKEL_RIGHT_WRIST		~  ~  => none
15: XN_SKEL_RIGHT_HAND      ›  ~  =>[SHMIF_L_WRIST(13)]
16: XN_SKEL_RIGHT_FINGERTIP ~  ~  => none
17: XN_SKEL_LEFT_HIP		›  ›  => SHMIF_R_HIP(25)
18: XN_SKEL_LEFT_KNEE		›  ›  => SHMIF_R_KNEE(26)
19: XN_SKEL_LEFT_ANKLE		~  ~  => none
20: XN_SKEL_LEFT_FOOT		›  ~  =>[SHMIF_R_ANKLE(27)]
21: XN_SKEL_RIGHT_HIP		›  ›  => SHMIF_L_HIP(20)
22: XN_SKEL_RIGHT_KNEE		›  ›  => SHMIF_L_KNEE(21)
23: XN_SKEL_RIGHT_ANKLE		~  ~  => none
24: XN_SKEL_RIGHT_FOOT		›  ~  =>[SHMIF_L_ANKLE(22)]

[ ] ‚Í––’[i‰ñ“]‚È‚µj
*/
static  int  _OpenNI2SHMPosJoint[] =		// OPENNI_JOINT_NUM
{
	 0,  4,  3,  1, -1, 
	-1, 16, 17, -1, 18, -1, 
	-1, 11, 12, -1, 13, -1,
	25, 26, -1, 27,
	20, 21, -1, 22 
};

static  int  _OpenNI2SHMRotJoint[] =		// OPENNI_JOINT_NUM
{
	 0,  4,  3,  1, -1, 
	-1, 16, 17, -1, -1, -1, 
	-1, 11, 12, -1, -1, -1,
	25, 26, -1, -1,
	20, 21, -1, -1 
};


int	 OpenNI2SHMPosJointNum(int n)
{
	return _OpenNI2SHMPosJoint[n];
}


int	 OpenNI2SHMRotJointNum(int n)
{
	return _OpenNI2SHMRotJoint[n];
}



////////////////////////////////////////////////////////////////////////////////////////
//
// OpenNI2
//

/*							  ˆÊ’u  ‰ñ“]  
 0: PELVIS						›  ›  => SHMIF_PELVIS(0)
 1: NITE_JOINT_HEAD				›  ›  => SHMIF_HEAD(4)
 2: NITE_JOINT_NECK				›  ›  => SHMIF_NECK(3)                      1
 3: NITE_JOINT_LEFT_SHOULDER	›  ›  => SHMIF_R_SHLDR(16)                  |
 4: NITE_JOINT_RIGHT_SHOULDER	›  ›  => SHMIF_L_SHLDR(11)      7 - 5 - 3 - 2 - 4 - 6 - 8
 5: NITE_JOINT_LEFT_ELBOW		›  ›  => SHMIF_R_ELBOW(17)                  |
 6: NITE_JOINT_RIGHT_ELBOW		›  ›  => SHMIF_L_ELBOW(12)                  9
 7: NITE_JOINT_LEFT_HAND		›  ~  =>[SHMIF_R_WRIST(18)]                 |
 8: NITE_JOINT_RIGHT_HAND		›  ~  =>[SHMIF_L_WRIST(13)]                 0    
 9: NITE_JOINT_TORSO			›  ›  => SHMIF_TORSO(1)                 10      11
10: NITE_JOINT_LEFT_HIP			›  ›  => SHMIF_R_HIP(25)                |       |
11: NITE_JOINT_RIGHT_HIP		›  ›  => SHMIF_L_HIP(20)                12      13
12: NITE_JOINT_LEFT_KNEE		›  ›  => SHMIF_R_KNEE(26)               |       |
13: NITE_JOINT_RIGHT_KNEE		›  ›  => SHMIF_L_KNEE(21)               14      15
14: NITE_JOINT_LEFT_FOOT		›  ~  =>[SHMIF_R_ANKLE(27)]
15: NITE_JOINT_RIGHT_FOOT		›  ~  =>[SHMIF_L_ANKLE(22)]

[ ] ‚Í––’[i‰ñ“]‚È‚µj
*/
static  int  _OpenNI22SHMPosJoint[] =		// OPENNI2_JOINT_NUM
{
	 0,  4,  3,
	16, 11, 17, 12, 18, 13,
	 1,
	25, 20, 26, 21, 27, 22
};

static  int  _OpenNI22SHMRotJoint[] =		// OPENNI2_JOINT_NUM
{
	 0,  4,  3,
	16, 11, 17, 12, -1, -1,
	 1,
	25, 20, 26, 21, -1, -1
};


int	 OpenNI22SHMPosJointNum(int n)
{
	return _OpenNI22SHMPosJoint[n];
}


int	 OpenNI22SHMRotJointNum(int n)
{
	return _OpenNI22SHMRotJoint[n];
}





////////////////////////////////////////////////////////////////////////////////////////
//
// Kinect SDK
//

/*
 0: NUI_SKELETON_POSITION_HIP_CENTER      => SHMIF_PELVIS(0)
 1: NUI_SKELETON_POSITION_SPINE           => SHMIF_TORSO(1)
 2: NUI_SKELETON_POSITION_SHOULDER_CENTER => SHMIF_NECK(3)
 3: NUI_SKELETON_POSITION_HEAD            => SHMIF_HEAD(4)                        3
 4: NUI_SKELETON_POSITION_SHOULDER_LEFT   => SHMIF_L_SHLDR(11)                    |
 5: NUI_SKELETON_POSITION_ELBOW_LEFT      => SHMIF_L_ELBOW(12)  11 - 10 - 9 - 8 - 2 - 4 - 5 - 6 - 7
 6: NUI_SKELETON_POSITION_WRIST_LEFT      => SHMIF_L_WRIST(13)                    |
 7: NUI_SKELETON_POSITION_HAND_LEFT       => none                                 1
 8: NUI_SKELETON_POSITION_SHOULDER_RIGHT  => SHMIF_R_SHLDR(16)                    |
 9: NUI_SKELETON_POSITION_ELBOW_RIGHT     => SHMIF_R_ELBOW(17)               16   0   12
10: NUI_SKELETON_POSITION_WRIST_RIGHT     => SHMIF_R_WRIST(18)               |        |
11: NUI_SKELETON_POSITION_HAND_RIGHT      => none                            17       13
12: NUI_SKELETON_POSITION_HIP_LEFT        => SHMIF_L_HIP(20)                 |        |
13: NUI_SKELETON_POSITION_KNEE_LEFT       => SHMIF_L_KNEE(21)                18       14
14: NUI_SKELETON_POSITION_ANKLE_LEFT      => SHMIF_L_ANKLE(22)               |        |
15: NUI_SKELETON_POSITION_FOOT_LEFT       =>[SHMIF_L_FOOT(23)]               19       15
16: NUI_SKELETON_POSITION_HIP_RIGHT       => SHMIF_R_HIP(25)
17: NUI_SKELETON_POSITION_KNEE_RIGHT      => SHMIF_R_KNEE(26)
18: NUI_SKELETON_POSITION_ANKLE_RIGHT     => SHMIF_R_ANKLE(27) 
19: NUI_SKELETON_POSITION_FOOT_RIGHT      =>[SHMIF_R_FOOT(28)]

[ ] ‚Í––’[, 
*/
static  int  _Kinect2SHMPosJoint[] =		// KINECT_JOINT_NUM
{
	 0,  1,  3,  4, 
	11, 12, 13, -1,
	16, 17, 18, -1,
	20, 21, 22, 23, 
	25, 26, 27, 28
};

static  int  _Kinect2SHMRotJoint[] =		// KINECT_JOINT_NUM
{
	 0,  1,  3,  4, 
	11, 12, 13, -1,
	16, 17, 18, -1,
	20, 21, 22, -1, 
	25, 26, 27, -1
};


int	 Kinect2SHMRotJointNum(int n)
{
	return _Kinect2SHMRotJoint[n];
}


int	 Kinect2SHMPosJointNum(int n)
{
	return _Kinect2SHMPosJoint[n];
}




////////////////////////////////////////////////////////////////////////////////////////
//
// ‹¤’ÊƒWƒ‡ƒCƒ“ƒg”Ô†
//

/*
#define	  NI_PELVIS		 0  => SHMIF_PELVIS(0)
#define	  NI_WAIST		 1  => none
#define	  NI_TORSO		 2  => SHMIF_TORSO(1)
#define	  NI_CHEST		 3  => SHMIF_CHEST(2)
#define	  NI_NECK		 4  => SHMIF_NECK(3)
#define	  NI_HEAD		 5  => SHMIF_HEAD(4)
#define	  NI_SKULL		 6  =>[SHMIF_SKULL(5)]

#define	  NI_L_EYE		 7  => SHMIF_L_EYE(6)
#define	  NI_R_EYE		 8  => SHMIF_R_EYE(7)
#define	  NI_L_BUST		 9  => SHMIF_L_BUST(8)
#define	  NI_R_BUST		10  => SHMIF_R_BUST(9)

#define   NI_L_COLLAR	11  => SHMIF_L_COLLAR(10)
#define   NI_L_SHLDR	12  => SHMIF_L_SHLDR(11)
#define   NI_L_ELBOW	13  => SHMIF_L_ELBOW(12)
#define	  NI_L_WRIST	14  => SHMIF_L_WRIST(13)
#define	  NI_L_HAND		15  => none
#define   NI_L_FNGRTIP  16  =>[SHMIF_L_FINGERTIP(14)]

#define   NI_R_COLLAR	17  => SHMIF_R_COLLAR(15)
#define	  NI_R_SHLDR	18  => SHMIF_R_SHLDR(16)
#define	  NI_R_ELBOW	19  => SHMIF_R_ELBOW(17)
#define	  NI_R_WRIST	20  => SHMIF_R_WRIST(18)
#define	  NI_R_HAND		21  => none
#define   NI_R_FNGRTIP  22  =>[SHMIF_R_FINGERTIP(19)]

#define	  NI_L_HIP		23  => SHMIF_L_HIP(20) 
#define   NI_L_KNEE		24  => SHMIF_L_KNEE(21) 
#define	  NI_L_ANKLE	25  => SHMIF_L_ANKLE(22) 
#define	  NI_L_FOOT		26  => SHMIF_L_FOOT(23)
#define	  NI_L_TOE		27  =>[SHMIF_L_TOE(24)]
	
#define	  NI_R_HIP		28  => SHMIF_R_HIP(25)
#define	  NI_R_KNEE		29  => SHMIF_R_KNEE(26) 
#define	  NI_R_ANKLE	30  => SHMIF_R_ANKLE(27) 
#define	  NI_R_FOOT		31  => SHMIF_R_FOOT(28)
#define	  NI_R_TOE		32  =>[SHMIF_R_TOE(29)]

#define	  NI_L_FINGER	33  => SHMIF_L_HAND(30)
#define	  NI_R_FINGER	34  => SHMIF_R_HAND(31)
#define   NI_FACE		35  => SHMIF_FACE(32)

#define	  NI_AVATAR		36  => SHMIF_AVATAR(33)
	
[ ] ‚Í––’[
*/
static  int  _Ni2SHMPosJoint[] =		// NI_TOTAL_JOINT_NUM (37)
{
	 0, -1,  1,  2,  3,  4,  5,
	 6,  7,  8,  9,
	10, 11, 12, 13, -1, 14,
	15, 16, 17, 18, -1, 19,
	20, 21, 22, 23, 24,
	25, 26, 27, 28, 29,
	30, 31, 32, 33
};

static  int  _Ni2SHMRotJoint[] =		// NI_TOTAL_JOINT_NUM (37)
{
	 0, -1,  1,  2,  3,  4, -1,
	 6,  7,  8,  9,
	10, 11, 12, 13, -1, -1,
	15, 16, 17, 18, -1, -1,
	20, 21, 22, 23, -1,
	25, 26, 27, 28, -1,
	30, 31, 32, 33
};


int	 Ni2SHMPosJointNum(int n)
{
	return _Ni2SHMRotJoint[n];
}


int	 Ni2SHMRotJointNum(int n)
{
	return _Ni2SHMRotJoint[n];
}







////////////////////////////////////////////////////////////////////////////////////////
//
// Wrapper
//

//
// Library ŒÅ—L‚ÌƒWƒ‡ƒCƒ“ƒg”Ô†‚©‚ç SHMIF‚ÌÀ•W—pƒWƒ‡ƒCƒ“ƒg”Ô†‚ð“¾‚é
//
int  NiSDK2SHMPosJointNum(int n, NiSDK_Lib lib)
{
	if (lib==NiSDK_OpenNI2) {
		return _OpenNI22SHMPosJoint[n];
	}
	else if (lib==NiSDK_OpenNI) {
		return _OpenNI2SHMPosJoint[n];
	}
	else if (lib==NiSDK_Kinect) {
		return _Kinect2SHMPosJoint[n];
	}
	else if (lib==NiSDK_None) {
		return _Ni2SHMPosJoint[n];
	}

	return -1;
}



//
// Library ŒÅ—L‚ÌƒWƒ‡ƒCƒ“ƒg”Ô†‚©‚ç SHMIF‚Ì‰ñ“]—pƒWƒ‡ƒCƒ“ƒg”Ô†‚ð“¾‚é
//
int  NiSDK2SHMRotJointNum(int n, NiSDK_Lib lib)
{
	if (lib==NiSDK_OpenNI2) {
		return _OpenNI22SHMRotJoint[n];
	}
	else if (lib==NiSDK_OpenNI) {
		return _OpenNI2SHMRotJoint[n];
	}
	else if (lib==NiSDK_Kinect) {
		return _Kinect2SHMRotJoint[n];
	}
	else if (lib==NiSDK_None) {
		return _Ni2SHMRotJoint[n];
	}

	return -1;
}



//
// Library ŒÅ—L‚ÌƒWƒ‡ƒCƒ“ƒg”Ô†‚©‚ç SL/OS‚ÌÀ•W—pƒWƒ‡ƒCƒ“ƒg–¼‚ð“¾‚é
//
std::string  NiSDK2SHMPosJointName(int n, NiSDK_Lib lib)
{
	std::string  str = "";
	int j = -1;

	if (lib==NiSDK_OpenNI2) {
		j = _OpenNI22SHMPosJoint[n];
	}
	else if (lib==NiSDK_OpenNI) {
		j = _OpenNI2SHMPosJoint[n];
	}
	else if (lib==NiSDK_Kinect) {
		j = _Kinect2SHMPosJoint[n];
	}
	else if (lib==NiSDK_None) {
		j = _Ni2SHMPosJoint[n];
	}

	if (j>=0) str = SHMJointName(j);

	return str;
}



//
// Library ŒÅ—L‚ÌƒWƒ‡ƒCƒ“ƒg”Ô†‚©‚ç SL/OS‚Ì‰ñ“]—pƒWƒ‡ƒCƒ“ƒg–¼‚ð“¾‚é
//
std::string  NiSDK2SHMRotJointName(int n, NiSDK_Lib lib)
{
	std::string  str = "";
	int j = -1;

	if (lib==NiSDK_OpenNI2) {
		j = _OpenNI22SHMRotJoint[n];
	}
	else if (lib==NiSDK_OpenNI) {
		j = _OpenNI2SHMRotJoint[n];
	}
	else if (lib==NiSDK_Kinect) {
		j = _Kinect2SHMRotJoint[n];
	}
	else if (lib==NiSDK_None) {
		j = _Ni2SHMRotJoint[n];
	}

	if (j>=0) str = SHMJointName(j);

	return str;
}



//
// ‹¤’ÊƒWƒ‡ƒCƒ“ƒg”Ô†‚©‚ç SL/OS‚ÌÀ•W—pƒWƒ‡ƒCƒ“ƒg–¼‚ð“¾‚é
//
std::string  Ni2SHMPosJointName(int n)
{
	std::string  str = "";

	int j = _Ni2SHMPosJoint[n];
	if (j>=0) str = SHMJointName(j);

	return str;
}



//
// ‹¤’ÊƒWƒ‡ƒCƒ“ƒg”Ô†‚©‚ç SL/OS‚Ì‰ñ“]—pƒWƒ‡ƒCƒ“ƒg–¼‚ð“¾‚é
//
std::string  Ni2SHMRotJointName(int n)
{
	std::string  str = "";

	int j = _Ni2SHMRotJoint[n];
	if (j>=0) str = SHMJointName(j);

	return str;
}
