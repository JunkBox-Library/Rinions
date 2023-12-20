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





// SDK 固有のジョイント番号から SharedMemory(SL/OS) のジョイントの情報を得る
int			 NiSDK2SHMPosJointNum (int n, NiSDK_Lib lib);	// 座標用ジョイント番号
std::string  NiSDK2SHMPosJointName(int n, NiSDK_Lib lib);	// 座標用ジョイント名
int			 NiSDK2SHMRotJointNum (int n, NiSDK_Lib lib);	// 回転用ジョイント番号
std::string  NiSDK2SHMRotJointName(int n, NiSDK_Lib lib);	// 回転用ジョイント名


// 共通ジョイント番号から SharedMemory(SL/OS) のジョイントの情報を得る
std::string  Ni2SHMPosJointName(int n);						// 座標用ジョイント名
std::string  Ni2SHMRotJointName(int n);						// 回転用ジョイント名


