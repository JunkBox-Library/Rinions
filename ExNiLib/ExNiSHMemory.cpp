
#include  "stdafx.h"

#include  "ExNiSHMemory.h"
#include  "MessageBoxDLG.h"
#include  "WinTools.h"

#include  "anm_data.h"



/*
使用するパラメータ
	appParam.animationUUID		CSetAnimation
*/




CExNiSHMemory::CExNiSHMemory(void)
{
	indexHandle  = NULL;
	ptrAnimIndex = NULL;
		
	for (int i=0; i<SHMIF_JOINT_NUM; i++) {
		mapHandle[i]  = NULL;
		ptrMapData[i] = NULL;
	}

	isCreated  = createSharedMemory();
	isTracking = FALSE;

	appParam.init();
}





CExNiSHMemory::~CExNiSHMemory(void)
{
	DEBUG_INFO("DESTRUCTOR: CExNiSHMemory");

	closeSharedMemory();
}






///////////////////////////////////////////////////////////////////////////


BOOL  CExNiSHMemory::createSharedMemory(void)
{
	BOOL ret = TRUE;

	for (int i=0; i<SHMIF_JOINT_NUM; i++) {
		mapHandle[i] = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 
								SHMIF_DATA_SIZE*SHMIF_CHANNEL_NUM, (LPCTSTR)mbs2ts((char*)SHMJointName(i).c_str())); 
		if (mapHandle[i]==NULL) {
			ret = FALSE;
			break;
		}
		ptrMapData[i] = (double*)MapViewOfFile(mapHandle[i], FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (ptrMapData[i]==NULL) {
			ret = FALSE;
			break;
		}
		memset((char*)ptrMapData[i], 0, SHMIF_DATA_SIZE*SHMIF_CHANNEL_NUM);
	}

	// Index
	if (ret) {
		indexHandle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 
										SHMIF_INDEX_LEN*SHMIF_CHANNEL_NUM, _T(SHMIF_INDEX_NAME)); 
		if (indexHandle!=NULL) ptrAnimIndex = (char*)MapViewOfFile(indexHandle, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (indexHandle==NULL || ptrAnimIndex==NULL) ret = FALSE;
	}

	if (!ret) closeSharedMemory();

	clearSharedMemory();
	return ret;
}





void  CExNiSHMemory::closeSharedMemory(void)
{
	clearSharedMemory();

	for (int i=0; i<SHMIF_JOINT_NUM; i++) {
		if (ptrMapData[i]!=NULL) {
			memset((char*)ptrMapData[i], 0, SHMIF_DATA_SIZE*SHMIF_CHANNEL_NUM);
			UnmapViewOfFile(ptrMapData[i]);
			ptrMapData[i] = NULL;
		}
		if (mapHandle[i]!=NULL) {
			CloseHandle(mapHandle[i]);
			mapHandle[i] = NULL;
		}
	}


	if (ptrAnimIndex!=NULL) {
		memset(ptrAnimIndex, 0, SHMIF_INDEX_LEN*SHMIF_CHANNEL_NUM);
		UnmapViewOfFile(ptrAnimIndex);
		ptrAnimIndex = NULL;
	}
	
	if (indexHandle!=NULL) {
		CloseHandle(indexHandle);
		indexHandle = NULL;
	}

	isCreated = FALSE;
}




void  CExNiSHMemory::clearSharedMemory(void)
{
	clearLocalAnimationIndex();
	clearNetworkAnimationIndex();

	clearAnimationData();
}




///////////////////////////////////////////////////////////////////////////
// Index


void  CExNiSHMemory::setAnimationIndex(char* uuid, int n)
{
	if (ptrAnimIndex==NULL) return;

	char* ptr = ptrAnimIndex + n*SHMIF_INDEX_LEN;
	memcpy(ptr, uuid, ANM_COM_LEN_UUID);
}




void  CExNiSHMemory::setLocalAnimationIndex(void)
{
	if (appParam.animationUUID!="") {
		char* mbstr = ts2mbs(appParam.animationUUID);
		setAnimationIndex(mbstr, 0);
		::free(mbstr);
	}
}




void  CExNiSHMemory::clearLocalAnimationIndex(void)
{
	if (ptrAnimIndex!=NULL) {
		memset(ptrAnimIndex, 0, SHMIF_INDEX_LEN);
	}
}




int	 CExNiSHMemory::getNetworkAnimationIndexNum(void)
{
	int n = 0;
	char* ptr = ptrAnimIndex + SHMIF_INDEX_LEN;

	for (int i=1; i<SHMIF_CHANNEL_NUM; i++) {
		if (*ptr!=0x00) n++;
		ptr += SHMIF_INDEX_LEN;
	}

	return n;
}




int	 CExNiSHMemory::checkNetworkAnimationIndex(char* uuid)
{
	if (ptrAnimIndex==NULL) return -1;

	int   n, spidx = -1;
	char* ptr = ptrAnimIndex + SHMIF_INDEX_LEN;

	for (n=1; n<SHMIF_CHANNEL_NUM; n++) {
		if (!strncasecmp(ptr, uuid, ANM_COM_LEN_UUID)) break;
		else if (spidx==-1 && *ptr==0x00) spidx = n;
		ptr += SHMIF_INDEX_LEN;
	}

	if (n==SHMIF_CHANNEL_NUM) {
		if (spidx!=-1) {
			ptr = ptrAnimIndex + spidx*SHMIF_INDEX_LEN;
			memcpy(ptr, uuid, ANM_COM_LEN_UUID);
		}
		n = spidx;
	}

	return n;
}




int  CExNiSHMemory::clearNetworkAnimationIndex(char* uuid)
{
	char* ptr = ptrAnimIndex;
	if (ptr==NULL) return -1;

	int n;
	for (n=1; n<SHMIF_CHANNEL_NUM; n++) {
		if (!strncasecmp(ptr, uuid, ANM_COM_LEN_UUID)) {
			memset(ptr, 0, SHMIF_INDEX_LEN);
			break;
		}
		ptr += SHMIF_INDEX_LEN;
	}

	if (n<SHMIF_CHANNEL_NUM) return n;
	return -1;
}




void  CExNiSHMemory::clearNetworkAnimationIndex()
{
	if (ptrAnimIndex!=NULL) {
		memset(ptrAnimIndex+SHMIF_INDEX_LEN, 0, SHMIF_INDEX_LEN*(SHMIF_CHANNEL_NUM-1));
	}
}





//////////////////////////////////////////////////////////////////////////
// Data

void  CExNiSHMemory::updateLocalAnimationData(Vector<float>* posVect, Quaternion<float>* rotQuat, NiSDK_Lib lib, int joints_num)
{
	double* shm = NULL;

	unsigned short info[4];
	memset(info, 0, sizeof(unsigned short)*4);
	double* joint_info = (double*)info;

	info[1] = (unsigned short)joints_num;

	unsigned short mode = 0;
	if (appParam.detectParts==NI_PROFILE_UPPER) {
		mode |= NI_SDK_PROFILE_UPPER;
	}
	if (isTracking) {
		mode |= NI_SDK_IS_TRACKING;
	}


	// Position
	if (appParam.outDataPostion) {
		for (int j=0; j<joints_num; j++) {
			int n = NiSDK2SHMPosJointNum(j, lib);
			if (n>=0 && posVect[j].c>=0.0) {
				shm = ptrMapData[n];
				if (shm!=NULL) {
					shm[1]  = (double)posVect[j].x;
					shm[2]  = (double)posVect[j].y;
					shm[3]  = (double)posVect[j].z;
					info[0] = mode | ANM_COM_DATA_POSITION;
					shm[0]  = *joint_info;
				}
			}
		}
	}

	
	// Quaternion
	if (appParam.outDataQuate) {
		// Positon of Root Joint
		if (!appParam.outDataPostion) {
			int n = NiSDK2SHMPosJointNum(0, lib);
			if (n>=0 && posVect[0].c>=0.0) {
				shm = ptrMapData[n];
				if (shm!=NULL) {
					shm[1]  = (double)posVect[0].x;
					shm[2]  = (double)posVect[0].y;
					shm[3]  = (double)posVect[0].z;
					info[0] = mode | ANM_COM_DATA_POSITION;
					shm[0]  = *joint_info;
				}
			}
		}

		for (int j=0; j<joints_num; j++) {
			int n = NiSDK2SHMRotJointNum(j, lib);
			if (n>=0 && rotQuat[j].c>=0.0) {
				shm = ptrMapData[n];
				if (shm!=NULL) {
					shm[4]  = (double)rotQuat[j].x;
					shm[5]  = (double)rotQuat[j].y;
					shm[6]  = (double)rotQuat[j].z;
					shm[7]  = (double)rotQuat[j].s;
					info[0] = mode | ANM_COM_DATA_ROTATION;
					shm[0]  = *joint_info;
				}
			}
		}
	}
}




void  CExNiSHMemory::clearLocalAnimationData(void)
{
	for (int i=0; i<SHMIF_JOINT_NUM; i++) {
		if (ptrMapData[i]!=NULL) {
			memset((char*)ptrMapData[i], 0, SHMIF_DATA_SIZE);
		}
	}
}




void  CExNiSHMemory::clearNetworkAnimationData(void)
{
	for (int i=0; i<SHMIF_JOINT_NUM; i++) {
		if (ptrMapData[i]!=NULL) {
			memset((char*)ptrMapData[i]+SHMIF_DATA_SIZE, 0, SHMIF_DATA_SIZE*(SHMIF_CHANNEL_NUM-1));
		}
	}
}




void  CExNiSHMemory::clearAnimationData(void)
{
	for (int i=0; i<SHMIF_JOINT_NUM; i++) {
		if (ptrMapData[i]!=NULL) {
			memset((char*)ptrMapData[i], 0, SHMIF_DATA_SIZE*SHMIF_CHANNEL_NUM);
		}
	}
}





//////////////////////////////////////////////////////////////////////////
// Index + Data

/**


@param data   データ部へのポインタ
@param uuid   アニメーションUUID
@param joints データの個数
@param size   一個のデータサイズ


dataの先頭4Byte       mode[0]                    mode[1]
     +-------------------------------------------------------+
     |ジョイントの状態(1)|パケットの状態(1)|ジョイント番号(2)|
     +-------------------------------------------------------+

ジョイントの状態
  上半身モード　　　NI_SHM_PROFILE_UPPER
  トラックング中	NI_SHM_IS_TRACKING

パケット内のデータの状態：
//  0x0000 位置データと回転データが存在する．
  ANM_COM_DATA_POSITION (0x0001) 位置データが存在する．
  ANM_COM_DATA_ROTATION（0x0002）回転データが存在する．

*/
void  CExNiSHMemory::updateNetworkAnimation(char* data, char* uuid, int joints)
{
	//if (data==NULL) {
	//	clearNetworkAnimation(uuid);
	//	return;
	//}

	int channel = checkNetworkAnimationIndex(uuid);
	if (channel<0) return;		// インデックス部にUUIDが無い

	int chnlpos = channel*ANM_COM_NUM_DATA;
	unsigned short* mode = NULL;

	unsigned short info[4];
	memset(info, 0, sizeof(unsigned short)*4);
	double* joint_info = (double*)info;


	float* net = (float*)data;
	for (int i=0; i<joints; i++) {
		mode = (unsigned short*)net;
		int n = mode[1];	// Joint番号
		//
		if (n>=0 && n<SHMIF_JOINT_NUM && ptrMapData[n]!=NULL) {
			double* shm = ptrMapData[n] + chnlpos;
				
			//
			info[0] = mode[0];		// ジョイントとパケットの状態
			info[1] = mode[1];		// ジョイント番号
			shm[0]  = *joint_info;
			net++;

			if (mode[0]&ANM_COM_DATA_POSITION) {
				shm[1] = (double)net[0];
				shm[2] = (double)net[1];
				shm[3] = (double)net[2];
				net += 3;
			}
			if (mode[0]&ANM_COM_DATA_ROTATION) {
				shm[4] = (double)net[0];
				shm[5] = (double)net[1];
				shm[6] = (double)net[2];
				shm[7] = (double)net[3];
				net += 4;
			}
		}
		//
		else {
			net++;		
			if (mode[0]&ANM_COM_DATA_POSITION) {
				net += 3;
			}
			if (mode[0]&ANM_COM_DATA_ROTATION) {
				net += 4;
			}
		}
	}

	/*
	// Double Data
	else {
		double* net = (double*)data;
		for (int i=0; i<joints; i++) {
			mode = (unsigned short*)net;
			int n = mode[1];
			//
			if (n>=0 && n<SHMIF_JOINT_NUM && ptrMapData[n]!=NULL) {
				double* shm = ptrMapData[n] + chnlpos;

				//
				info[0] = mode[0];		// ジョイントとパケットの状態
				info[1] = mode[1];		// ジョイント番号
				shm[0]  = *joint_info;
				net++;
				
				// 1-3
				if (mode[0]&ANM_COM_DATA_POSITION) {
					memcpy((char*)(shm+1), (char*)net, ANM_COM_LEN_POS);
					net += 3;
				}
				// 4-7
				if (mode[0]&ANM_COM_DATA_ROTATION) {
					memcpy((char*)(shm+4), (char*)net, ANM_COM_LEN_ROT);
					net += 4;
				}
			}
			//
			else {
				net++;		
				if (mode[0]&ANM_COM_DATA_POSITION) {
					net += 3;
				}
				if (mode[0]&ANM_COM_DATA_ROTATION) {
					net += 4;
				}
			}
		}
	}
	*/
}





void  CExNiSHMemory::clearNetworkAnimation(char* uuid)
{
	char* ptr = ptrAnimIndex;
	if (ptr==NULL) return;

	int n;

	// 検索：ローカルはクリア対象ではないので，検索は nが 1から
	for (n=1; n<SHMIF_CHANNEL_NUM; n++) {
		if (!strncasecmp(ptr, uuid, ANM_COM_LEN_UUID)) {
			memset(ptr, 0, SHMIF_INDEX_LEN);
			break;
		}
		ptr += SHMIF_INDEX_LEN;
	}

	// クリア
	if (n<SHMIF_CHANNEL_NUM) {
		for (int i=0; i<SHMIF_JOINT_NUM; i++) {
			if (ptrMapData[i]!=NULL) {
				memset((char*)ptrMapData[i]+SHMIF_DATA_SIZE*n, 0, SHMIF_DATA_SIZE);
			}
		}
	}

	return;
}

