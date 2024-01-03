
#include "stdafx.h"

#include  "ExportNiData.h"






//////////////////////////////////////////////////////////////////////////////////////////
//
//
//

void  CExportNiData::init(void)
{
	clear();
	outPeriod = NI_FPS_MAX;		// 30ms
}




void  CExportNiData::clear(void)
{
	outTimer   = 0;
	modTime    = 0;
	frameRate  = 0.0f;
	frameTimer = 0;
	frameNum   = 0;

	ctrlFPS    = TRUE;
	sendLogNet = FALSE;	
}



BOOL  CExportNiData::exportJointsData(Vector<float>* posVect, Quaternion<float>* rotQuat, float* jntAngl, CExNiNetwork* net, NiSDK_Lib lib, int joint_num)
{
	if ((!ctrlFPS || checkOutputPeriod()) && net!=NULL) {
		exportData(posVect, rotQuat, jntAngl, net, lib, joint_num);		// データを出力
		calcLocalFPS();													// FPS の計算
		return TRUE;
	}

	return FALSE;
}




//
//
//
/*
void  CExportNiData::exportJointsLog(Vector<double> currentPos, double* jointAngle, CExNiNetwork* net, NiSDK_Lib lib, int joint_num)
{
//	if ((!ctrlFPS || checkOutputPeriod()) && net!=NULL) {
	if (net!=NULL) {
		if (net->sendSocket>0) {
			sendAnimationLog<float>(currentPos, jointAngle, net, lib, joint_num);
		}
	}
}
*/



void  CExportNiData::exportJointsLogStart(CExNiNetwork* net, NiSDK_Lib lib, int joint_num)
{
//	if ((!ctrlFPS || checkOutputPeriod()) && net!=NULL) {
	if (net!=NULL) {
		if (net->sendSocket>0) {
			net->sendAnimationLogStart(lib, joint_num);
		}
	}
}




void  CExportNiData::exportJointsLogStop(CExNiNetwork* net)
{
//	if ((!ctrlFPS || checkOutputPeriod()) && net!=NULL) {
	if (net!=NULL) {
		if (net->sendSocket>0) {
			net->sendAnimationLogStop();
		}
	}
}







//////////////////////////////////////////////////////////////////////////////////////////
//
//
//

BOOL  CExportNiData::checkOutputPeriod(void)
{
	unsigned short tmptimer;
	int laptm = (int)GetMsecondsLapTimer(outTimer, &tmptimer) - outPeriod;

	if (laptm+modTime<0) return FALSE;

	//if (laptm>0) modTime = laptm;
	//else         modTime = 0;
	modTime  = Min(laptm + modTime, outPeriod);
	outTimer = tmptimer;

	return TRUE;
}




void  CExportNiData::calcLocalFPS(void)
{
	unsigned short tmptimer;
	unsigned short laptm = GetMsecondsLapTimer(frameTimer, &tmptimer)/1000;

	frameNum++;

	if (laptm>=NINET_FRM_RATE_INTVL) {
		frameRate  = (float)frameNum/laptm;
		frameTimer = tmptimer;
		frameNum   = 0;
		SendWinMessage(NISHM_WM_LCLFPS_UPDATE);
	}

	return;
}




//
// 共有メモリとネットワークにデータを出力する．
//
void  CExportNiData::exportData(Vector<float>* posVect, Quaternion<float>* rotQuat, float* jntAngl, CExNiNetwork* net, NiSDK_Lib lib, int joint_num)
{
	CExNiSHMemory*  mem = net->sharedMem;
	CParameterSet param = net->appParam;

	//
	if (net->sendSocket>0) {
		if (param.netOutMode==NETandLOCAL) {
			mem->updateLocalAnimationData(posVect, rotQuat, lib, joint_num);
		}

		if (!sendLogNet || jntAngl==NULL) {
			net->sendAnimationData(posVect, rotQuat, NULL, lib, joint_num, TRUE);		// Data Only
		}
		else if (param.netLogOnly && param.netOutMode==NETandLOCAL) {
			net->sendAnimationData(posVect, rotQuat, jntAngl, lib, joint_num, FALSE);	// Log Only
		}
		else {
			net->sendAnimationData(posVect, rotQuat, jntAngl, lib, joint_num, TRUE);	// Data and Log
		}
	}

	else {
		mem->updateLocalAnimationData(posVect, rotQuat, lib, joint_num);
	}
}


