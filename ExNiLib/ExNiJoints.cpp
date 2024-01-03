
#include  "stdafx.h"

#include  "WinTools.h"
#include  "ExNiJoints.h"
#include  "NiToolWin.h"
#include  "NiJointsTool.h"

#include  "Graph.h"





using namespace jbxl;
using namespace jbxwl;





CExNiJoints::CExNiJoints(void)
{ 
	init();
}




CExNiJoints::CExNiJoints(NiSDK_Lib lib)	: CNiJoints(lib)
{
	init();
}




void  CExNiJoints::init(void)
{
	appParam.init();
	
	//
	pViewData = NULL;

	memset(&faceRect,  0, sizeof(rectangle));
	memset( eyesRect,  0, sizeof(rectangle)*2);
	memset(&mouthRect, 0, sizeof(rectangle));
	
	enableOpencvFace  = false;
	enableOpencvEyes  = false;
	enableOpencvMouth = false;

	isDetectFace = FALSE;
	isDetectEyes = FALSE;

//	pLogFile  = NULL;

	//
	mvav_type = MVAV_Expo;
	mvav_num  = 3;

	noise_min = 0.02f;		// 2cm
}




void  CExNiJoints::clear(void)
{
	init();
	CNiJoints::clear();
}




void  CExNiJoints::setParameter(CParameterSet param)
{
	//
	appParam	= param;

	//
	confidence	= param.confidence;
	mvav_type	= param.mvavType;
	mvav_num	= param.mvavNum;

	initMvavWeight();
}





/////////////////////////////////////////////////////////////////////////////////////////////
//
// Face Tracking
//

void  CExNiJoints::execFaceTracking(void)
{

// OpenCV
#ifdef ENABLE_OPENCV
	execOpencvFaceTracking();
#endif

	return;
}




void  CExNiJoints::drawFaceTracking(int color, int line)
{

// OpenCV
#ifdef ENABLE_OPENCV
	drawOpencvFaceTracking(color, line);
#endif

	return;
}





//////////////////////////////////////////////////////////////////////////////
// 
// OpenCV Face Tracking
//


BOOL  CExNiJoints::init_opencv(void)
{
#ifdef ENABLE_OPENCV
	//
	clear_opencv();

	char* path = GetProgramFolderA();

	std::string opencvFacePath  = path;
	std::string opencvEyesPath  = path;
	std::string opencvMouthPath = path;

	opencvFacePath  += OPENCV_FACE_FILE;
	opencvEyesPath  += OPENCV_EYES_FILE;
	opencvMouthPath += OPENCV_MOUTH_FILE;

	enableOpencvFace = opencvFace.load(opencvFacePath);
	if (enableOpencvFace) {
		enableOpencvEyes  = opencvEyes.load (opencvEyesPath);
		enableOpencvMouth = opencvMouth.load(opencvMouthPath);
	}

	::free(path);

	return TRUE;
#else

	return FALSE;
#endif
}




void  CExNiJoints::clear_opencv(void)
{
	//
	enableOpencvFace  = false;
	enableOpencvEyes  = false;
	enableOpencvMouth = false;
}



#ifdef ENABLE_OPENCV


void  CExNiJoints::execOpencvFaceTracking(void)
{
	memset(&faceRect,  0, sizeof(rectangle));
	memset( eyesRect,  0, sizeof(rectangle)*2);
	memset(&mouthRect, 0, sizeof(rectangle));

	isDetectFace  = FALSE;
	isDetectEyes  = FALSE;
	isDetectMouth = FALSE;

	//
	if (!enableOpencvFace) return;
	if (pViewData==NULL || *pViewData==NULL) return;

	//
	MSGraph<uByte>* vp = new MSGraph<uByte>((*pViewData)->xsize, (*pViewData)->ysize);
	int j = 0;
	for (int i=0; i<vp->xs*vp->ys; i++) {
		int b = (int)((*pViewData)->grptr[j++]);	// Blue
		int g = (int)((*pViewData)->grptr[j++]);	// Green
		int r = (int)((*pViewData)->grptr[j++]);	// Red
		j++;										// Alpha
		vp->gp[i] = (uByte)((b + g + r)/3);
		//vp->gp[i] = (uByte)(b*0.11 + g*0.59 + r*0.30);
	}

	//
	Vector<int> shd = crdVect[NI_SDK_L_SHLDR] - crdVect[NI_SDK_R_SHLDR];
	Vector<int> n2h = crdVect[NI_SDK_HEAD]    - crdVect[NI_SDK_NECK];

	int headx = crdVect[NI_SDK_HEAD].x;
	int heady = crdVect[NI_SDK_HEAD].y;

	// パラメータ依存
	int xsize = (int)(shd.norm()*0.8);
	int ysize = (int)(n2h.norm()*1.2);
	//
	if (sdk_lib==NiSDK_Kinect) {
		heady += ysize/4;
	}
	
	//
	int sz = vp->xs;
	int sx = Max(headx-xsize/2, 0);
	int sy = Max(heady-ysize/2, 0);
	int ex = Min(headx+xsize/2, sz-1);
	int ey = Min(heady+ysize/2, sz-1);

	MSGraph<uByte>* xp = grab_MSGraph(vp, sx, sy, ex, ey);
	vp->free();
	delete(vp);

	/*
	// ROI
	MSGraph<unsigned int> gp;
	gp.xs = (*pViewData)->xsize;
	gp.ys = (*pViewData)->ysize;
	gp.gp = (unsigned int*)((*pViewData)->grptr);
	MSGraph_Box(gp, sx, sy, ex, ey, BGRA2Int(255,0,0,0));
	*/

	//
	if (headx-xsize/2>=0 && headx+xsize/2<sz) {
		int    num   = 0;
		float scale = Min(xsize, ysize)/64.0;
		rectangle* face = cvDetectObjects(opencvFace, xp, num, 24, scale);

		faceRect.x = 0;
		faceRect.y = 0;
		faceRect.xsize = ex - sx;
		faceRect.ysize = ey - sy;

		//
		isDetectFace = TRUE;
		if (num>0) {
			//isDetectFace = TRUE;
			faceRect = *face;
		}
		if (face!=NULL) ::free(face);
	}


	if (isDetectFace) {
		//
		// Eyes Detect
		if (enableOpencvEyes) {
			MSGraph<uByte>* zp = grab_MSGraph(xp, faceRect.x, faceRect.y, faceRect.x+faceRect.xsize, faceRect.y+faceRect.ysize);	
				
			int mum = 0;
			rectangle* eyes = cvDetectObjects(opencvEyes, zp, mum, 4, 1.0);
			if (mum>0) {
				isDetectEyes = TRUE;
				//
				mum = Min(mum, 2);
				for (int j=0; j<mum; j++) {
					eyesRect[j] = eyes[j];
				}
			}
			if (eyes!=NULL) ::free(eyes);
			zp->free();
			delete(zp);
		}

		//
		// Mouth Detect
		if (enableOpencvMouth) {
			MSGraph<uByte>* zp = grab_MSGraph(xp, faceRect.x, faceRect.y+faceRect.ysize/2, faceRect.x+faceRect.xsize, faceRect.y+faceRect.ysize);	
			//
			int mum = 0;
			rectangle* mouth = cvDetectObjects(opencvMouth, zp, mum, 4, 1.0);
			if (mum>0) {
				isDetectMouth = TRUE;
				mouthRect = *mouth;
			}
			if (mouth!=NULL) ::free(mouth);
			zp->free();
			delete(zp);
		}

		//
		faceRect.x += sx;
		faceRect.y += sy;
	}

	xp->free();
	delete(xp);
}




void  CExNiJoints::drawOpencvFaceTracking(int color, int line)
{
	if (!enableOpencvFace) return;
	if (pViewData==NULL   || *pViewData==NULL)  return;
	if (faceRect.xsize==0 || faceRect.ysize==0) return;

	MSGraph<unsigned int> vp;
	vp.xs = (*pViewData)->xsize;
	vp.ys = (*pViewData)->ysize;
	vp.gp = (unsigned int*)((*pViewData)->grptr);

	MSGraph_Box(vp, faceRect.x, faceRect.y, faceRect.x+faceRect.xsize, faceRect.y+faceRect.ysize, color);
	for (int i=1; i<line; i++) {
		MSGraph_Box(vp, faceRect.x-i, faceRect.y-i, faceRect.x+faceRect.xsize+i, faceRect.y+faceRect.ysize+i, color);
		MSGraph_Box(vp, faceRect.x+i, faceRect.y+i, faceRect.x+faceRect.xsize-i, faceRect.y+faceRect.ysize-i, color);
	}

	//
	if (enableOpencvEyes) {
		for (int i=0; i<2; i++) {
			if (eyesRect[i].xsize!=0 && eyesRect[i].ysize!=0) {
				int cx = faceRect.x + eyesRect[i].x + eyesRect[i].xsize/2; 
				int cy = faceRect.y + eyesRect[i].y + eyesRect[i].ysize/2; 
				int rr = (eyesRect[i].xsize + eyesRect[i].ysize)/4;
				MSGraph_Circle(vp, cx, cy, rr, color);
				for (int i=1; i<line; i++) {
					MSGraph_Circle(vp, cx, cy, rr+i, color);
					MSGraph_Circle(vp, cx, cy, rr-i, color);
				}
			}
		}
	}

	if (enableOpencvMouth) {
		int xs = faceRect.x + mouthRect.x; 
		int ys = faceRect.y + faceRect.ysize/2 + mouthRect.y; 
		MSGraph_Box(vp, xs, ys, xs+mouthRect.xsize, ys+mouthRect.ysize, color);
		for (int i=1; i<line; i++) {
			MSGraph_Box(vp, xs-i, ys-i, xs+mouthRect.xsize+i, ys+mouthRect.ysize+i, color);
			MSGraph_Box(vp, xs+i, ys+i, xs+mouthRect.xsize-i, ys+mouthRect.ysize-i, color);
		}
	}
}




#endif	// ENABLE_OPENCV








/////////////////////////////////////////////////////////////////////////////////////////////
//
// 平滑化
//

void  CExNiJoints::initMvavWeight(void)
{
	mvav_num = Min(mvav_num, NI_MVAV_MAX_NUM);
	mvav_num = Max(mvav_num, 1);

	if (mvav_type==MVAV_Simple) {
		for (int i=0; i<mvav_num; i++) {
			mvav_weight[i] = 1.0f/mvav_num;
		}
	}
	//
	else if (mvav_type==MVAV_Weight) {
		float sum = (mvav_num+1.0f)*mvav_num/2.0f;
		for (int i=0; i<mvav_num; i++) {
			mvav_weight[i] = (mvav_num - i)/sum;
		}
	}
	//
	else if (mvav_type==MVAV_Expo) {
		float exp_param = (float)(NI_MVAV_EXP_DEC*LN_10/mvav_num);
		float sum = 1.0f;
		mvav_weight[0] = 1.0f;
		//
		for (int i=1; i<mvav_num; i++) {
			float weight = (float)exp(-exp_param*i);
			mvav_weight[i] = weight;
			sum += weight;
		}
		for (int i=0; i<mvav_num; i++) {
			mvav_weight[i] = mvav_weight[i]/sum;
		}
	}
	//
	else {
		mvav_weight[0] = 1.0f;
		for (int i=1; i<mvav_num; i++) {
			mvav_weight[i] = 0.0f;
		}
	}
}





void  CExNiJoints::PosMovingAverage(NiSDK_Lib sdk_lib)
{
	if (mvav_num<2) return;
	if (posVect==NULL) return;

	for (int j=0; j<NI_NORML_JOINT_NUM; j++) {
		//
		int joint = Ni2SDKPosJointNum(j, sdk_lib);
		if (joint>=0) {
			//
			Vector<float> vect = posVect[joint]*mvav_weight[0];
			float sum = mvav_weight[0];

			for (int i=1; i<mvav_num; i++) {
				Vector<float>* pv = (Vector<float>*)posRing[joint].get(-i);
				if (pv!=NULL && pv->c>0.0f) {
					vect = vect + (*pv)*mvav_weight[i];
					sum += mvav_weight[i];
				}
			}

			posVect[joint] = vect/sum;
		}
	}
}




void  CExNiJoints::RotMovingAverage(NiSDK_Lib sdk_lib)
{
	if (mvav_num<2) return;
	if (rotQuat==NULL) return;

	for (int j=0; j<NI_NORML_JOINT_NUM; j++) {
		//
		int joint = Ni2SDKRotJointNum(j, sdk_lib);
		if (joint>=0) {
			//
			Quaternion<float> quat = rotQuat[joint]*mvav_weight[0];
			float sum = mvav_weight[0];

			for (int i=1; i<mvav_num; i++) {
				Quaternion<float>* pq = (Quaternion<float>*)rotRing[joint].get(-i);
				if (pq!=NULL && pq->c>0.0f) {
					quat = quat + (*pq)*mvav_weight[i];
					sum += mvav_weight[i];
				}
			}

			rotQuat[joint] = quat/sum;
		}		
	}

}






/////////////////////////////////////////////////////////////////////////////////////////////
//
// 制約条件
//

void  CExNiJoints::PosVibNoiseCanceler(void)
{
	if (posVect==NULL || posRing==NULL) return;

	for (int j=0; j<NI_NORML_JOINT_NUM; j++) {
		//
		int joint = Ni2SDKPosJointNum(j, sdk_lib);
		if (joint>=0) {
			//
			Vector<float> v[2], d[2];
			memset(&v, 0, sizeof(Vector<float>)*2);

			for (int n=0, i=0; i<NI_MVAV_MAX_NUM; i++) {
				Vector<float>* pv = (Vector<float>*)posRing[joint].get(-i);
				if (pv!=NULL && pv->c>0.0) {
					v[n] = *pv;
					if (n==1) break;
					n++;
				}
			}
	
			if (v[1].c>0.0) {
				d[0] = posVect[joint] - v[0];
				d[1] = v[0] - v[1];
				d[0].norm();
				d[1].norm();
				if (d[0]*d[1]<-0.71 && d[0].n<noise_min && d[1].n<noise_min) {
					posVect[joint] = v[0];
				}
			}
		}
	}
}




void  CExNiJoints::RotVibNoiseCanceler(void)
{
	if (rotQuat==NULL || rotRing==NULL) return;

	for (int j=0; j<NI_NORML_JOINT_NUM; j++) {
		//
		int joint = Ni2SDKRotJointNum(j, sdk_lib);
		if (joint>=0) {
			//
			Quaternion<float> qt;
			memset(&qt, 0, sizeof(Quaternion<float>));

			for (int i=0; i<NI_MVAV_MAX_NUM; i++) {
				Quaternion<float>* pq = (Quaternion<float>*)rotRing[joint].get(-i);
				if (pq!=NULL && pq->c>0.0) {
					qt = *pq;
					break;
				}
			}
	
			if (qt.c>0.0) {
				Vector<float> v1 = rotQuat[joint].getVector();
				Vector<float> v2 = qt.getVector();
				float th = rotQuat[joint].getAngle(); 
				if (v1*v2<-0.71f && th<0.0873f) {	// 5Deg
					rotQuat[joint] = qt;
				}
			}
		}
	}
}




void  CExNiJoints::CheckGroundLevel(float ground_level)
{
	if (posVect==NULL) return;


	// 誤差が大きすぎる
	/*
	if (posVect[NI_SDK_L_ANKLE].z<ground_level) {
		posVect[NI_SDK_L_FOOT].z += ground_level - posVect[NI_SDK_L_ANKLE].z;
		posVect[NI_SDK_L_ANKLE].z = ground_level;
	}
	if (posVect[NI_SDK_R_ANKLE].z<ground_level) {
		posVect[NI_SDK_R_FOOT].z += ground_level - posVect[NI_SDK_R_ANKLE].z;
		posVect[NI_SDK_R_ANKLE].z = ground_level;
	}
	*/

	return;
}





void  CExNiJoints::CheckBoneLength(void)
{
	// Not Yet
	return;

	if (posVect==NULL) return;


	// HIP - KNEE
	if (posVect[NI_SDK_R_KNEE].c>=confidence) {
		float dist_rhk = VectorDist(posVect[NI_SDK_R_HIP], posVect[NI_SDK_R_KNEE]);
		if (dist_rhk<0.20 || dist_rhk>1.20) {
			posVect[NI_SDK_R_KNEE].c  = -1.0;
			posVect[NI_SDK_R_ANKLE].c = -1.0;
			posVect[NI_SDK_R_FOOT].c  = -1.0;
		}
	}
	if (posVect[NI_SDK_L_KNEE].c>=confidence) {
		float dist_lhk = VectorDist(posVect[NI_SDK_L_HIP], posVect[NI_SDK_L_KNEE]);
		if (dist_lhk<0.20 || dist_lhk>1.20) {
			posVect[NI_SDK_L_KNEE].c  = -1.0;
			posVect[NI_SDK_L_ANKLE].c = -1.0;
			posVect[NI_SDK_L_FOOT].c  = -1.0;
		}
	}

	// KNEE - ANKLE
	if (posVect[NI_SDK_R_ANKLE].c>=confidence) {
		float dist_rka = VectorDist(posVect[NI_SDK_R_KNEE], posVect[NI_SDK_R_ANKLE]);
		if (dist_rka<0.20 || dist_rka>1.20) {
			posVect[NI_SDK_R_ANKLE].c = -1.0;
			posVect[NI_SDK_R_FOOT].c  = -1.0;
		}
	}
	if (posVect[NI_SDK_L_ANKLE].c>=confidence) {
		float dist_lka = VectorDist(posVect[NI_SDK_L_KNEE], posVect[NI_SDK_L_ANKLE]);
		if (dist_lka<0.20 || dist_lka>1.20) {
			posVect[NI_SDK_L_ANKLE].c = -1.0;
			posVect[NI_SDK_L_FOOT].c  = -1.0;
		}
	}
}






//////////////////////////////////////////////////////////////////////////
//
// 回転の制約
// 

void  CExNiJoints::CheckJointsRotation(void)
{
	if (rotQuat==NULL) return;

	NiCorrectJointsRotation(rotQuat, sdk_lib);

	return;
}





///////////////////////////////////////////////////
//
// 制約条件の設定
//

void  jbxwl::initRBoundJointsRotation()
{
	NiInitRBoundJointsRotation();

	setRBoundJointsRotation();
	d2rRBoundJointsRotation();
}







////////////////////////////////////////////////////////////////////////////////////////////////

/**
X-Rotation, Y-Rotation, Z-Rotation の範囲を指定する．単位は度．

tmin!=0.0 の場合に条件が適用される．

@see http://wiki.secondlife.com/wiki/Suggested_BVH_Joint_Rotation_Limits

*/
void  jbxwl::setRBoundJointsRotation()
{
	// Second Life
//	RBound_NI_TORSO.set   ( -30.0,  30.0,  -45.0,  68.0,  -45.0,  45.0, 1.0);	// ZXY
	RBound_NI_CHEST.set   ( -30.0,  30.0,  -45.0,  22.0,  -45.0,  45.0, 1.0);	// ZXY
	RBound_NI_NECK.set    ( -30.0,  30.0,  -37.0,  22.0,  -45.0,  45.0, 1.0);	// ZXY
	RBound_NI_HEAD.set    ( -30.0,  30.0,  -37.0,  22.0,  -45.0,  45.0, 1.0);	// ZXY

	RBound_NI_L_COLLAR.set( -30.0,  30.0,    0.0,   0.0,  -30.0,  10.0, 1.0);	// YXZ
	RBound_NI_R_COLLAR.set( -30.0,  30.0,    0.0,   0.0,  -10.0,  30.0, 1.0);	// YXZ
	RBound_NI_L_SHLDR.set ( -91.0,  97.0, -135.0,  90.0, -180.0,  98.0, 1.0);	// YZX
	RBound_NI_R_SHLDR.set ( -97.0,  91.0, -135.0,  90.0,  -98.0, 180.0, 1.0);	// YZX
	RBound_NI_L_ELBOW.set (   0.0,   0.0,  -90.0,  79.0, -146.0,   0.0, 1.0);	// YXZ
	RBound_NI_R_ELBOW.set (   0.0,   0.0,  -90.0,  79.0,    0.0, 146.0, 1.0);	// YXZ
	RBound_NI_L_WRIST.set ( -90.0,  86.0,  -45.0,  45.0,  -25.0,  36.0, 1.0);	// YZX
	RBound_NI_R_WRIST.set ( -86.0,  90.0,  -45.0,  45.0,  -36.0,  25.0, 1.0);	// YZX

	RBound_NI_L_HIP.set   ( -17.0,  88.0, -155.0,  45.0,  -85.0, 105.0, 1.0);	// ZXY
	RBound_NI_R_HIP.set   ( -88.0,  17.0, -155.0,  45.0, -105.0,  85.0, 1.0);	// ZXY
	RBound_NI_L_KNEE.set  (   0.0,   0.0,    0.0, 150.0,    0.0,   0.0, 1.0);	// ZXY
	RBound_NI_R_KNEE.set  (   0.0,   0.0,    0.0, 150.0,    0.0,   0.0, 1.0);	// ZXY
	RBound_NI_L_ANKLE.set ( -74.0,  15.0,  -31.0,  63.0,  -26.0,  26.0, 1.0);	// XZY
	RBound_NI_R_ANKLE.set ( -15.0,  74.0,  -31.0,  63.0,  -26.0,  26.0, 1.0);	// XZY


	// QAvimator
	RBound_NI_TORSO.set   ( -40.0,  40.0,  -50.0,  90.0,  -35.0,  35.0, 1.0);
//	RBound_NI_CHEST.set   ( -30.0,  30.0,  -40.0,  20.0,  -40.0,  40.0, 1.0);
//	RBound_NI_NECK.set    ( -40.0,  40.0,  -50.0,  50.0,  -60.0,  60.0, 1.0);
//	RBound_NI_HEAD.set    ( -30.0,  30.0,  -90.0,  40.0,  -50.0,  50.0, 1.0);
//
//	RBound_NI_L_COLLAR.set( -30.0,  30.0,  -10.0,  10.0,  -30.0,  10.0, 1.0);
//	RBound_NI_R_COLLAR.set( -30.0,  30.0,  -10.0,  10.0,  -10.0,  30.0, 1.0);
//	RBound_NI_L_SHLDR.set (-105.0, 105.0,  -60.0,  30.0, -130.0,  80.0, 1.0);
//	RBound_NI_R_SHLDR.set (-105.0, 105.0,  -60.0,  30.0,  -80.0, 130.0, 1.0);
//	RBound_NI_L_ELBOW.set (  -5.0,   5.0,  -20.0,  20.0, -146.0,  13.0, 1.0);
//	RBound_NI_R_ELBOW.set (  -5.0,   5.0,  -20.0,  20.0,  -13.0, 146.0, 1.0);
//	RBound_NI_L_WRIST.set ( -80.0,  80.0,  -60.0,  30.0,  -20.0,  20.0, 1.0);
//	RBound_NI_R_WRIST.set ( -80.0,  80.0,  -60.0,  30.0,  -20.0,  20.0, 1.0);
//	
//	RBound_NI_L_HIP.set   ( -27.0,  60.0, -160.0,  80.0,  -30.0,  50.0, 1.0);
//	RBound_NI_R_HIP.set   ( -60.0,  27.0, -160.0,  80.0,  -50.0,  30.0, 1.0);
//	RBound_NI_L_KNEE.set  (  -5.0,   5.0,  -20.0, 155.0,  -20.0,  20.0, 1.0);
//	RBound_NI_R_KNEE.set  (  -5.0,   5.0,  -20.0, 155.0,  -20.0,  20.0, 1.0);
//	RBound_NI_L_ANKLE.set ( -30.0,  30.0,  -50.0, 100.0,  -30.0,  30.0, 1.0);
//	RBound_NI_R_ANKLE.set ( -30.0,  30.0,  -50.0, 100.0,  -30.0,  30.0, 1.0);

	//
	// Modify for Rinions
	RBound_NI_L_ELBOW.set(-146.0, 146.0, -90.0, 79.0, -146.0,  13.0, 1.0);
	RBound_NI_R_ELBOW.set(-146.0, 146.0, -90.0, 79.0,  -13.0, 146.0, 1.0);

	//
	readRBoundJointsRotation(JOINT_RSTRCTN_TXT);

	return;
}





//
//
void  jbxwl::d2rRBoundJointsRotation()
{
	// degree -> radian
	RBound_NI_TORSO.multiple((float)DEGREE2RAD);
	RBound_NI_CHEST.multiple((float)DEGREE2RAD);
	RBound_NI_NECK.multiple((float)DEGREE2RAD);
	RBound_NI_HEAD.multiple((float)DEGREE2RAD);

	RBound_NI_L_COLLAR.multiple((float)DEGREE2RAD);
	RBound_NI_R_COLLAR.multiple((float)DEGREE2RAD);
	RBound_NI_L_SHLDR.multiple((float)DEGREE2RAD);
	RBound_NI_R_SHLDR.multiple((float)DEGREE2RAD);
	RBound_NI_L_ELBOW.multiple((float)DEGREE2RAD);
	RBound_NI_R_ELBOW.multiple((float)DEGREE2RAD);
	RBound_NI_L_WRIST.multiple((float)DEGREE2RAD);
	RBound_NI_R_WRIST.multiple((float)DEGREE2RAD);
	
	RBound_NI_L_HIP.multiple((float)DEGREE2RAD);
	RBound_NI_R_HIP.multiple((float)DEGREE2RAD);
	RBound_NI_L_KNEE.multiple((float)DEGREE2RAD);
	RBound_NI_R_KNEE.multiple((float)DEGREE2RAD);
	RBound_NI_L_ANKLE.multiple((float)DEGREE2RAD);
	RBound_NI_R_ANKLE.multiple((float)DEGREE2RAD);

	return;
}




void  jbxwl::readRBoundJointsRotation(char* fname)
{
	RBound<float> rb;
	char joint[LBUF];
	//
	char* path = GetProgramFolderA();

	std::string jointRstrctnPath = path;
	jointRstrctnPath  += fname;

	FILE* fp = fopen(jointRstrctnPath.c_str(), "r");
	if (fp==NULL) return;

	rb.tmin = rb.tmax = 1.0;

	fgets(joint, LBUF, fp);

	while (!feof(fp)) {
		char* name = cawk(joint, ' ', 1);
		char* xmin = cawk(joint, ' ', 2);	
		char* xmax = cawk(joint, ' ', 3);	
		char* ymin = cawk(joint, ' ', 4);	
		char* ymax = cawk(joint, ' ', 5);	
		char* zmin = cawk(joint, ' ', 6);	
		char* zmax = cawk(joint, ' ', 7);

		if (name!=NULL && xmin!=NULL && xmax!=NULL && ymin!=NULL && ymax!=NULL && zmin!=NULL && zmax!=NULL) {
			//
			rb.xmin = (float)atof(xmin);
			rb.xmax = (float)atof(xmax);
			rb.ymin = (float)atof(ymin);
			rb.ymax = (float)atof(ymax);
			rb.zmin = (float)atof(zmin);
			rb.zmax = (float)atof(zmax);

			if		(!strcasecmp(joint, "TORSO"))    RBound_NI_TORSO    = rb;
			else if (!strcasecmp(joint, "NECK"))     RBound_NI_NECK     = rb;
			else if (!strcasecmp(joint, "HEAD"))     RBound_NI_HEAD     = rb;
			else if (!strcasecmp(joint, "CHEST"))    RBound_NI_CHEST    = rb;
			else if (!strcasecmp(joint, "L_COLLAR")) RBound_NI_L_COLLAR = rb;
			else if (!strcasecmp(joint, "L_SHLDR"))  RBound_NI_L_SHLDR  = rb;
			else if (!strcasecmp(joint, "L_ELBOW"))  RBound_NI_L_ELBOW  = rb;
			else if (!strcasecmp(joint, "L_WRIST"))  RBound_NI_L_WRIST  = rb;
			else if (!strcasecmp(joint, "R_COLLAR")) RBound_NI_R_COLLAR = rb;
			else if (!strcasecmp(joint, "R_SHLDR"))  RBound_NI_R_SHLDR  = rb;
			else if (!strcasecmp(joint, "R_ELBOW"))  RBound_NI_R_ELBOW  = rb;
			else if (!strcasecmp(joint, "R_WRIST"))  RBound_NI_R_WRIST  = rb;
			else if (!strcasecmp(joint, "L_HIP"))    RBound_NI_L_HIP    = rb;
			else if (!strcasecmp(joint, "L_KNEE"))   RBound_NI_L_KNEE   = rb;
			else if (!strcasecmp(joint, "L_ANKLE"))  RBound_NI_L_ANKLE  = rb;
			else if (!strcasecmp(joint, "R_HIP"))    RBound_NI_R_HIP    = rb;
			else if (!strcasecmp(joint, "R_KNEE"))   RBound_NI_R_KNEE   = rb;
			else if (!strcasecmp(joint, "R_ANKLE"))  RBound_NI_R_ANKLE  = rb;
		}

		if (name!=NULL) free(name);
		if (xmin!=NULL) free(xmin);
		if (xmax!=NULL) free(xmax);
		if (ymin!=NULL) free(ymin);
		if (ymax!=NULL) free(ymax);
		if (zmin!=NULL) free(zmin);
		if (zmax!=NULL) free(zmax);
		//
		fgets(joint, LBUF, fp);
	}

	fclose(fp);

	return;
}


