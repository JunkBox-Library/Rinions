

#include "tools++.h"
#include "Dx9.h"
#include "Rotation.h"


using namespace jbxl;


#define MMD_SHM_API __declspec(dllexport)

MMD_SHM_API bool __stdcall OpenNIInit(HWND, bool, LPDIRECT3DDEVICE9, WCHAR*, CHAR*);
MMD_SHM_API void __stdcall OpenNIClean(void);
MMD_SHM_API void __stdcall OpenNIDrawDepthMap(bool);
MMD_SHM_API void __stdcall OpenNIDepthTexture(IDirect3DTexture9**);
MMD_SHM_API void __stdcall OpenNIGetSkeltonJointPosition(int, D3DXVECTOR3*);
MMD_SHM_API void __stdcall OpenNIIsTracking(bool*);
MMD_SHM_API void __stdcall OpenNIGetVersion(float*);






Vector<double>     get_joint_position(int num);
Quaternion<double> get_joint_rotation(int num);




////////////////////////////////////////////////////////////////////////////////////////////////
//

/*
���L��������
    mPelvis(0), mTorso(1), mChest(2), mNeck(3), mHead(4), mSkull(5), 
	mEyeLeft(6), mEyeRight(7), mBustLeft(8), mBustRight(9),
    mCollarLeft (10), mShoulderLeft (11), mElbowLeft (12), mWristLeft (13), mFingertipLeft (14),
    mCollarRight(15), mShoulderRight(16), mElbowRight(17), mWristRight(18), mFingertipRight(19),
    mHipLeft (20), mKneeLeft (21), mAnkleLeft (22), mFootLeft (23), mToeLeft (24),
    mHipRight(25), mKneeRight(26), mAnkleRight(27), mFootRight(28), mToeRight(29), 
    L_Hand(30), R_Hand(31), Expression(32), Rinions(33: for Control)
*/




//�{�[���̐�
#define MMD_JOINT_NUM		23

//�{�[���C���f�b�N�X
#define MMD_CENTER			0	//�Z���^�[  mPelvis(0)
#define MMD_NECK			1	//��		mNeck(3)
#define MMD_HEAD			2	//��		mHead(4)

#define MMD_L_SHOULDER		3	//����		mShoulderRight(16)
#define MMD_L_ELBOW			4	//���Ђ�	mElbowRight(17)
#define MMD_L_HAND			5	//�����	mWristRight(18)

#define MMD_R_SHOULDER		6	//�E��		mShoulderLeft(11)
#define MMD_R_ELBOW			7	//�E�Ђ�	mElbowLeft(12)
#define MMD_R_HAND			8	//�E���	mWristLeft(13)

#define MMD_L_HIP			9	//����		mHipRight(25)
#define MMD_L_KNEE			10	//���Ђ�	mKneeRight(26)
#define MMD_L_FOOT			11	//������	mFootRight(28)

#define MMD_R_HIP			12	//�E��		mHipLeft(20)
#define MMD_R_KNEE			13	//�E�Ђ�	mKneeLeft(21)
#define MMD_R_FOOT			14	//�E����	mFootLeft(23)

#define MMD_TORSO			15	//����	    mTorso(1)

#define MMD_L_FINGERTIP		16	//�����	mWristRight(18)
#define MMD_R_FINGERTIP		17	//�E���	mWristLeft(13)


#define MMD_L_TOE			18	//���ܐ�	mFootRight(28)		for *ver = 1.50f
#define MMD_R_TOE			19	//�E�ܐ�	mFootLeft(23)		for *ver = 1.50f

#define MMD_L_SHOULDER_M	20	//�����C��						for *ver = 1.50f
#define MMD_R_SHOULDER_M	21	//�E���C��						for *ver = 1.50f
#define MMD_HEAD_FRONT		22	//�琳�ʕ���					for *ver = 1.50f



