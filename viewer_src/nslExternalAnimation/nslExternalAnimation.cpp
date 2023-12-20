/** 
 * nslExternalAnimation.cpp  v2.0
 *
 *		Copyright (c) 2011, Fumi.Iseki
 * 
 */

#include "linden_common.h"
#include "llcharacter.h"

#include "nslExternalAnimation.h"



using namespace nsl;



static  std::string  SLJointName[] =
{
	"mPelvis",
	"mTorso",
	"mChest",
	"mNeck",
	"mHead",
	"mCollarLeft",
	"mShoulderLeft",
	"mElbowLeft",
	"mWristLeft",
	"mCollarRight",
	"mShoulderRight",
	"mElbowRight",
	"mWristRight",
	"mHipLeft",
	"mKneeLeft",
	"mAnkleLeft",
	"mHipRight",
	"mKneeRight",
	"mAnkleRight"
}; 






void  ExternalAnimation::init(LLUUID id)
{
	uuid        = id;
	isExternal  = FALSE;
	indexHandle = NULL;
	ptrIndex    = NULL;

	for (int i=0; i<EXANIM_JOINT_NUM; i++) {
		mapHandle[i] = NULL;
		ptrShm[i]    = NULL;
	}

	//
	CreateIndexShm();
	CreateAllJointsShm();
}






/////////////////////////////////////////////////////////////////////////////////////
// Shared Memory of Joint

void 	ExternalAnimation::CreateAllJointsShm(void)
{
	if (!isExternal) return;

	for (int i=0; i<EXANIM_JOINT_NUM; i++) {
		CreateJointShm(i);
	}
}




void 	ExternalAnimation::CreateJointShm(int n)
{
	if (ptrShm[n]==NULL) {
		if (mapHandle[n]==NULL) {
			mapHandle[n] = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, EXANIM_DATA_SIZE*EXANIM_CHANNEL_NUM, SLJointName[n].c_str());
		}

		if (mapHandle[n]!=NULL) {
			ptrShm[n] = (double*)MapViewOfFile(mapHandle[n], FILE_MAP_READ, 0, 0, EXANIM_DATA_SIZE*EXANIM_CHANNEL_NUM);
		}
	}
}




void 	ExternalAnimation::OpenJointShm(int n)
{
	//if (n<0 || n>=EXANIM_JOINT_NUM	) return;
	
	if (ptrShm[n]==NULL) {
		if (mapHandle[n]==NULL) {
			mapHandle[n] = OpenFileMappingA(FILE_MAP_READ, FALSE, SLJointName[n].c_str());
		}

		if (mapHandle[n]!=NULL) {
			ptrShm[n] = (double*)MapViewOfFile(mapHandle[n], FILE_MAP_READ, 0, 0, EXANIM_DATA_SIZE	*EXANIM_CHANNEL_NUM);
		}
	}
}




void 	ExternalAnimation::CloseShm(void)
{
	CloseIndexShm();

	for (int i=0; i<EXANIM_JOINT_NUM	; i++) {
		if (ptrShm[i]!=NULL)    UnmapViewOfFile(ptrShm[i]);
		if (mapHandle[i]!=NULL) CloseHandle(mapHandle[i]);
		ptrShm[i]    = NULL;
		mapHandle[i] = NULL;
	}
}






/////////////////////////////////////////////////////////////////////////////////////
// Shred Memory of Animation UUID 

void    ExternalAnimation::CreateIndexShm()
{
	if (ptrIndex==NULL) {
		if (indexHandle==NULL) {
			indexHandle = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, EXANIM_INDEX_LEN*EXANIM_CHANNEL_NUM, EXANIM_INDEX_NAME);
		}

		if (indexHandle!=NULL) {
			ptrIndex = (char*)MapViewOfFile(indexHandle, FILE_MAP_READ, 0, 0, EXANIM_INDEX_LEN*EXANIM_CHANNEL_NUM);
		}
	}
}




void    ExternalAnimation::OpenIndexShm()
{
	if (ptrIndex==NULL) {
		if (indexHandle==NULL) {
			indexHandle = OpenFileMappingA(FILE_MAP_READ, FALSE, EXANIM_INDEX_NAME	);
		}

		if (indexHandle!=NULL) {
			ptrIndex = (char*)MapViewOfFile(indexHandle, FILE_MAP_READ, 0, 0, EXANIM_INDEX_LEN	*EXANIM_CHANNEL_NUM);
		}
	}
}




void 	ExternalAnimation::CloseIndexShm(void)
{
	if (ptrIndex!=NULL)    UnmapViewOfFile(ptrIndex);
	if (indexHandle!=NULL) CloseHandle(indexHandle);
	ptrIndex    = NULL;
	indexHandle = NULL;
}




int 	ExternalAnimation::GetChannelIndexShm(std::string uuid)
{
	OpenIndexShm();
	if (ptrIndex==NULL) return -1;

	int   n;
	char* ptr = ptrIndex;
	for (n=0; n<EXANIM_CHANNEL_NUM; n++) {
		if (!strncasecmp(uuid.c_str(), ptr, EXANIM_UUID_LEN	)) break;
		ptr += EXANIM_INDEX_LEN	;
	}

	if (n==EXANIM_CHANNEL_NUM) n = -1;
	return n;
}




void 	ExternalAnimation::CheckChannelIndexShm(void)
{
	channel = GetChannelIndexShm(uuid.asString());
	if (channel>=0) isExternal = TRUE;
	else            isExternal = FALSE;
}






/////////////////////////////////////////////////////////////////////////////////////
// Get Joint Data

LLQuaternion  ExternalAnimation::GetAnimationRotation(std::string joint_name)
{
	LLQuaternion quat(0.0, 0.0, 0.0, 1.0);

	char* ptr = NULL;
	for (int i=0; i<EXANIM_JOINT_NUM	; i++) {
		if (joint_name==SLJointName[i]) {
			OpenJointShm(i);
			ptr = (char*)ptrShm[i];
			break;
		}
	}

	if (ptr!=NULL) {
		double* element = (double*)(ptr + channel*EXANIM_DATA_SIZE);
		quat.set(element[4], element[5], element[6], element[7]);
	}

	return quat;
}




LLVector3  ExternalAnimation::GetAnimationPosition(std::string joint_name)
{
	LLVector3 vect(0.0, 0.0, 0.0);

	char* ptr = NULL;
	for (int i=0; i<EXANIM_JOINT_NUM	; i++) {
		if (joint_name==SLJointName[i]) {
			OpenJointShm(i);
			ptr = (char*)ptrShm[i];
			break;
		}
	}

	if (ptr!=NULL) {
		double* element = (double*)(ptr + channel*EXANIM_DATA_SIZE);
		vect.set(element[1], element[2], element[3]);
	}

	return vect;
}




LLVector3  ExternalAnimation::GetAnimationScale(std::string joint_name)
{
	return LLVector3(0.0, 0.0, 0.0);
}




