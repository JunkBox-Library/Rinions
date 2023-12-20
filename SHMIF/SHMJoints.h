#pragma once


#include  "common++.h"
#include  "NiToolWin.h"
#include  "NiJointsTool.h"

#include  "SHMIF.h"



using namespace jbxwl;



int	 OpenNI2SHMRotJointNum(int n);
int	 OpenNI2SHMPosJointNum(int n);

int	 Kinect2SHMRotJointNum(int n);
int	 Kinect2SHMPosJointNum(int n);

int	 Ni2SHMRotJointNum(int n);
int	 Ni2SHMPosJointNum(int n);





// SDK �ŗL�̃W���C���g�ԍ����� SharedMemory(SL/OS) �̃W���C���g�̏��𓾂�
int			 NiSDK2SHMPosJointNum (int n, NiSDK_Lib lib);	// ���W�p�W���C���g�ԍ�
std::string  NiSDK2SHMPosJointName(int n, NiSDK_Lib lib);	// ���W�p�W���C���g��
int			 NiSDK2SHMRotJointNum (int n, NiSDK_Lib lib);	// ��]�p�W���C���g�ԍ�
std::string  NiSDK2SHMRotJointName(int n, NiSDK_Lib lib);	// ��]�p�W���C���g��


// ���ʃW���C���g�ԍ����� SharedMemory(SL/OS) �̃W���C���g�̏��𓾂�
std::string  Ni2SHMPosJointName(int n);						// ���W�p�W���C���g��
std::string  Ni2SHMRotJointName(int n);						// ��]�p�W���C���g��


