#include "StdAfx.h"


#if defined(ENABLE_OPENNI) || defined(ENABLE_OPENNI2)



#include  "ExOpenNiWin.h"
#include  "WinTools.h"




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CExOpenNiWin クラス
//

CExOpenNiWin::CExOpenNiWin(NiSDK_Lib lib) : COpenNiWin(lib)
{
	niFileDev		= NULL;

	niJoints		= NULL;
	niNetwork		= NULL;
	sharedMem		= NULL;
	niExportData	= NULL;

	pLogFrame		= NULL;
	pSensorFrame	= NULL;
	pDepthFrame		= NULL;
	pModelFrame		= NULL;
	pLogDoc			= NULL;

	isDetectFace	= FALSE;
	dataSaving		= FALSE;
	logfSaving		= FALSE;
	logfSending		= FALSE;
	hasAngleData	= FALSE;

	vect_fwrd.set(1.0, 0.0, 0,0, 1.0);

	appParam.init();
	tempFilePath = makeTempFilePath();
}




CExOpenNiWin::~CExOpenNiWin()
{
	DEBUG_INFO("DESTRUCTOR: CExOpenNiWin: START");
	deleteBackupDev();

//	dev_backup		= NULL;
	niJoints		= NULL;
	niNetwork		= NULL;
	sharedMem		= NULL;
	pLogFrame		= NULL;
	pSensorFrame	= NULL;
	pDepthFrame		= NULL;
	pModelFrame		= NULL;
	pLogDoc			= NULL;

	DEBUG_INFO("DESTRUCTOR: CExOpenNiWin: END");
}




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//

CString  CExOpenNiWin::makeTempFilePath(void)
{		
	return MakeWorkingFolderPath(_T(OPENNI_RECORDE_TEMP_FILE), TRUE, _T("\\NSL\\Rinions\\"), TRUE);
}



void  CExOpenNiWin::setLogFramePtr(CExTextFrame* pfrm)
{
	pLogFrame = pfrm;
	
	if (pfrm!=NULL) pLogDoc = (CLogWndDoc*)(((CLogWndFrame*)pLogFrame)->pDoc);
	else            pLogDoc = NULL;
}



void  CExOpenNiWin::setParameter(CParameterSet param)
{ 
	appParam  = param;

	m_use_nite_smth = param.useNiteSmooth;
	m_confidence	= param.confidence;
	m_nite_smooth	= param.smoothNITE;
	m_profile		= param.detectParts;
}






///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//

BOOL  CExOpenNiWin::makeFileDevice(LPCTSTR fname)
{
	BOOL ret = FALSE;

	if (hasBackupDev()) {
		ret = remakeFileDevice(fname);
	}
	else {
		ret = backupDevice();
		if (ret) {
			char* mbstr = ts2mbs(fname);
			ret = initDevice(mbstr, TRUE);
			::free(mbstr);
			if (!ret) restoreDevice();
		}
	}

	return ret;
}



BOOL  CExOpenNiWin::remakeFileDevice(LPCTSTR fname)
{
	char* mbstr = ts2mbs(fname);
	BOOL ret = resetDevice(mbstr, TRUE);
	::free(mbstr);
	return ret;
}






///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//

BOOL  CExOpenNiWin::startLogTransfer(void)
{
	if (logfSending) return logfSending;

	if (niExportData!=NULL) {
		niExportData->sendLogNet = appParam.sendLogNet;
		if (niExportData->sendLogNet) {
			niExportData->exportJointsLogStart(niNetwork, m_library, NI_TOTAL_JOINT_NUM);
			logfSending = TRUE;
		}
	}

	return logfSending;
}




void  CExOpenNiWin::stopLogTransfer(void)
{
	if (!logfSending) return;

	if (niExportData!=NULL) {
		niExportData->sendLogNet = appParam.sendLogNet;
		if (niExportData->sendLogNet) {
			niExportData->exportJointsLogStop(niNetwork);
		}
	}

	logfSending = FALSE;
	return;
}





///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Virtual Function
//

/////////////////////////////////////////////////
//
// ジョイントデータの処理と出力
//
void  CExOpenNiWin::convertJointsData(void)
{
	hasAngleData = FALSE;
	//
	if (appParam.usePosData) {
		if (appParam.detectParts==XN_SKEL_PROFILE_ALL) {
			convertPos2JointsData();
		}
		else if (appParam.detectParts==XN_SKEL_PROFILE_UPPER) {
			convertUpperPos2JointsData();
		}
		else {
			return;
		}
	}
	else {
		convertRot2JointsData();
	}

	// Additional Processing
	if (niJoints!=NULL) {
		niJoints->copyJoints2NiJoints(m_is_mirroring);
		niJoints->niPosVect[NI_AVATAR] = currentPos;
		
		//Face Tracking
		if (appParam.useFaceDetect) {
			niJoints->execFaceTracking();
			isDetectFace = niJoints->isDetectFace;
		}
	}

	//
	if (pSensorFrame==NULL) return;

	//
	// データ出力
	if (niExportData!=NULL) {
		if (niJoints!=NULL) {
			niExportData->sendLogNet = hasAngleData && appParam.sendLogNet;
			if (niExportData->sendLogNet && logfSending) {
				niExportData->exportJointsData(niJoints->niPosVect, niJoints->niRotQuat, niJoints->niJntAngl, niNetwork, NiSDK_None, NI_TOTAL_JOINT_NUM);
			}
			else {
				niExportData->exportJointsData(niJoints->niPosVect, niJoints->niRotQuat, NULL, niNetwork, NiSDK_None, NI_TOTAL_JOINT_NUM);
			}
			
			/*
			if (ret && niExportData->sendLogNet && logfSending) {
				Vector<double> pos(-currentPos.y, currentPos.z, -currentPos.x);
				if (m_is_mirroring) pos.x = -pos.x;
				niExportData->exportJointsLog(pos, niJoints->niJntAngl, niNetwork, m_library, NI_TOTAL_JOINT_NUM);
			}*/
		}
	}
}




void  CExOpenNiWin::saveJointsData(void)
{
	if (dataSaving) {
		//
		if (niFileDev!=NULL) {
			if (niJoints!=NULL) {
				niFileDev->writeTempJText(niJoints->niPosVect, niJoints->niRotQuat, niJoints->niJntAngl, m_is_mirroring, NiSDK_None);
			}
			else {
				niFileDev->writeTempJText(posVect, rotQuat, jntAngl, m_is_mirroring);
			}
		}
	}
}




void  CExOpenNiWin::loggingJointsData(void)
{
	// CRITICAL SECTION にするべきか?
	//
	
	//
	// Log File
	if (logfSaving && hasAngleData) {
		if (niFileDev!=NULL && niJoints!=NULL) {
			niFileDev->writeLogData(niJoints->niPosVect, niJoints->niRotQuat, niJoints->niJntAngl, m_is_mirroring, NiSDK_None);
		}
	}


	//
	// Log Window
	if (pLogDoc!=NULL) {
		pLogDoc->lock();

		//
		for (int j=0; j<OPENNI_JOINT_NUM; j++) {
			XnVector3D  pos = joint_PositionData(j);
			XnMatrix3X3 rot = joint_RotationData(j);
			
			int n = j;
			if (m_is_mirroring) n = NiSDKMirrorJointNum(j, m_library);
			//
			if (appParam.printPostnMode || appParam.printRotMxMode || appParam.printQuateMode) {
				pLogDoc->printFormat("LOCAL: %s (%2d)\n", NiSDK2JointName(n, m_library).c_str(), j);
			}
			if (appParam.printPostnMode) {
				pLogDoc->printFormat("LOCAL: POS1: %f, %f, %f\n", pos.X, pos.Y, pos.Z);
				pLogDoc->printFormat("LOCAL: POS2: %f, %f, %f\n", posVect[n].x, posVect[n].y, posVect[n].z);
			}
			if (appParam.printRotMxMode) {
				pLogDoc->printFormat("LOCAL: ROT1: %f, %f, %f\n", rot.elements[0], rot.elements[1], rot.elements[2]);
				pLogDoc->printFormat("LOCAL: ROT2: %f, %f, %f\n", rot.elements[3], rot.elements[4], rot.elements[5]);
				pLogDoc->printFormat("LOCAL: ROT3: %f, %f, %f\n", rot.elements[6], rot.elements[7], rot.elements[8]);
			}
			if (appParam.printQuateMode) {
				pLogDoc->printFormat("LOCAL: QUAT: %f, %f, %f, %f\n", rotQuat[n].x, rotQuat[n].y, rotQuat[n].z, rotQuat[n].s);
			}
			if (appParam.printAngleMode) {
				pLogDoc->printFormat("LOCAL: ANGL: %f\n", jntAngl[j]);
			}
		}

		// Special Joints
		if (niJoints!=NULL) {
			for (int j=0; j<NI_SPECL_JOINT_NUM; j++) {
				int n = NI_NORML_JOINT_NUM + j;
				if (m_is_mirroring) n = NiSDKMirrorJointNum(n, NiSDK_None);

				if (appParam.printPostnMode || appParam.printRotMxMode || appParam.printQuateMode) {
					pLogDoc->printFormat("LOCAL: %s (%2d)\n", NiSDK2JointName(n, NiSDK_None).c_str(), NI_NORML_JOINT_NUM+j);
				}
			
				if (appParam.printPostnMode) {
					Vector<double> vect = niJoints->niPosVect[n];
					pLogDoc->printFormat("LOCAL: POS : %f, %f, %f\n", vect.x, vect.y, vect.z);
				}
				if (appParam.printQuateMode) {
					Quaternion quat = niJoints->niRotQuat[n];
					pLogDoc->printFormat("LOCAL: QUAT: %f, %f, %f, %f\n", quat.x, quat.y, quat.z, quat.s);
				}
			}
		}

		//
		pLogDoc->unlock();
	}
}


	
void  CExOpenNiWin::drawAddition(int col, int line)
{
	// Face Tracking
	if (niJoints!=NULL) {
		if (appParam.useFaceDetect) {
			if (isDetectFace) niJoints->drawFaceTracking(col, line);
		}
	}
}




////////////////////////////////////////////////////

void  CExOpenNiWin::convertPos2JointsData(void)
{
	//
	for (int j=0; j<OPENNI_JOINT_NUM; j++) {
		if (posVect[j].c<m_confidence) {
			Vector<double>* ptr = (Vector<double>*)posRing[j].get(-1);
			if (ptr!=NULL && ptr->c>0.0) posVect[j] = *ptr;
			else posVect[j].init(-1.0);
		}
	}

	//
	if (appParam.useMvavSmooth) niJoints->PosMovingAverage(m_library);
	if (appParam.useJointConst) niJoints->PosVibNoiseCanceler();

	//
	Vector<double> torso_down  = posVect[NI_SDK_PELVIS]  - posVect[NI_SDK_TORSO];
	Vector<double> shldr_left  = posVect[NI_SDK_L_SHLDR] - posVect[NI_SDK_R_SHLDR];
	Vector<double> torso_up	   = - torso_down;
	Vector<double> shldr_right = - shldr_left;

	rotQuat[NI_SDK_PELVIS].init();

	// PELVIS YZ軸の回転
	if (posVect[NI_SDK_R_HIP].c>=m_confidence && posVect[NI_SDK_L_HIP].c>=m_confidence) { 
		Vector<double> vect = NewellMethod4(posVect[NI_SDK_R_HIP], posVect[NI_SDK_PELVIS], posVect[NI_SDK_L_HIP], posVect[NI_SDK_TORSO]);
		
		rotQuat[NI_SDK_PELVIS] = V2VQuaternion(vect_fwrd, vect);
		//if (rotQuat[NI_SDK_PELVIS].c==-1.0 && rotQuat[NI_SDK_PELVIS].s==0.0 && rotQuat[NI_SDK_PELVIS].n==0.0) {
		//	rotQuat[NI_SDK_PELVIS].setRotation(PI, torso_up);
		//}
	}

	// X軸回転
	Quaternion quat_lhip = ~rotQuat[NI_SDK_PELVIS]*posVect[NI_SDK_L_HIP]*rotQuat[NI_SDK_PELVIS];
	Quaternion quat_rhip = ~rotQuat[NI_SDK_PELVIS]*posVect[NI_SDK_R_HIP]*rotQuat[NI_SDK_PELVIS];

	Vector<double> hip_left  = quat_lhip.getVector() - quat_rhip.getVector(); 
	Vector<double> hip_right = - hip_left;
	double thx = atan2(hip_left.z, hip_left.y);
	Quaternion xrot;
	xrot.setRotation(thx, 1.0, 0.0, 0.0, 1.0);

	// Y軸回転補正
	Quaternion yrot;
	yrot.setRotation((double)appParam.YaxisCorrect, 0.0, 1.0, 0.0, 1.0);

	//
	rotQuat[NI_SDK_PELVIS]  = rotQuat[NI_SDK_PELVIS]*xrot*yrot;

	//
	rotQuat[NI_SDK_TORSO]   = VPPQuaternion(torso_up, posVect[NI_SDK_TORSO], posVect[NI_SDK_NECK]);
	rotQuat[NI_SDK_NECK]    = PPPQuaternion(posVect[NI_SDK_TORSO],  posVect[NI_SDK_NECK],  posVect[NI_SDK_HEAD]);

	// To Relative Coordinate
	rotQuat[NI_SDK_L_SHLDR] = VPPQuaternion(shldr_left,  posVect[NI_SDK_L_SHLDR], posVect[NI_SDK_L_ELBOW]);
	rotQuat[NI_SDK_R_SHLDR] = VPPQuaternion(shldr_right, posVect[NI_SDK_R_SHLDR], posVect[NI_SDK_R_ELBOW]);
	rotQuat[NI_SDK_L_HIP]	= VPPQuaternion(torso_down,	 posVect[NI_SDK_L_HIP],   posVect[NI_SDK_L_KNEE]);
	rotQuat[NI_SDK_R_HIP]	= VPPQuaternion(torso_down,  posVect[NI_SDK_R_HIP],   posVect[NI_SDK_R_KNEE]);
	//
	rotQuat[NI_SDK_L_ELBOW] = PPPQuaternion(posVect[NI_SDK_L_SHLDR], posVect[NI_SDK_L_ELBOW], posVect[NI_SDK_L_WRIST]);
	rotQuat[NI_SDK_R_ELBOW] = PPPQuaternion(posVect[NI_SDK_R_SHLDR], posVect[NI_SDK_R_ELBOW], posVect[NI_SDK_R_WRIST]);
	rotQuat[NI_SDK_L_KNEE]	= PPPQuaternion(posVect[NI_SDK_L_HIP],	 posVect[NI_SDK_L_KNEE],  posVect[NI_SDK_L_ANKLE]);
	rotQuat[NI_SDK_R_KNEE]	= PPPQuaternion(posVect[NI_SDK_R_HIP],	 posVect[NI_SDK_R_KNEE],  posVect[NI_SDK_R_ANKLE]);

	// To Avatar Coordinate
	rotQuat[NI_SDK_L_ELBOW] = ~rotQuat[NI_SDK_PELVIS]*rotQuat[NI_SDK_L_ELBOW]*rotQuat[NI_SDK_PELVIS];
	rotQuat[NI_SDK_R_ELBOW] = ~rotQuat[NI_SDK_PELVIS]*rotQuat[NI_SDK_R_ELBOW]*rotQuat[NI_SDK_PELVIS];
	rotQuat[NI_SDK_L_SHLDR] = ~rotQuat[NI_SDK_PELVIS]*rotQuat[NI_SDK_L_SHLDR]*rotQuat[NI_SDK_PELVIS];
	rotQuat[NI_SDK_R_SHLDR] = ~rotQuat[NI_SDK_PELVIS]*rotQuat[NI_SDK_R_SHLDR]*rotQuat[NI_SDK_PELVIS];
	rotQuat[NI_SDK_L_KNEE]	= ~rotQuat[NI_SDK_PELVIS]*rotQuat[NI_SDK_L_KNEE] *rotQuat[NI_SDK_PELVIS];
	rotQuat[NI_SDK_R_KNEE]	= ~rotQuat[NI_SDK_PELVIS]*rotQuat[NI_SDK_R_KNEE] *rotQuat[NI_SDK_PELVIS];
	rotQuat[NI_SDK_L_HIP]	= ~rotQuat[NI_SDK_PELVIS]*rotQuat[NI_SDK_L_HIP]	 *rotQuat[NI_SDK_PELVIS];
	rotQuat[NI_SDK_R_HIP]	= ~rotQuat[NI_SDK_PELVIS]*rotQuat[NI_SDK_R_HIP]	 *rotQuat[NI_SDK_PELVIS];
	rotQuat[NI_SDK_NECK]	= ~rotQuat[NI_SDK_PELVIS]*rotQuat[NI_SDK_NECK]	 *rotQuat[NI_SDK_PELVIS];
	rotQuat[NI_SDK_TORSO]   = ~rotQuat[NI_SDK_PELVIS]*rotQuat[NI_SDK_TORSO]	 *rotQuat[NI_SDK_PELVIS];


	//
	for (int j=0; j<OPENNI_JOINT_NUM; j++) {
		if (rotQuat[j].c<m_confidence) {
			Quaternion* ptr = (Quaternion*)rotRing[j].get(-1);
			if (ptr!=NULL && ptr->c>0.0) rotQuat[j] = *ptr;
			else rotQuat[j].init();
		}
	}

	//
	if (appParam.useJointConst) niJoints->CheckJointsRotation();

	//
	// Angle between Joints
	if (appParam.printAngleMode || appParam.saveLogLocal || appParam.sendLogNet) {
		hasAngleData = TRUE;
		Vector<double> upz(0.0, 0.0, 1.0, 1.0);
		//
		jntAngl[NI_SDK_PELVIS]  = VectorAngle(upz, torso_up);
		jntAngl[NI_SDK_TORSO]   = VectorAngle(torso_up,  posVect[NI_SDK_NECK] - posVect[NI_SDK_TORSO]);
		jntAngl[NI_SDK_NECK]    = VectorAngle(posVect[NI_SDK_TORSO], posVect[NI_SDK_NECK],  posVect[NI_SDK_HEAD]);
		//
		jntAngl[NI_SDK_L_SHLDR] = VectorAngle(shldr_left,  posVect[NI_SDK_L_ELBOW] - posVect[NI_SDK_L_SHLDR]);
		jntAngl[NI_SDK_R_SHLDR] = VectorAngle(shldr_right, posVect[NI_SDK_R_ELBOW] - posVect[NI_SDK_R_SHLDR]);
		jntAngl[NI_SDK_L_HIP]	= VectorAngle(torso_down,  posVect[NI_SDK_L_KNEE]  - posVect[NI_SDK_L_HIP]);
		jntAngl[NI_SDK_R_HIP]	= VectorAngle(torso_down,  posVect[NI_SDK_R_KNEE]  - posVect[NI_SDK_R_HIP]);

		//
		if (posVect[NI_SDK_L_WRIST].c>0.0) jntAngl[NI_SDK_L_ELBOW] = VectorAngle(posVect[NI_SDK_L_SHLDR], posVect[NI_SDK_L_ELBOW], posVect[NI_SDK_L_WRIST]);
		else							   jntAngl[NI_SDK_L_ELBOW] = 0.0;
		if (posVect[NI_SDK_R_WRIST].c>0.0) jntAngl[NI_SDK_R_ELBOW] = VectorAngle(posVect[NI_SDK_R_SHLDR], posVect[NI_SDK_R_ELBOW], posVect[NI_SDK_R_WRIST]);
		else							   jntAngl[NI_SDK_R_ELBOW] = 0.0;
		//
		if (posVect[NI_SDK_L_ANKLE].c>0.0) jntAngl[NI_SDK_L_KNEE] = VectorAngle(posVect[NI_SDK_L_HIP], posVect[NI_SDK_L_KNEE], posVect[NI_SDK_L_ANKLE]);
		else							   jntAngl[NI_SDK_L_KNEE] = 0.0;
		if (posVect[NI_SDK_R_ANKLE].c>0.0) jntAngl[NI_SDK_R_KNEE] = VectorAngle(posVect[NI_SDK_R_HIP], posVect[NI_SDK_R_KNEE], posVect[NI_SDK_R_ANKLE]);
		else							   jntAngl[NI_SDK_R_KNEE] = 0.0;
	}

	return;
}





void  CExOpenNiWin::convertUpperPos2JointsData(void)
{
	//
	for (int j=0; j<OPENNI_JOINT_NUM; j++) {
		if (posVect[j].c<m_confidence) {
			Vector<double>* ptr = (Vector<double>*)posRing[j].get(-1);
			if (ptr!=NULL && ptr->c>0.0) posVect[j] = *ptr;
			else posVect[j].init(-1.0);
		}
	}

	//
	if (appParam.useMvavSmooth) niJoints->PosMovingAverage(m_library);
	if (appParam.useJointConst) niJoints->PosVibNoiseCanceler();

	//
	Vector<double> torso_down  = posVect[NI_SDK_PELVIS]  - posVect[NI_SDK_TORSO];
	Vector<double> shldr_left  = posVect[NI_SDK_L_SHLDR] - posVect[NI_SDK_R_SHLDR];
	Vector<double> torso_up	   = - torso_down;
	Vector<double> shldr_right = - shldr_left;

	//
	rotQuat[NI_SDK_PELVIS].init();
	posVect[NI_SDK_PELVIS].init();

	// PELVIS YZ軸の回転
	if (posVect[NI_SDK_R_SHLDR].c>=m_confidence && posVect[NI_SDK_L_SHLDR].c>=m_confidence) { 
		Vector<double> vect = NewellMethod4(posVect[NI_SDK_R_SHLDR], posVect[NI_SDK_TORSO], posVect[NI_SDK_L_SHLDR], posVect[NI_SDK_NECK]);
		
		rotQuat[NI_SDK_PELVIS] = V2VQuaternion(vect_fwrd, vect);
		//if (rotQuat[NI_SDK_PELVIS].c==-1.0 && rotQuat[NI_SDK_PELVIS].s==0.0 && rotQuat[NI_SDK_PELVIS].n==0.0) {
		//	rotQuat[NI_SDK_PELVIS].setRotation(PI, torso_up);
		//}
	}

	// X 軸回転
	Quaternion quat_lshd = ~rotQuat[NI_SDK_PELVIS]*posVect[NI_SDK_L_SHLDR]*rotQuat[NI_SDK_PELVIS];
	Quaternion quat_rshd = ~rotQuat[NI_SDK_PELVIS]*posVect[NI_SDK_R_SHLDR]*rotQuat[NI_SDK_PELVIS];

	Vector<double> shd_left  = quat_lshd.getVector() - quat_rshd.getVector(); 
	Vector<double> shd_right = - shd_left;
	double thx = atan2(shd_left.z, shd_left.y);
	Quaternion xrot;
	xrot.setRotation(thx, 1.0, 0.0, 0.0, 1.0);

	// Y軸回転補正
	Quaternion yrot;
	yrot.setRotation((double)appParam.YaxisCorrect, 0.0, 1.0, 0.0, 1.0);

	//
	rotQuat[NI_SDK_PELVIS] = rotQuat[NI_SDK_PELVIS]*xrot*yrot;


	/////////////////////////
	//
	rotQuat[NI_SDK_TORSO]  = VPPQuaternion(torso_up, posVect[NI_SDK_TORSO], posVect[NI_SDK_NECK]);
	rotQuat[NI_SDK_NECK]   = PPPQuaternion(posVect[NI_SDK_TORSO],  posVect[NI_SDK_NECK],  posVect[NI_SDK_HEAD]);

	// To Relative Coordinate
	rotQuat[NI_SDK_L_SHLDR] = VPPQuaternion(shldr_left,  posVect[NI_SDK_L_SHLDR], posVect[NI_SDK_L_ELBOW]);
	rotQuat[NI_SDK_R_SHLDR] = VPPQuaternion(shldr_right, posVect[NI_SDK_R_SHLDR], posVect[NI_SDK_R_ELBOW]);
	//
	rotQuat[NI_SDK_L_ELBOW] = PPPQuaternion(posVect[NI_SDK_L_SHLDR], posVect[NI_SDK_L_ELBOW], posVect[NI_SDK_L_WRIST]);
	rotQuat[NI_SDK_R_ELBOW] = PPPQuaternion(posVect[NI_SDK_R_SHLDR], posVect[NI_SDK_R_ELBOW], posVect[NI_SDK_R_WRIST]);

	//
	Quaternion pelvisXY;
	Vector<double> euler = Quaternion2EulerXYZ(rotQuat[NI_SDK_PELVIS]);
	euler.z = 0.0;
	pelvisXY.setEulerXYZ(euler);

	// To Avatar Coordinate
	rotQuat[NI_SDK_L_ELBOW] = ~rotQuat[NI_SDK_PELVIS]*rotQuat[NI_SDK_L_ELBOW]*rotQuat[NI_SDK_PELVIS];
	rotQuat[NI_SDK_R_ELBOW] = ~rotQuat[NI_SDK_PELVIS]*rotQuat[NI_SDK_R_ELBOW]*rotQuat[NI_SDK_PELVIS];
	rotQuat[NI_SDK_L_SHLDR] = ~rotQuat[NI_SDK_PELVIS]*rotQuat[NI_SDK_L_SHLDR]*rotQuat[NI_SDK_PELVIS];
	rotQuat[NI_SDK_R_SHLDR] = ~rotQuat[NI_SDK_PELVIS]*rotQuat[NI_SDK_R_SHLDR]*rotQuat[NI_SDK_PELVIS];
	rotQuat[NI_SDK_NECK]	= ~rotQuat[NI_SDK_PELVIS]*rotQuat[NI_SDK_NECK]	 *rotQuat[NI_SDK_PELVIS];
	rotQuat[NI_SDK_TORSO]   = ~rotQuat[NI_SDK_PELVIS]*rotQuat[NI_SDK_TORSO]	 *rotQuat[NI_SDK_PELVIS];

	//
	rotQuat[NI_SDK_L_KNEE].init();
	rotQuat[NI_SDK_R_KNEE].init();
	rotQuat[NI_SDK_R_HIP]   = ~pelvisXY;
	rotQuat[NI_SDK_L_HIP]   = ~pelvisXY;


	//
	for (int j=0; j<OPENNI_JOINT_NUM; j++) {
		if (rotQuat[j].c<m_confidence) {
			Quaternion* ptr = (Quaternion*)rotRing[j].get(-1);
			if (ptr!=NULL && ptr->c>0.0) rotQuat[j] = *ptr;
			else rotQuat[j].init();
		}
	}

	//
	if (appParam.useJointConst) niJoints->CheckJointsRotation();

	//
	// Angle between Joints
	if (appParam.printAngleMode || appParam.saveLogLocal || appParam.sendLogNet) {
		hasAngleData = TRUE;
		Vector<double> upz(0.0, 0.0, 1.0, 1.0);
		//
		jntAngl[NI_SDK_PELVIS]  = VectorAngle(upz, torso_up);
		jntAngl[NI_SDK_TORSO]   = VectorAngle(torso_up,  posVect[NI_SDK_NECK] - posVect[NI_SDK_TORSO]);
		jntAngl[NI_SDK_NECK]    = VectorAngle(posVect[NI_SDK_TORSO], posVect[NI_SDK_NECK],  posVect[NI_SDK_HEAD]);
		//
		jntAngl[NI_SDK_L_SHLDR] = VectorAngle(shldr_left,  posVect[NI_SDK_L_ELBOW] - posVect[NI_SDK_L_SHLDR]);
		jntAngl[NI_SDK_R_SHLDR] = VectorAngle(shldr_right, posVect[NI_SDK_R_ELBOW] - posVect[NI_SDK_R_SHLDR]);

		//
		if (posVect[NI_SDK_L_WRIST].c>0.0) jntAngl[NI_SDK_L_ELBOW] = VectorAngle(posVect[NI_SDK_L_SHLDR], posVect[NI_SDK_L_ELBOW], posVect[NI_SDK_L_WRIST]);
		else							   jntAngl[NI_SDK_L_ELBOW] = 0.0;
    	if (posVect[NI_SDK_R_WRIST].c>0.0) jntAngl[NI_SDK_R_ELBOW] = VectorAngle(posVect[NI_SDK_R_SHLDR], posVect[NI_SDK_R_ELBOW], posVect[NI_SDK_R_WRIST]);
		else							   jntAngl[NI_SDK_R_ELBOW] = 0.0;
	}

	return;
}





void  CExOpenNiWin::convertRot2JointsData(void)
{
	//
	if (posVect[NI_SDK_R_HIP].c<m_confidence) {
		Vector<double>* ptr = (Vector<double>*)posRing[NI_SDK_R_HIP].get(-1);
		if (ptr!=NULL && ptr->c>0.0) posVect[NI_SDK_R_HIP] = *ptr;
		else posVect[NI_SDK_R_HIP].init(-1.0);
	}
	if (posVect[NI_SDK_L_HIP].c<m_confidence) {
		Vector<double>* ptr = (Vector<double>*)posRing[NI_SDK_L_HIP].get(-1);
		if (ptr!=NULL && ptr->c>0.0) posVect[NI_SDK_L_HIP] = *ptr;
		else posVect[NI_SDK_L_HIP].init(-1.0);
	}

	//
	Vector<double> torso_down  = posVect[NI_SDK_PELVIS]  - posVect[NI_SDK_TORSO];
	Vector<double> shldr_left  = posVect[NI_SDK_L_SHLDR] - posVect[NI_SDK_R_SHLDR];
	Vector<double> torso_up	   = - torso_down;
	Vector<double> shldr_right = - shldr_left;

	// PELVIS YZ平面の回転
	if (posVect[NI_SDK_R_HIP].c>=m_confidence && posVect[NI_SDK_L_HIP].c>=m_confidence) { 
		Vector<double> vect = NewellMethod4(posVect[NI_SDK_R_HIP], posVect[NI_SDK_PELVIS], posVect[NI_SDK_L_HIP], posVect[NI_SDK_TORSO]);
		
		rotQuat[NI_SDK_PELVIS]  = V2VQuaternion(vect_fwrd, vect);
		if (rotQuat[NI_SDK_PELVIS].c==-1.0 && rotQuat[NI_SDK_PELVIS].s==0.0 && rotQuat[NI_SDK_PELVIS].n==0.0) {
			rotQuat[NI_SDK_PELVIS].setRotation(PI, torso_up);
		}
	}
	else {
		rotQuat[NI_SDK_PELVIS].init();
	}

	// X 軸回転（座標変換）
	Quaternion quat_lhip = ~rotQuat[NI_SDK_PELVIS]*posVect[NI_SDK_L_HIP]*rotQuat[NI_SDK_PELVIS];
	Quaternion quat_rhip = ~rotQuat[NI_SDK_PELVIS]*posVect[NI_SDK_R_HIP]*rotQuat[NI_SDK_PELVIS];

	Vector<double> hip_left = quat_lhip.getVector() - quat_rhip.getVector(); 
	Vector<double> hip_right = - hip_left;
	double thx = atan2(hip_left.z, hip_left.y);
	Quaternion xrot;
	xrot.setRotation(thx, 1.0, 0.0, 0.0, 1.0);

	//
	rotQuat[NI_SDK_PELVIS]  = rotQuat[NI_SDK_PELVIS]*xrot;
	
	//
	rotQuat[NI_SDK_R_KNEE]  = rotQuat[NI_SDK_R_KNEE] *~rotQuat[NI_SDK_R_HIP];
	rotQuat[NI_SDK_L_KNEE]  = rotQuat[NI_SDK_L_KNEE] *~rotQuat[NI_SDK_L_HIP];
	rotQuat[NI_SDK_R_ELBOW] = rotQuat[NI_SDK_R_ELBOW]*~rotQuat[NI_SDK_R_SHLDR];
	rotQuat[NI_SDK_L_ELBOW] = rotQuat[NI_SDK_L_ELBOW]*~rotQuat[NI_SDK_L_SHLDR];

	rotQuat[NI_SDK_R_SHLDR] = rotQuat[NI_SDK_R_SHLDR]*~rotQuat[NI_SDK_NECK];
	rotQuat[NI_SDK_L_SHLDR] = rotQuat[NI_SDK_L_SHLDR]*~rotQuat[NI_SDK_NECK];
	rotQuat[NI_SDK_R_HIP]   = rotQuat[NI_SDK_R_HIP]  *~rotQuat[NI_SDK_PELVIS];
	rotQuat[NI_SDK_L_HIP]   = rotQuat[NI_SDK_L_HIP]  *~rotQuat[NI_SDK_PELVIS];
	rotQuat[NI_SDK_NECK]    = rotQuat[NI_SDK_NECK]   *~rotQuat[NI_SDK_TORSO];
	rotQuat[NI_SDK_TORSO]   = rotQuat[NI_SDK_TORSO]  *~rotQuat[NI_SDK_PELVIS];


	// Confidence
	for (int j=0; j<OPENNI_JOINT_NUM; j++) {
		if (rotQuat[j].c<m_confidence) {
			Quaternion* ptr = (Quaternion*)rotRing[j].get(-1);
			if (ptr!=NULL && ptr->c>0.0) rotQuat[j] = *ptr;
			else rotQuat[j].init();
		}
	}

	//
	if (appParam.useMvavSmooth) niJoints->RotMovingAverage(m_library);
	if (appParam.useJointConst) {
		niJoints->RotVibNoiseCanceler();
		niJoints->CheckJointsRotation();
	}
}



void  CExOpenNiWin::lostTrackingUser(int uid)
{ 
	DEBUG_INFO("CExOpenNiWin::lostTrackingUser():   LOST   TRACKING USER (%d)", uid);

	clearJointsData();
	//
	sharedMem->isTracking = FALSE;
	sharedMem->clearLocalAnimationData();

	if (niNetwork->sendSocket>0) {
		if (appParam.netOutMode==NETandLOCAL) sharedMem->clearLocalAnimationIndex();
		//
		niNetwork->sendAnimationData(posVect, rotQuat, NULL, m_library, OPENNI_JOINT_NUM, TRUE);
	}
}




void  CExOpenNiWin::detectTrackingUser(int uid)
{ 
	DEBUG_INFO("CExOpenNiWin::detectTrackingUser(): DETECT TRACKING USER (%d)", uid);

	sharedMem->isTracking = TRUE;
}






//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// for TEST
//

bool  CExOpenNiWin::setStartBoneLength()
{
	bool ret = false;

	memset(stbnLen, 0, sizeof(double)*OPENNI_JOINT_NUM);

	if (stbnLen[ 1]==0.0 && posVect[ 1].n!=0.0 && posVect[ 2].n!=0.0) {
		stbnLen[ 1] = VectorDist(posVect[ 1], posVect[ 2]);
	}
	if (stbnLen[ 2]==0.0 && posVect[ 2].n!=0.0 && posVect[ 3].n!=0.0) {
		stbnLen[ 2] = VectorDist(posVect[ 2], posVect[ 3]);
	}
	if (stbnLen[ 3]==0.0 && posVect[ 3].n!=0.0 && posVect[ 0].n!=0.0) {
		stbnLen[ 3] = VectorDist(posVect[ 3], posVect[ 0]);
	}

	if (stbnLen[ 6]==0.0 && posVect[ 6].n!=0.0 && posVect[ 2].n!=0.0) {
		stbnLen[ 6] = VectorDist(posVect[ 6], posVect[ 2]);
	}
	if (stbnLen[ 7]==0.0 && posVect[ 7].n!=0.0 && posVect[ 6].n!=0.0) {
		stbnLen[ 7] = VectorDist(posVect[ 7], posVect[ 6]);
	}
	if (stbnLen[ 9]==0.0 && posVect[ 9].n!=0.0 && posVect[ 7].n!=0.0) {
		stbnLen[ 9] = VectorDist(posVect[ 9], posVect[ 7]);
	}

	if (stbnLen[12]==0.0 && posVect[12].n!=0.0 && posVect[ 2].n!=0.0) {
		stbnLen[12] = VectorDist(posVect[12], posVect[ 2]);
	}
	if (stbnLen[13]==0.0 && posVect[13].n!=0.0 && posVect[12].n!=0.0) {
		stbnLen[13] = VectorDist(posVect[13], posVect[12]);
	}
	if (stbnLen[15]==0.0 && posVect[15].n!=0.0 && posVect[13].n!=0.0) {
		stbnLen[15] = VectorDist(posVect[15], posVect[13]);
	}

	if (stbnLen[17]==0.0 && posVect[17].n!=0.0 && posVect[ 0].n!=0.0) {
		stbnLen[17] = VectorDist(posVect[17], posVect[ 0]);
	}
	if (stbnLen[18]==0.0 && posVect[18].n!=0.0 && posVect[17].n!=0.0) {
		stbnLen[18] = VectorDist(posVect[18], posVect[17]);
	}
	if (stbnLen[20]==0.0 && posVect[20].n!=0.0 && posVect[18].n!=0.0) {
		stbnLen[20] = VectorDist(posVect[20], posVect[18]);
	}

	if (stbnLen[21]==0.0 && posVect[21].n!=0.0 && posVect[ 0].n!=0.0) {
		stbnLen[21] = VectorDist(posVect[21], posVect[ 0]);
	}
	if (stbnLen[22]==0.0 && posVect[22].n!=0.0 && posVect[21].n!=0.0) {
		stbnLen[22] = VectorDist(posVect[22], posVect[21]);
	}
	if (stbnLen[24]==0.0 && posVect[24].n!=0.0 && posVect[22].n!=0.0) {
		stbnLen[24] = VectorDist(posVect[24], posVect[22]);
	}

	return ret;
}








////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// スレッド
//

#include  "CameraView.h"


UINT  openniEventLoop(LPVOID pParam)
{
	if (pParam==NULL) return 1;

	CExOpenNiWin* openni = (CExOpenNiWin*)pParam;
	if (!openni->hasContext()) return 1; 
	if (openni->pSensorFrame==NULL)    return 1; 
	
	//CExView* pview = openni->pSensorFrame->pView;
	CCameraView* pview = (CCameraView*)openni->pSensorFrame->pView;
	openni->pViewData  = &pview->viewData;
	
	//
	try {
		Loop {
			if (pview->m_handler->wait((DWORD)0)) break;
			
			if (openni->getDevState()==NI_STATE_DETECT_STOPPING || openni->getDevState()==NI_STATE_SAVE_WORKING) continue;
			openni->waitStreamData();		
			
			if (openni->m_use_image) {
				openni->setImageData();
			}
			
			openni->hasDepthData = FALSE;
			if (openni->getDevState()==NI_STATE_DETECT_EXEC || !isNull(openni->pDepthFrame)) {
				openni->hasDepthData = openni->setDepthData();
			}

			openni->hasUserData = FALSE;
			if (openni->getDevState()==NI_STATE_DETECT_EXEC) {
				openni->hasUserData = openni->setSceneData();
			}

			//
			if (openni->getDevState()==NI_STATE_DETECT_STOPPING || openni->getDevState()==NI_STATE_SAVE_WORKING) continue;
			openni->makeDisplayImage();			// need Scene Data when detected users are painted

			if (openni->getDevState()==NI_STATE_DETECT_EXEC) {
				openni->trackingJoints(openni->appParam.useRotData);	// need Scene Data
			}

			//
			if (pview->SetNewSurface()) {
				pview->ExecRender();
			}


			// Depth Image
			if (openni->pDepthFrame!=NULL && openni->pDepthFrame->pView!=NULL) {
				CCameraView* dpthvw = (CCameraView*)openni->pDepthFrame->pView;
				dpthvw->lock();
				if (openni->hasDepthData) {
					openni->makeDisplayDepth(dpthvw);
					if (dpthvw->SetNewSurface()) {
						dpthvw->ExecRender();
					}
				}
				dpthvw->unlock();
			}
		}
	}

	catch (std::exception& ex) {
		openni->m_err_mesg = _T("openniEventLoop(): EXCEPTION: ");
		openni->m_err_mesg+= mbs2ts((char*)ex.what());
		
		DEBUG_INFO("openniEventLoop(): Exception is Occurred.");

		//
		if (openni->pDepthFrame!=NULL  && openni->pDepthFrame->pView!=NULL) {
			((CCameraView*)openni->pDepthFrame->pView)->unlock();
		}
		if (openni->pSensorFrame!=NULL && openni->pSensorFrame->pView!=NULL) {
			deleteNull(((CCameraView*)openni->pSensorFrame->pView)->m_handler);
		}
		return 2;
	}

	return 0;
}


#endif