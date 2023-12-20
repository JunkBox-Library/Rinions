#include  "stdafx.h"


#include  "ExNiFileDev.h"





//////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CExNiFileDev Class
//

CExNiFileDev::~CExNiFileDev(void)
{
	DEBUG_INFO("DESTRUCTOR: CExNiFileDev");
}




void  CExNiFileDev::init(void)
{ 
	vect_fwrd.set(1.0, 0.0, 0.0, 1.0, 1.0);

	len_scale = 1.0;
	init_pos  = FALSE;
	mirroring = TRUE;

	niExportData = NULL;

	appParam.init();
	setTempFilePath();

	//
	pLogFile = NULL;
}




BOOL  CExNiFileDev::saveAsJText(LPCTSTR backup, BOOL force)
{ 
	BOOL ret = FALSE;

	if (tempFilePath!=_T("") && backup!=NULL) {
		ret = saveJTextfromFile((LPCTSTR)tempFilePath, backup, force);
	}

	return ret;
}




BOOL  CExNiFileDev::saveAsBVH(LPCTSTR backup, BVHSaveParam param)
{ 
	BOOL ret = FALSE;

	if (tempFilePath!=_T("") && backup!=NULL) {
		ret = saveBVHfromFile((LPCTSTR)tempFilePath, backup, param);
	}

	return ret;
}



//
// 戻り値：　正: 最後のフレームの次のフレームの番号, 負: エラー 次のフレームの番号x(-1)
//	
int  CExNiFileDev::convertJointsData(CExNiNetwork* net, FileDevParam param)
{
	if (net==NULL || niExportData==NULL) return 0;
	if (param.controler!=NULL) {
		if (*param.controler!=NI_FILE_PLAYER_START && *param.controler!=NI_FILE_PLAYER_GO) return -1;
	}
	if (!startFrame(param.start_frame)) return 0;

	TCHAR buf[LNAME];

	//
	len_scale  = param.size_scale;
	mirroring  = param.mirroring;
	init_pos   = param.init_pos; 
	time_scale = param.time_scale;

	//
	int frame = frame_start;
	if (*param.controler==NI_FILE_PLAYER_START) clearStartPosition();

	while (frame<frames_num) {
		sntprintf(buf, LNAME, _T(" %d"), frame+1);
		param.counter->SetWindowText(buf);
		
		//
		clearJointsData();
		getJointsDataSeq(mirroring);						// 次のフレーム
		correctPosVect(posVect, frame, *(param.controler));	// for start position
		if (param.calc_quat) calcRotQaut(posVect, rotQuat); // クオータニオンの再計算

		// Additional Processing
		posVect[NI_AVATAR] = currentPos;

		//
		niExportData->exportJointsData(posVect, rotQuat, NULL, net, NiSDK_None, NI_TOTAL_JOINT_NUM);
		if (param.controler!=NULL) {
			if (*param.controler!=NI_FILE_PLAYER_START && *param.controler!=NI_FILE_PLAYER_GO) {
				frame++;
				break;
			}
		}
		frame++;

		if (!nextFrame()) return -frame;
		DisPatcher();
	}

	if (frame==frames_num) frame = 0;
	return frame;
}





void  CExNiFileDev::correctPosVect(Vector<float>* posvect, int frame, int cntrlr)
{
	if (posvect[NI_PELVIS].c<0.0) {
		posvect[NI_PELVIS] = (posvect[NI_R_HIP] + posvect[NI_L_HIP])/2.0;
	}
	if (posvect[NI_NECK].c<0.0) {
		posvect[NI_NECK] = (posvect[NI_R_SHLDR] + posvect[NI_L_SHLDR])/2.0;
	}
	if (posvect[NI_TORSO].c<0.0) {
		posvect[NI_TORSO] = (posvect[NI_PELVIS] + posvect[NI_NECK])/2.0;
	}

	if (cntrlr==NI_FILE_PLAYER_START || cntrlr==NI_FILE_PLAYER_GO) {// && init_pos) {
		currentPos = posvect[NI_PELVIS]*len_scale;
		if (frame==0) {
			if (init_pos) {
				startPos = currentPos;
			}
			else {
				startPos.set(0.0, 0.0, 0.0);
			}
			//DEBUG_INFO("startPos = (%f, %f, %f)", startPos.x, startPos.y, startPos.z);
		}
	}
	
	if (frame>=0) {
		for (int j=0; j<NI_NORML_JOINT_NUM; j++) {
			posvect[j] = posvect[j]*len_scale - startPos;
		}
	}

	//DEBUG_INFO("PELVIS POS = (%f. %f, %f)", posvect[NI_PELVIS].x, posvect[NI_PELVIS].y, posvect[NI_PELVIS].z);
	
	return;
}	
	



//
// 位置データからクォータニオンを計算
//
void  CExNiFileDev::calcRotQaut(Vector<float>* posvect, Quaternion<float>* rotquat)
{
	//
	Vector<float> torso_down  = posvect[NI_PELVIS]  - posvect[NI_TORSO];
	Vector<float> shldr_left  = posvect[NI_L_SHLDR] - posvect[NI_R_SHLDR];
	Vector<float> torso_up	  = - torso_down;
	Vector<float> shldr_right = - shldr_left;

	// PELVIS YZ平面の回転
	Vector<float> vect = NewellMethod4(posvect[NI_R_HIP], posvect[NI_PELVIS], posvect[NI_L_HIP], posvect[NI_TORSO]);
	rotquat[NI_PELVIS] = V2VQuaternion(vect_fwrd, vect);

	// X 軸回転
	Quaternion<float> quat_lhip = ~rotquat[NI_PELVIS]*posvect[NI_L_HIP]*rotquat[NI_PELVIS];
	Quaternion<float> quat_rhip = ~rotquat[NI_PELVIS]*posvect[NI_R_HIP]*rotquat[NI_PELVIS];

	Vector<float> hip_left  = quat_lhip.getVector() - quat_rhip.getVector(); 
	Vector<float> hip_right = - hip_left;
	float thx = atan2(hip_left.z, hip_left.y);
	Quaternion<float> xrot;
	xrot.setRotation(thx, 1.0, 0.0, 0.0, 1.0);

	//
	rotquat[NI_PELVIS] = rotquat[NI_PELVIS]*xrot;
	rotquat[NI_TORSO]  = VPPQuaternion(torso_up, posvect[NI_TORSO], posvect[NI_NECK]);
	rotquat[NI_NECK]   = PPPQuaternion(posvect[NI_TORSO],  posvect[NI_NECK],  posvect[NI_HEAD]);

	//
	rotquat[NI_L_SHLDR] = VPPQuaternion(shldr_left,    posvect[NI_L_SHLDR], posvect[NI_L_ELBOW]);
	rotquat[NI_R_SHLDR] = VPPQuaternion(shldr_right,   posvect[NI_R_SHLDR], posvect[NI_R_ELBOW]);
	rotquat[NI_L_HIP]   = VPPQuaternion(torso_down,    posvect[NI_L_HIP],   posvect[NI_L_KNEE]);
	rotquat[NI_R_HIP]   = VPPQuaternion(torso_down,    posvect[NI_R_HIP],   posvect[NI_R_KNEE]);
//	rotquat[NI_L_ANKLE] = VPPQuaternion(vect_ft_left,  posvect[NI_L_ANKLE], posvect[NI_L_FOOT]);
//	rotquat[NI_R_ANKLE] = VPPQuaternion(vect_ft_right, posvect[NI_R_ANKLE], posvect[NI_R_FOOT]);

	//
	if (posvect[NI_L_WRIST].c>0.0) {
		rotquat[NI_L_ELBOW] = PPPQuaternion(posvect[NI_L_SHLDR], posvect[NI_L_ELBOW], posvect[NI_L_WRIST]);
		rotquat[NI_L_WRIST] = PPPQuaternion(posvect[NI_L_ELBOW], posvect[NI_L_WRIST], posvect[NI_L_HAND]);
	}
	else {
		rotquat[NI_L_ELBOW] = PPPQuaternion(posvect[NI_L_SHLDR], posvect[NI_L_ELBOW], posvect[NI_L_HAND]);
	}
	if (posvect[NI_R_WRIST].c>0.0) {
		rotquat[NI_R_ELBOW] = PPPQuaternion(posvect[NI_R_SHLDR], posvect[NI_R_ELBOW], posvect[NI_R_WRIST]);
		rotquat[NI_R_WRIST] = PPPQuaternion(posvect[NI_R_ELBOW], posvect[NI_R_WRIST], posvect[NI_R_HAND]);
	}
	else {
		rotquat[NI_R_ELBOW] = PPPQuaternion(posvect[NI_R_SHLDR], posvect[NI_R_ELBOW], posvect[NI_R_HAND]);
	}

	//
	if (posvect[NI_L_ANKLE].c>0.0) {
		rotquat[NI_L_KNEE]  = PPPQuaternion(posvect[NI_L_HIP],   posvect[NI_L_KNEE],  posvect[NI_L_ANKLE]);
	}
	else{
		rotquat[NI_L_KNEE]  = PPPQuaternion(posvect[NI_L_HIP],   posvect[NI_L_KNEE],  posvect[NI_L_FOOT]);
	}
	if (posvect[NI_R_ANKLE].c>0.0) {
		rotquat[NI_R_KNEE]  = PPPQuaternion(posvect[NI_R_HIP],   posvect[NI_R_KNEE],  posvect[NI_R_ANKLE]);
	}
	else{
		rotquat[NI_R_KNEE]  = PPPQuaternion(posvect[NI_R_HIP],   posvect[NI_R_KNEE],  posvect[NI_R_FOOT]);
	}


	// To Avatar Coordinate
	rotquat[NI_L_WRIST] = ~rotquat[NI_PELVIS]*rotquat[NI_L_WRIST]*rotquat[NI_PELVIS];
	rotquat[NI_R_WRIST] = ~rotquat[NI_PELVIS]*rotquat[NI_R_WRIST]*rotquat[NI_PELVIS];
	rotquat[NI_L_ELBOW] = ~rotquat[NI_PELVIS]*rotquat[NI_L_ELBOW]*rotquat[NI_PELVIS];
	rotquat[NI_R_ELBOW] = ~rotquat[NI_PELVIS]*rotquat[NI_R_ELBOW]*rotquat[NI_PELVIS];
	rotquat[NI_L_SHLDR] = ~rotquat[NI_PELVIS]*rotquat[NI_L_SHLDR]*rotquat[NI_PELVIS];
	rotquat[NI_R_SHLDR] = ~rotquat[NI_PELVIS]*rotquat[NI_R_SHLDR]*rotquat[NI_PELVIS];
//	rotquat[NI_L_ANKLE] = ~rotquat[NI_PELVIS]*rotquat[NI_L_ANKLE]*rotquat[NI_PELVIS];
//	rotquat[NI_R_ANKLE] = ~rotquat[NI_PELVIS]*rotquat[NI_R_ANKLE]*rotquat[NI_PELVIS];
	rotquat[NI_L_KNEE]  = ~rotquat[NI_PELVIS]*rotquat[NI_L_KNEE] *rotquat[NI_PELVIS];
	rotquat[NI_R_KNEE]  = ~rotquat[NI_PELVIS]*rotquat[NI_R_KNEE] *rotquat[NI_PELVIS];
	rotquat[NI_L_HIP]   = ~rotquat[NI_PELVIS]*rotquat[NI_L_HIP]  *rotquat[NI_PELVIS];
	rotquat[NI_R_HIP]   = ~rotquat[NI_PELVIS]*rotquat[NI_R_HIP]  *rotquat[NI_PELVIS];
	rotquat[NI_NECK]    = ~rotquat[NI_PELVIS]*rotquat[NI_NECK]   *rotquat[NI_PELVIS];
	rotquat[NI_TORSO]   = ~rotquat[NI_PELVIS]*rotquat[NI_TORSO]  *rotquat[NI_PELVIS];

	return;
}





void  CExNiFileDev::Vector2Quaternion(Vector<float>* posvect, Quaternion<float>* rotquat)
{
	correctPosVect(posvect, -1, 0);	//
	calcRotQaut(posvect, rotquat);
	
	if (appParam.useJointConst) NiCorrectJointsRotation(rotquat, NiSDK_None);

	return;
}







/////////////////////////////////////////////////////////////////////////////////////////////
//
// Log File
//

BOOL  CExNiFileDev::openLogFile(LPCTSTR logpath)
{
	Buffer path = make_Buffer(LNAME);
	Buffer name = make_Buffer(LNAME);
	char*  lctm = GetLocalTime('.', '_');		// not free

	copy_ts2Buffer(logpath, &path);
	copy_ts2Buffer(appParam.userName, &name);

	cat_s2Buffer("\\", &path);
	mkdir((const char*)path.buf);
	cat_Buffer(&name, &path);
	cat_s2Buffer("\\", &path);
	mkdir((const char*)path.buf);

	cat_s2Buffer(lctm, &path);
	cat_s2Buffer(".txt", &path);

	pLogFile = fopen((const char*)path.buf, "w");
	if (pLogFile==NULL) {
		free_Buffer(&path);
		free_Buffer(&name);
		return FALSE;
	}

	if (JTXT_Tool==NULL) JTXT_Tool = new CNiJTextTool();
	if (JTXT_Tool==NULL) return FALSE;
	//
	JTXT_Tool->writeHeader(pLogFile);
	fprintf(pLogFile, "%s\n", name.buf);

	/*
	log_ttl_time = 0;
	log_lap_time = 0;
	//
	fprintf(pLogFile, "%s\n", name.buf);
	fprintf(pLogFile, NI_LOG_POS_TITLE);
	for (int j=0; j<NI_TOTAL_JOINT_NUM; j++) {
		int n = Ni2SDKRotJointNum(j, sdk_lib);
		if (n>=0) fprintf(pLogFile, ", %s", NiJointName(j).c_str());
	}
	fprintf(pLogFile, "\n");
	*/

	free_Buffer(&path);
	free_Buffer(&name);
	return TRUE;
}




void  CExNiFileDev::closeLogFile(void)
{
	if (pLogFile!=NULL) {
		fclose(pLogFile);
		pLogFile = NULL;
	}
}




void  CExNiFileDev::writeLogData(Vector<float>* pos, Quaternion<float>* rot, float* agl, BOOL mirror, NiSDK_Lib lib)
{
	writeJTextData(pLogFile, pos, rot, agl, mirror, lib);

	/*
	unsigned short ctime;
	log_ttl_time += GetMsecondsLapTimer(log_lap_time, &ctime);
	log_lap_time  = ctime;

	fprintf(pLogFile, "%ld, %f, %f, %f", log_ttl_time, pos.x, pos.y, pos.z);
	for (int j=0; j<NI_TOTAL_JOINT_NUM; j++) {
		int n = Ni2SDKRotJointNum(j, sdk_lib);
		if (n>=0) fprintf(pLogFile, ", %f", niJntAngl[j]*RAD2DEGREE);
	}
	fprintf(pLogFile, "\n");
	*/
}



