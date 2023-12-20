#include "StdAfx.h"


#ifdef ENABLE_KINECT_SDK




#include  "ExKinectWin.h"





using namespace jbxwl;





/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CExOpenNiWin クラス
//

CExKinectWin::CExKinectWin(void)
{
	niFileDev		= NULL;
	niExportData	= NULL;

	niJoints		= NULL;
	niNetwork		= NULL;
	sharedMem		= NULL;

	pLogFrame		= NULL;
	pSensorFrame	= NULL;
	pDepthFrame		= NULL;
	pModelFrame		= NULL;
	pLogDoc			= NULL;

	dataSaving		= FALSE;
	logfSaving		= FALSE;
	logfSending		= FALSE;
	hasAngleData	= FALSE;

	vect_fwrd.set(1.0, 0.0, 0.0, 1.0);

	appParam.init();
	clearJointsData();
}



CExKinectWin::~CExKinectWin(void) 
{
	DEBUG_INFO("DESTRUCTOR: CExKinectWin: START");

	niNetwork		= NULL;
	sharedMem		= NULL;

	pLogFrame		= NULL;
	pSensorFrame	= NULL;
	pDepthFrame		= NULL;
	pModelFrame		= NULL;
	pLogDoc			= NULL;
	
	DEBUG_INFO("DESTRUCTOR: CExKinectWin: END");
}




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//

void  CExKinectWin::setLogFramePtr(CExTextFrame* pfrm)
{
	pLogFrame = pfrm;
	
	if (pfrm!=NULL) pLogDoc = (CLogWndDoc*)(((CLogWndFrame*)pLogFrame)->pDoc);
	else            pLogDoc = NULL;
}



void  CExKinectWin::setParameter(CParameterSet param) 
{ 
	appParam  = param;

	m_use_knct_smth = param.useKnctSmooth;
	smoothParam.fSmoothing  = param.smoothKNCT;
	smoothParam.fCorrection = param.correction;
	m_profile = param.detectParts;
}






///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//

BOOL  CExKinectWin::startLogTransfer(void)
{
	if (logfSending) return logfSending;

	if (niExportData!=NULL) {
//		niExportData->sendLogNet = hasAngleData && appParam.sendLogNet;
		niExportData->sendLogNet = appParam.sendLogNet;
		if (niExportData->sendLogNet) {
			niExportData->exportJointsLogStart(niNetwork, NiSDK_Kinect, NI_TOTAL_JOINT_NUM);
			logfSending = TRUE;
		}
	}

	return logfSending;
}




void  CExKinectWin::stopLogTransfer(void)
{
	if (!logfSending) return;

	if (niExportData!=NULL) {
//		niExportData->sendLogNet = hasAngleData && appParam.sendLogNet;
		niExportData->sendLogNet = appParam.sendLogNet;
		if (niExportData->sendLogNet) {
			niExportData->exportJointsLogStop(niNetwork);
			//logfSending = FALSE;
		}
	}

	logfSending = FALSE;
	return;
}





///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Virtual Function
//

//////////////////////////////////////////////////
//
// ジョイントデータの処理と出力
//
void  CExKinectWin::convertJointsData(void)
{
	hasAngleData = FALSE;

	// Face Tracking
	if (m_use_face) {
		FT_VECTOR3D  hint_pos[2];

		Vector4 pos_neck = joint_PositionData(NUI_SKELETON_POSITION_SHOULDER_CENTER);
		Vector4 pos_head = joint_PositionData(NUI_SKELETON_POSITION_HEAD);

		hint_pos[0].x = pos_neck.x;
		hint_pos[0].y = pos_neck.y;
		hint_pos[0].z = pos_neck.z;
		hint_pos[1].x = pos_head.x;
		hint_pos[1].y = pos_head.y;
		hint_pos[1].z = pos_head.z;
		if (!m_is_mirroring) {
			hint_pos[0].x = - hint_pos[0].x;
			hint_pos[1].x = - hint_pos[1].x;
		}
		//
		isDetectFace = faceDetect(hint_pos);
	}


	// Convert Data
	if (appParam.detectParts==KINECT_SKEL_PROFILE_ALL) {
		convertPos2JointsData();
	}
	else if (appParam.detectParts==KINECT_SKEL_PROFILE_UPPER) {
		convertUpperPos2JointsData();
	}
	else {
		return;
	}

	// Additional Processing
	if (niJoints!=NULL) {
		niJoints->copyJoints2NiJoints(m_is_mirroring);
		niJoints->niPosVect[NI_AVATAR] = currentPos;
		//
		//if (appParam.useFaceDetect) {
		//	niJoints->execFaceTracking();
		//}
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

			/* Log
			niExportData->sendLogNet = hasAngleData && appParam.sendLogNet;
			if (ret && niExportData->sendLogNet && logfSending) {
				Vector<float> pos(-currentPos.y, currentPos.z, -currentPos.x);
				if (m_is_mirroring) pos.x = -pos.x;
				niExportData->exportJointsLog(pos, niJoints->niJntAngl, niNetwork, NiSDK_Kinect, NI_TOTAL_JOINT_NUM);
			}*/
		}
	}
}




void  CExKinectWin::saveJointsData(void)
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




void  CExKinectWin::loggingJointsData(void)
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
		for (int j=0; j<KINECT_JOINT_NUM; j++) {
			Vector4 pos = jointPositionData(j);
			if (appParam.printPostnMode || appParam.printQuateMode) {
				pLogDoc->printFormat("LOCAL: %s (%2d)\n", NiSDK2JointName(j, NiSDK_Kinect).c_str(), j);
			}
			
			if (appParam.printPostnMode) {
				pLogDoc->printFormat("LOCAL: POS1: %f, %f, %f\n", pos.x, pos.y, pos.z);
				pLogDoc->printFormat("LOCAL: POS2: %f, %f, %f\n", posVect[j].x, posVect[j].y, posVect[j].z);
			}
			if (appParam.printQuateMode) {
				pLogDoc->printFormat("LOCAL: QUAT: %f, %f, %f, %f\n", rotQuat[j].x, rotQuat[j].y, rotQuat[j].z, rotQuat[j].s);
			}
			if (appParam.printAngleMode) {
				pLogDoc->printFormat("LOCAL: ANGL: %f\n", jntAngl[j]);
			}
		}

		// Special Joints
		if (niJoints!=NULL) {
			for (int j=0; j<NI_SPECL_JOINT_NUM; j++) {
				int n = NI_NORML_JOINT_NUM + j;

				if (appParam.printPostnMode || appParam.printQuateMode) {
					pLogDoc->printFormat("LOCAL: %s (%2d)\n", NiSDK2JointName(n, NiSDK_None).c_str(), n);
				}
			
				if (appParam.printPostnMode) {
					Vector<float> vect = niJoints->niPosVect[n];
					pLogDoc->printFormat("LOCAL: POS : %f, %f, %f\n", vect.x, vect.y, vect.z);
				}
				if (appParam.printQuateMode) {
					Quaternion<float> quat = niJoints->niRotQuat[n];
					pLogDoc->printFormat("LOCAL: QUAT: %f, %f, %f, %f\n", quat.x, quat.y, quat.z, quat.s);
				}
			}	
		}

		//
		pLogDoc->unlock();
	}
}







//////////////////////////////////////////////////////

void  CExKinectWin::drawAddition(int col, int line)
{
	if (isDetectFace) {
		drawFaceRect(pViewData, m_image_scale, m_is_mirroring, col, line);
	}

	/*
	if (niJoints!=NULL) {
		if (appParam.useFaceDetect) niJoints->drawFaceTracking(col, line);
	}*/
}





//////////////////////////////////////////////////////

void   CExKinectWin::convertPos2JointsData(void)
{
	//
	for (int j=0; j<KINECT_JOINT_NUM; j++) {
		if (posVect[j].c<m_confidence) {
			Vector<float>* ptr = (Vector<float>*)posRing[j].get(-1);
			if (ptr!=NULL && ptr->c>0.0) posVect[j] = *ptr;
			else posVect[j].init(-1.0);
		}
	}

	//
	if (appParam.useMvavSmooth) niJoints->PosMovingAverage(NiSDK_Kinect);
	if (appParam.useJointConst) niJoints->PosVibNoiseCanceler();

	//
	Vector<float> torso_down  =(posVect[NI_SDK_R_HIP] + posVect[NI_SDK_L_HIP])/2.0 - posVect[NI_SDK_PELVIS];
	Vector<float> shldr_left  = posVect[NI_SDK_L_SHLDR] - posVect[NI_SDK_R_SHLDR];
	Vector<float> torso_up	   = - torso_down;
	Vector<float> shldr_right = - shldr_left;

	rotQuat[NI_SDK_PELVIS].init();

	// PELVIS YZ軸の回転
	if (posVect[NI_SDK_R_HIP].c>=m_confidence && posVect[NI_SDK_L_HIP].c>=m_confidence) { 
		Vector<float> vect = NewellMethod(posVect[NI_SDK_R_HIP], posVect[NI_SDK_L_HIP], posVect[NI_SDK_PELVIS]);
		//
		rotQuat[NI_SDK_PELVIS] = V2VQuaternion(vect_fwrd, vect);
		//if (rotQuat[NI_SDK_PELVIS].c==-1.0 && rotQuat[NI_SDK_PELVIS].s==0.0 && rotQuat[NI_SDK_PELVIS].n==0.0) {
		//	rotQuat[NI_SDK_PELVIS].setRotation(PI, torso_up);
		//}
	}

	// X 軸回転
	Quaternion<float> quat_lhip = ~rotQuat[NI_SDK_PELVIS]*posVect[NI_SDK_L_HIP]*rotQuat[NI_SDK_PELVIS];
	Quaternion<float> quat_rhip = ~rotQuat[NI_SDK_PELVIS]*posVect[NI_SDK_R_HIP]*rotQuat[NI_SDK_PELVIS];

	Vector<float> hip_left  = quat_lhip.getVector() - quat_rhip.getVector(); 
	Vector<float> hip_right = - hip_left;
	float thx = atan2(hip_left.z, hip_left.y);
	Quaternion<float> xrot;
	xrot.setRotation(thx, 1.0, 0.0, 0.0, 1.0);

	// Y軸回転補正
	Quaternion<float> yrot;
	yrot.setRotation((float)appParam.YaxisCorrect, 0.0, 1.0, 0.0, 1.0);

	//
	rotQuat[NI_SDK_PELVIS]  = rotQuat[NI_SDK_PELVIS]*xrot*yrot;

	//
	rotQuat[NI_SDK_TORSO]   = VPPQuaternion(torso_up, posVect[NI_SDK_TORSO], posVect[NI_SDK_NECK]);
	rotQuat[NI_SDK_NECK]    = PPPQuaternion(posVect[NI_SDK_TORSO],  posVect[NI_SDK_NECK],  posVect[NI_SDK_HEAD]);

	// To Relative Coordinate
	rotQuat[NI_SDK_L_SHLDR] = VPPQuaternion(shldr_left,  posVect[NI_SDK_L_SHLDR], posVect[NI_SDK_L_ELBOW]);
	rotQuat[NI_SDK_R_SHLDR] = VPPQuaternion(shldr_right, posVect[NI_SDK_R_SHLDR], posVect[NI_SDK_R_ELBOW]);
	rotQuat[NI_SDK_L_HIP]   = VPPQuaternion(torso_down,  posVect[NI_SDK_L_HIP],   posVect[NI_SDK_L_KNEE]);
	rotQuat[NI_SDK_R_HIP]   = VPPQuaternion(torso_down,  posVect[NI_SDK_R_HIP],   posVect[NI_SDK_R_KNEE]);
//	rotQuat[NI_SDK_L_ANKLE] = VPPQuaternion(vect_ft_left,  posVect[NI_SDK_L_ANKLE], posVect[NI_SDK_L_FOOT]);
//	rotQuat[NI_SDK_R_ANKLE] = VPPQuaternion(vect_ft_right, posVect[NI_SDK_R_ANKLE], posVect[NI_SDK_R_FOOT]);
	//
	rotQuat[NI_SDK_L_ELBOW] = PPPQuaternion(posVect[NI_SDK_L_SHLDR], posVect[NI_SDK_L_ELBOW], posVect[NI_SDK_L_WRIST]);
	rotQuat[NI_SDK_R_ELBOW] = PPPQuaternion(posVect[NI_SDK_R_SHLDR], posVect[NI_SDK_R_ELBOW], posVect[NI_SDK_R_WRIST]);
	rotQuat[NI_SDK_L_WRIST] = PPPQuaternion(posVect[NI_SDK_L_ELBOW], posVect[NI_SDK_L_WRIST], posVect[NI_SDK_L_HAND]);
	rotQuat[NI_SDK_R_WRIST] = PPPQuaternion(posVect[NI_SDK_R_ELBOW], posVect[NI_SDK_R_WRIST], posVect[NI_SDK_R_HAND]);
	rotQuat[NI_SDK_L_KNEE]  = PPPQuaternion(posVect[NI_SDK_L_HIP],   posVect[NI_SDK_L_KNEE],  posVect[NI_SDK_L_ANKLE]);
	rotQuat[NI_SDK_R_KNEE]  = PPPQuaternion(posVect[NI_SDK_R_HIP],   posVect[NI_SDK_R_KNEE],  posVect[NI_SDK_R_ANKLE]);


	// To Avatar Coordinate
	rotQuat[NI_SDK_L_WRIST] = ~rotQuat[NI_SDK_PELVIS]*rotQuat[NI_SDK_L_WRIST]*rotQuat[NI_SDK_PELVIS];
	rotQuat[NI_SDK_R_WRIST] = ~rotQuat[NI_SDK_PELVIS]*rotQuat[NI_SDK_R_WRIST]*rotQuat[NI_SDK_PELVIS];
	rotQuat[NI_SDK_L_ELBOW] = ~rotQuat[NI_SDK_PELVIS]*rotQuat[NI_SDK_L_ELBOW]*rotQuat[NI_SDK_PELVIS];
	rotQuat[NI_SDK_R_ELBOW] = ~rotQuat[NI_SDK_PELVIS]*rotQuat[NI_SDK_R_ELBOW]*rotQuat[NI_SDK_PELVIS];
	rotQuat[NI_SDK_L_SHLDR] = ~rotQuat[NI_SDK_PELVIS]*rotQuat[NI_SDK_L_SHLDR]*rotQuat[NI_SDK_PELVIS];
	rotQuat[NI_SDK_R_SHLDR] = ~rotQuat[NI_SDK_PELVIS]*rotQuat[NI_SDK_R_SHLDR]*rotQuat[NI_SDK_PELVIS];
//	rotQuat[NI_SDK_L_ANKLE] = ~rotQuat[NI_SDK_PELVIS]*rotQuat[NI_SDK_L_ANKLE]*rotQuat[NI_SDK_PELVIS];
//	rotQuat[NI_SDK_R_ANKLE] = ~rotQuat[NI_SDK_PELVIS]*rotQuat[NI_SDK_R_ANKLE]*rotQuat[NI_SDK_PELVIS];
	rotQuat[NI_SDK_L_KNEE]  = ~rotQuat[NI_SDK_PELVIS]*rotQuat[NI_SDK_L_KNEE] *rotQuat[NI_SDK_PELVIS];
	rotQuat[NI_SDK_R_KNEE]  = ~rotQuat[NI_SDK_PELVIS]*rotQuat[NI_SDK_R_KNEE] *rotQuat[NI_SDK_PELVIS];
	rotQuat[NI_SDK_L_HIP]   = ~rotQuat[NI_SDK_PELVIS]*rotQuat[NI_SDK_L_HIP]  *rotQuat[NI_SDK_PELVIS];
	rotQuat[NI_SDK_R_HIP]   = ~rotQuat[NI_SDK_PELVIS]*rotQuat[NI_SDK_R_HIP]  *rotQuat[NI_SDK_PELVIS];
	rotQuat[NI_SDK_NECK]    = ~rotQuat[NI_SDK_PELVIS]*rotQuat[NI_SDK_NECK]   *rotQuat[NI_SDK_PELVIS];
	rotQuat[NI_SDK_TORSO]   = ~rotQuat[NI_SDK_PELVIS]*rotQuat[NI_SDK_TORSO]  *rotQuat[NI_SDK_PELVIS];

	//
	// Face
	rotQuat[NI_SDK_HEAD].init(-1.0);
	if (isDetectFace) {
		rotQuat[NI_SDK_HEAD] = getFaceRotation();
		rotQuat[NI_SDK_HEAD] = ~rotQuat[NI_SDK_PELVIS]*rotQuat[NI_SDK_HEAD]*rotQuat[NI_SDK_PELVIS];
	}

	//
	for (int j=0; j<KINECT_JOINT_NUM; j++) {
		if (rotQuat[j].c<m_confidence) {
			Quaternion<float>* ptr = (Quaternion<float>*)rotRing[j].get(-1);
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
		Vector<float> upz(0.0, 0.0, 1.0, 1.0);
		//
		jntAngl[NI_SDK_PELVIS]  = VectorAngle(upz, torso_up);
		jntAngl[NI_SDK_TORSO]   = VectorAngle(torso_up, posVect[NI_SDK_NECK] - posVect[NI_SDK_TORSO]);
		jntAngl[NI_SDK_NECK]    = VectorAngle(posVect[NI_SDK_TORSO], posVect[NI_SDK_NECK],  posVect[NI_SDK_HEAD]);
		//
		jntAngl[NI_SDK_L_SHLDR] = VectorAngle(shldr_left,   posVect[NI_SDK_L_ELBOW] - posVect[NI_SDK_L_SHLDR]);
		jntAngl[NI_SDK_R_SHLDR] = VectorAngle(shldr_right,  posVect[NI_SDK_R_ELBOW] - posVect[NI_SDK_R_SHLDR]);
		jntAngl[NI_SDK_L_HIP]   = VectorAngle(torso_down,   posVect[NI_SDK_L_KNEE]  - posVect[NI_SDK_L_HIP]);
		jntAngl[NI_SDK_R_HIP]   = VectorAngle(torso_down,   posVect[NI_SDK_R_KNEE]  - posVect[NI_SDK_R_HIP]);
//		jntAngl[NI_SDK_L_ANKLE] = VectorAngle(vect_ft_left, posVect[NI_SDK_L_FOOT]  - posVect[NI_SDK_L_ANKLE]);
//		jntAngl[NI_SDK_R_ANKLE] = VectorAngle(vect_ft_right,posVect[NI_SDK_R_FOOT]  - posVect[NI_SDK_R_ANKLE]);
		
		//
		if (posVect[NI_SDK_L_WRIST].c>0.0) jntAngl[NI_SDK_L_ELBOW] = VectorAngle(posVect[NI_SDK_L_SHLDR], posVect[NI_SDK_L_ELBOW], posVect[NI_SDK_L_WRIST]);
		else							   jntAngl[NI_SDK_L_ELBOW] = 0.0;
		if (posVect[NI_SDK_R_WRIST].c>0.0) jntAngl[NI_SDK_R_ELBOW] = VectorAngle(posVect[NI_SDK_R_SHLDR], posVect[NI_SDK_R_ELBOW], posVect[NI_SDK_R_WRIST]);
		else							   jntAngl[NI_SDK_R_ELBOW] = 0.0;
		
		if (posVect[NI_SDK_L_HAND].c>0.0) jntAngl[NI_SDK_L_WRIST] = VectorAngle(posVect[NI_SDK_L_ELBOW], posVect[NI_SDK_L_WRIST], posVect[NI_SDK_L_HAND]);
		else							  jntAngl[NI_SDK_L_WRIST] = 0.0; 
		if (posVect[NI_SDK_R_HAND].c>0.0) jntAngl[NI_SDK_R_WRIST] = VectorAngle(posVect[NI_SDK_R_ELBOW], posVect[NI_SDK_R_WRIST], posVect[NI_SDK_R_HAND]);
		else							  jntAngl[NI_SDK_R_WRIST] = 0.0;

		if (posVect[NI_SDK_L_ANKLE].c>0.0) jntAngl[NI_SDK_L_KNEE] = VectorAngle(posVect[NI_SDK_L_HIP], posVect[NI_SDK_L_KNEE], posVect[NI_SDK_L_ANKLE]);
		else							   jntAngl[NI_SDK_L_KNEE] = 0.0; 
		if (posVect[NI_SDK_R_ANKLE].c>0.0) jntAngl[NI_SDK_R_KNEE] = VectorAngle(posVect[NI_SDK_R_HIP], posVect[NI_SDK_R_KNEE], posVect[NI_SDK_R_ANKLE]);
		else							   jntAngl[NI_SDK_R_KNEE] = 0.0;
	}

	return;
}





void   CExKinectWin::convertUpperPos2JointsData(void)
{
	//
	for (int j=0; j<KINECT_JOINT_NUM; j++) {
		if (posVect[j].c<m_confidence) {
			Vector<double>* ptr = (Vector<double>*)posRing[j].get(-1);
			if (ptr!=NULL && ptr->c>0.0) posVect[j] = *ptr;
			else posVect[j].init(-1.0);
		}
	}

	//
	if (appParam.useMvavSmooth) niJoints->PosMovingAverage(NiSDK_Kinect);
	if (appParam.useJointConst) niJoints->PosVibNoiseCanceler();

	//
//	Vector<double> torso_down(0.0, 0.0, -1.0);
	Vector<double> torso_down  = posVect[NI_SDK_TORSO]   - posVect[NI_SDK_NECK];
	Vector<double> torso_up	   = - torso_down;
	Vector<double> shldr_left  = posVect[NI_SDK_L_SHLDR] - posVect[NI_SDK_R_SHLDR];
	Vector<double> shldr_right = - shldr_left;

	//
	rotQuat[NI_SDK_PELVIS].init();
	posVect[NI_SDK_PELVIS].init();

	// PELVIS YZ軸の回転
	if (posVect[NI_SDK_R_SHLDR].c>=m_confidence && posVect[NI_SDK_L_SHLDR].c>=m_confidence) { 
		Vector<double> vect = NewellMethod(posVect[NI_SDK_L_SHLDR], posVect[NI_SDK_R_SHLDR], posVect[NI_SDK_TORSO]);
		//
		rotQuat[NI_SDK_PELVIS] = V2VQuaternion(vect_fwrd, vect); 
		//if (rotQuat[NI_SDK_PELVIS].c==-1.0 && rotQuat[NI_SDK_PELVIS].s==0.0 && rotQuat[NI_SDK_PELVIS].n==0.0) {
		//	rotQuat[NI_SDK_PELVIS].setRotation(PI, torso_up);
		//}
	}

	// X 軸回転
	Quaternion<double> quat_lshd = ~rotQuat[NI_SDK_PELVIS]*posVect[NI_SDK_L_SHLDR]*rotQuat[NI_SDK_PELVIS];
	Quaternion<double> quat_rshd = ~rotQuat[NI_SDK_PELVIS]*posVect[NI_SDK_R_SHLDR]*rotQuat[NI_SDK_PELVIS];

	Vector<double> shd_left  = quat_lshd.getVector() - quat_rshd.getVector();
	Vector<double> shd_right = - shd_left;
	float thx = atan2(shd_left.z, shd_left.y);
	Quaternion<double> xrot;
	xrot.setRotation(thx, 1.0, 0.0, 0.0, 1.0);

	// Y軸回転補正
	Quaternion<double> yrot;
	yrot.setRotation(appParam.YaxisCorrect, 0.0, 1.0, 0.0, 1.0);

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
	rotQuat[NI_SDK_L_WRIST] = PPPQuaternion(posVect[NI_SDK_L_ELBOW], posVect[NI_SDK_L_WRIST], posVect[NI_SDK_L_HAND]);
	rotQuat[NI_SDK_R_WRIST] = PPPQuaternion(posVect[NI_SDK_R_ELBOW], posVect[NI_SDK_R_WRIST], posVect[NI_SDK_R_HAND]);

	//
	Quaternion<float> pelvisXY;
	Vector<float> euler = Quaternion2EulerXYZ(rotQuat[NI_SDK_PELVIS]);
	euler.z = 0.0;
	pelvisXY.setEulerXYZ(euler);


	// To Avatar Coordinate
	rotQuat[NI_SDK_L_WRIST] = ~rotQuat[NI_SDK_PELVIS]*rotQuat[NI_SDK_L_WRIST]*rotQuat[NI_SDK_PELVIS];
	rotQuat[NI_SDK_R_WRIST] = ~rotQuat[NI_SDK_PELVIS]*rotQuat[NI_SDK_R_WRIST]*rotQuat[NI_SDK_PELVIS];
	rotQuat[NI_SDK_L_ELBOW] = ~rotQuat[NI_SDK_PELVIS]*rotQuat[NI_SDK_L_ELBOW]*rotQuat[NI_SDK_PELVIS];
	rotQuat[NI_SDK_R_ELBOW] = ~rotQuat[NI_SDK_PELVIS]*rotQuat[NI_SDK_R_ELBOW]*rotQuat[NI_SDK_PELVIS];
	rotQuat[NI_SDK_L_SHLDR] = ~rotQuat[NI_SDK_PELVIS]*rotQuat[NI_SDK_L_SHLDR]*rotQuat[NI_SDK_PELVIS];
	rotQuat[NI_SDK_R_SHLDR] = ~rotQuat[NI_SDK_PELVIS]*rotQuat[NI_SDK_R_SHLDR]*rotQuat[NI_SDK_PELVIS];
	rotQuat[NI_SDK_NECK]    = ~rotQuat[NI_SDK_PELVIS]*rotQuat[NI_SDK_NECK]   *rotQuat[NI_SDK_PELVIS];
	rotQuat[NI_SDK_TORSO]   = ~rotQuat[NI_SDK_PELVIS]*rotQuat[NI_SDK_TORSO]  *rotQuat[NI_SDK_PELVIS];

//	rotQuat[NI_SDK_L_ANKLE].init();
//	rotQuat[NI_SDK_R_ANKLE].init();
	rotQuat[NI_SDK_L_KNEE].init();
	rotQuat[NI_SDK_R_KNEE].init();
	rotQuat[NI_SDK_L_HIP]   = ~pelvisXY;
	rotQuat[NI_SDK_R_HIP]   = ~pelvisXY;

	//
	// Face
	rotQuat[NI_SDK_HEAD].init(-1.0);
	if (isDetectFace) {
		rotQuat[NI_SDK_HEAD] = getFaceRotation();
		rotQuat[NI_SDK_HEAD] = ~rotQuat[NI_SDK_PELVIS]*rotQuat[NI_SDK_HEAD]*rotQuat[NI_SDK_PELVIS];
	}

	//
	for (int j=0; j<KINECT_JOINT_NUM; j++) {
		if (rotQuat[j].c<m_confidence) {
			Quaternion<float>* ptr = (Quaternion<float>*)rotRing[j].get(-1);
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
		Vector<float> upz(0.0, 0.0, 1.0, 1.0);
		//
		jntAngl[NI_SDK_PELVIS]  = VectorAngle(upz, torso_up);
		jntAngl[NI_SDK_TORSO]   = VectorAngle(torso_up, posVect[NI_SDK_NECK] - posVect[NI_SDK_TORSO]);
		jntAngl[NI_SDK_NECK]    = VectorAngle(posVect[NI_SDK_TORSO], posVect[NI_SDK_NECK],  posVect[NI_SDK_HEAD]);
		//
		jntAngl[NI_SDK_L_SHLDR] = VectorAngle(shldr_left,   posVect[NI_SDK_L_ELBOW] - posVect[NI_SDK_L_SHLDR]);
		jntAngl[NI_SDK_R_SHLDR] = VectorAngle(shldr_right,  posVect[NI_SDK_R_ELBOW] - posVect[NI_SDK_R_SHLDR]);
		
		//
		if (posVect[NI_SDK_L_WRIST].c>0.0) jntAngl[NI_SDK_L_ELBOW] = VectorAngle(posVect[NI_SDK_L_SHLDR], posVect[NI_SDK_L_ELBOW], posVect[NI_SDK_L_WRIST]);
		else							   jntAngl[NI_SDK_L_ELBOW] = 0.0;
		if (posVect[NI_SDK_R_WRIST].c>0.0) jntAngl[NI_SDK_R_ELBOW] = VectorAngle(posVect[NI_SDK_R_SHLDR], posVect[NI_SDK_R_ELBOW], posVect[NI_SDK_R_WRIST]);
		else							   jntAngl[NI_SDK_R_ELBOW] = 0.0;
		//
		if (posVect[NI_SDK_L_HAND].c>0.0) jntAngl[NI_SDK_L_WRIST] = VectorAngle(posVect[NI_SDK_L_ELBOW], posVect[NI_SDK_L_WRIST], posVect[NI_SDK_L_HAND]);
		else							  jntAngl[NI_SDK_L_WRIST] = 0.0;
		if (posVect[NI_SDK_R_HAND].c>0.0) jntAngl[NI_SDK_R_WRIST] = VectorAngle(posVect[NI_SDK_R_ELBOW], posVect[NI_SDK_R_WRIST], posVect[NI_SDK_R_HAND]);
		else							  jntAngl[NI_SDK_R_WRIST] = 0.0;
	}

	return;
}





void  CExKinectWin::lostTrackingUser(int uid)
{ 
	DEBUG_INFO("CExKinectWin::lostTrackingUser():   LOST   TRACKING USER (%d)", uid);
 
	clearJointsData();
	//
	sharedMem->isTracking = FALSE;
	sharedMem->clearLocalAnimationData();

	if (niNetwork->sendSocket>0) {
		if (appParam.netOutMode==NETandLOCAL) sharedMem->clearLocalAnimationIndex();
		//
		niNetwork->sendAnimationData(posVect, rotQuat, NULL, NiSDK_Kinect, KINECT_JOINT_NUM, TRUE);
	}
}




void  CExKinectWin::detectTrackingUser(int uid)
{ 
	DEBUG_INFO("CExKinectWin::detectTrackingUser(): DETECT TRACKING USER (%d)", uid);

	sharedMem->isTracking = TRUE;
}




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// スレッド
//

#include  "CameraView.h"


UINT  kinectEventLoop(LPVOID pParam)
{
	if (pParam==NULL) return 1;

	CExKinectWin* kinect = (CExKinectWin*)pParam;
	if (!kinect->hasContext()) return 1; 
	if (kinect->pSensorFrame==NULL)    return 1; 
	
//	CExView* pview    = kinect->pSensorFrame->pView;
	CCameraView* pview = (CCameraView*)kinect->pSensorFrame->pView;
	kinect->pViewData  = &pview->viewData;


	BOOL ret;
	//
	try {
		Loop {
			if (pview->m_handler->wait((DWORD)0)) break;

			if (kinect->getDevState()==NI_STATE_DETECT_STOPPING) continue;

			if (kinect->m_use_image) {
				ret = kinect->waitImage();
			}

			//
			if (kinect->getDevState()==NI_STATE_DETECT_STOPPING) continue;
			kinect->hasDepthData = FALSE;
			if (kinect->getDevState()==NI_STATE_DETECT_EXEC || !isNull(kinect->pDepthFrame)) {
				ret = kinect->waitDepth();
				if (ret) kinect->hasDepthData = TRUE;
			}

			if (kinect->getDevState()==NI_STATE_DETECT_STOPPING) continue;
			//if (isNull(kinect->pSensorFrame)) break;
			kinect->makeDisplayImage();			// need Depth Data when detected users are painted

			if (kinect->getDevState()==NI_STATE_DETECT_EXEC) {
				kinect->trackingJoints();
			}

			//
			//if (isNull(kinect->pSensorFrame)) break;
			//if (isNull(kinect->pSensorFrame->pView)) break;
			//if (!pview->SetNewSurface()) break;
			//if (isNull(kinect->pSensorFrame)) break;
			if (pview->SetNewSurface()) {
				pview->ExecRender();
			}

			
			// Depth Image
			if (kinect->pDepthFrame!=NULL && kinect->pDepthFrame->pView!=NULL) {
				CCameraView* dpthvw = (CCameraView*)kinect->pDepthFrame->pView;
				dpthvw->lock();
				if (kinect->hasDepthData) {
					kinect->makeDisplayDepth(dpthvw);
					if (dpthvw->SetNewSurface()) {
						dpthvw->ExecRender();
					}
				}
				dpthvw->unlock();
			}
		}
	}

	catch (std::exception& ex) {
		kinect->m_err_mesg = _T("kinectEventLoop(): EXCEPTION: ");
		kinect->m_err_mesg+= mbs2ts((char*)ex.what());
		
		DEBUG_INFO("kinectEventLoop(): Exception is occurred.");

		//
		if (kinect->pDepthFrame!=NULL  && kinect->pDepthFrame->pView!=NULL) {
			((CCameraView*)kinect->pDepthFrame->pView)->unlock();
		}
		if (kinect->pSensorFrame!=NULL && kinect->pSensorFrame->pView!=NULL) {
			deleteNull(((CCameraView*)kinect->pSensorFrame->pView)->m_handler);
		}
		return 2;
	}
	
	return 0;
}




#endif