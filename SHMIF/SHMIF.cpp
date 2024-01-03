/** 
 *  SHMIF.cpp  v1.0
 *
 *		Copyright (c) 2011, Fumi.Iseki
 *
 *      CSHMInterFace クラスは，主にクライアントでの共有メモリの読み出しに用いる．
 * 
 */

#include "stdafx.h"

#include "SHMIF.h"
#include "NiJointsTool.h"





// SHM Joints
static  std::string _SHMJointName[] =	// SHMIF_JOINT_NUM
{
	"mPelvis", "mTorso", "mChest", "mNeck", "mHead", "mSkull", 
	"mEyeLeft", "mEyeRight", "mBustLeft", "mBustRight",
	"mCollarLeft",  "mShoulderLeft",  "mElbowLeft",  "mWristLeft",  "mFingertipLeft",
	"mCollarRight", "mShoulderRight", "mElbowRight", "mWristRight", "mFingertipRight",
	"mHipLeft",  "mKneeLeft",  "mAnkleLeft",  "mFootLeft",  "mToeLeft",
	"mHipRight", "mKneeRight", "mAnkleRight", "mFootRight", "mToeRight", 
	"L_Hand",  "R_Hand", "Expression", "Avatar"
};




std::string  SHMJointName(int n)
{
	std::string str = "";

	if (n>=0 && n<SHMIF_JOINT_NUM) {
		str = _SHMJointName[n];
	}

	return str;
}






///////////////////////////////////////////////////////////////////////////////////
//  CSHMInterFace クラス 
//
//   主にクライアントでの共有メモリの読み出しに用いる．
//

void  CSHMInterFace::init(char* id)
{
	uuid        = make_Buffer_bystr(id);
	isSHMemory  = FALSE;
	isTracking  = FALSE;
	isProfUpper = FALSE;

	indexHandle = NULL;
	ptrIndex    = NULL;
	channel     = -1;

	for (int i=0; i<SHMIF_JOINT_NUM; i++) {
		mapHandle[i] = NULL;
		ptrShm[i]    = NULL;
	}

	//
	createIndexShm();
	createAllJointsShm();
}






/////////////////////////////////////////////////////////////////////////
// Shared Memory of Joint

void  CSHMInterFace::createAllJointsShm(void)
{
	if (!isSHMemory) return;

	for (int i=0; i<SHMIF_JOINT_NUM; i++) {
		createJointShm(i);
	}
}




void  CSHMInterFace::createJointShm(int n)
{
	if (ptrShm[n]==NULL) {
		if (mapHandle[n]==NULL) {
			std::string joint_name = SHMJointName(n);
			mapHandle[n] = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, SHMIF_DATA_SIZE*SHMIF_CHANNEL_NUM, joint_name.c_str());
		}

		if (mapHandle[n]!=NULL) {
			ptrShm[n] = (double*)MapViewOfFile(mapHandle[n], FILE_MAP_READ, 0, 0, SHMIF_DATA_SIZE*SHMIF_CHANNEL_NUM);
		}
	}
}




void  CSHMInterFace::openJointShm(int n)
{
	//if (n<0 || n>=SHMIF_JOINT_NUM) return;
	
	if (ptrShm[n]==NULL) {
		if (mapHandle[n]==NULL) {
			std::string joint_name = SHMJointName(n);
			mapHandle[n] = OpenFileMappingA(FILE_MAP_READ, FALSE, joint_name.c_str());
		}

		if (mapHandle[n]!=NULL) {
			ptrShm[n] = (double*)MapViewOfFile(mapHandle[n], FILE_MAP_READ, 0, 0, SHMIF_DATA_SIZE*SHMIF_CHANNEL_NUM);
		}
	}
}




void  CSHMInterFace::closeShm(void)
{
	closeIndexShm();

	for (int i=0; i<SHMIF_JOINT_NUM; i++) {
		if (ptrShm[i]!=NULL)    UnmapViewOfFile(ptrShm[i]);
		if (mapHandle[i]!=NULL) CloseHandle(mapHandle[i]);
		ptrShm[i]    = NULL;
		mapHandle[i] = NULL;
	}

	free_Buffer(&uuid);
}






/////////////////////////////////////////////////////////////////////////
// Shred Memory of Animation UUID 

void  CSHMInterFace::createIndexShm()
{
	if (ptrIndex==NULL) {
		if (indexHandle==NULL) {
			indexHandle = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, SHMIF_INDEX_LEN*SHMIF_CHANNEL_NUM, SHMIF_INDEX_NAME);
		}

		if (indexHandle!=NULL) {
			ptrIndex = (char*)MapViewOfFile(indexHandle, FILE_MAP_READ, 0, 0, SHMIF_INDEX_LEN*SHMIF_CHANNEL_NUM);
		}
	}
}




void  CSHMInterFace::openIndexShm()
{
	if (ptrIndex==NULL) {
		if (indexHandle==NULL) {
			indexHandle = OpenFileMappingA(FILE_MAP_READ, FALSE, SHMIF_INDEX_NAME	);
		}

		if (indexHandle!=NULL) {
			ptrIndex = (char*)MapViewOfFile(indexHandle, FILE_MAP_READ, 0, 0, SHMIF_INDEX_LEN	*SHMIF_CHANNEL_NUM);
		}
	}
}




void  CSHMInterFace::closeIndexShm(void)
{
	if (ptrIndex!=NULL)    UnmapViewOfFile(ptrIndex);
	if (indexHandle!=NULL) CloseHandle(indexHandle);
	ptrIndex    = NULL;
	indexHandle = NULL;
}




int  CSHMInterFace::getChannelIndexShm(Buffer uuid)
{
	openIndexShm();
	if (ptrIndex==NULL) return -1;

	int   n;
	char* ptr = ptrIndex;
	for (n=0; n<SHMIF_CHANNEL_NUM; n++) {
		if (!strncasecmp((const char*)uuid.buf, ptr, SHMIF_UUID_LEN)) break;
		ptr += SHMIF_INDEX_LEN;
	}

	if (n==SHMIF_CHANNEL_NUM) n = -1;
	return n;
}




void  CSHMInterFace::checkChannelIndexShm(void)
{
	channel = getChannelIndexShm(uuid);
	if (channel>=0) {
		isSHMemory  = TRUE;
		isTracking  = TRUE;
	}
	else {
		isSHMemory  = FALSE;
		isTracking  = FALSE;
		isProfUpper = FALSE;
	}
}





//
void  CSHMInterFace::getJointStatus(double* data)
{
	unsigned short* ptr = (unsigned short*)data;

	if ((*ptr)&NI_SDK_PROFILE_UPPER) {
		isProfUpper = TRUE;
	}
	else {
		isProfUpper = FALSE;
	}

	//
	if ((*ptr)&NI_SDK_IS_TRACKING) {
		isTracking  = TRUE;
	}
	else {
		isTracking  = FALSE;
		isProfUpper = FALSE;
	}
}






/////////////////////////////////////////////////////////////////////////
// Get Joint Data

Quaternion<double>  CSHMInterFace::getAnimationRotation(std::string joint_name)
{
	Quaternion<double> quat(1.0, 0.0, 0.0, 0.0, 1.0);

	char* ptr = NULL;
	for (int i=0; i<SHMIF_JOINT_NUM; i++) {
		if (joint_name==SHMJointName(i)) {
			openJointShm(i);
			ptr = (char*)ptrShm[i];
			break;
		}
	}

	if (ptr!=NULL) {
		double* element = (double*)(ptr + channel*SHMIF_DATA_SIZE);
		getJointStatus(element);
		quat.set(element[7], element[4], element[5], element[6]);
	}

	return quat;
}




Vector<double>  CSHMInterFace::getAnimationPosition(std::string joint_name)
{
	Vector<double> vect(0.0, 0.0, 0.0);

	char* ptr = NULL;
	for (int i=0; i<SHMIF_JOINT_NUM; i++) {
		if (joint_name==SHMJointName(i)) {
			openJointShm(i);
			ptr = (char*)ptrShm[i];
			break;
		}
	}

	if (ptr!=NULL) {
		double* element = (double*)(ptr + channel*SHMIF_DATA_SIZE);
		getJointStatus(element);
		vect.set(element[1], element[2], element[3]);
	}

	return vect;
}




Vector<double>  CSHMInterFace::getAnimationScale(std::string joint_name)
{
	Vector<double> vect(0.0, 0.0, 0.0);

	char* ptr = NULL;
	for (int i=0; i<SHMIF_JOINT_NUM; i++) {
		if (joint_name==SHMJointName(i)) {
			openJointShm(i);
			ptr = (char*)ptrShm[i];
			break;
		}
	}

	if (ptr!=NULL) {
		double* element = (double*)(ptr + channel*SHMIF_DATA_SIZE);
		getJointStatus(element);
	}

	return vect;
}




