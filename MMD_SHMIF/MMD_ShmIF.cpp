
/*
MMD用 Rinions Shared Memory インターフェイス v0.9 
                                            by Fumi.Iseki

*/



#include  "stdafx.h"

#include  "MMD_ShmIF.h"
#include  "SHMIF.h"
#include  "AnimationDLG.h"

// DLG
#include  "resource.h"
#include  "WinTools.h"
#include  "..\Rinions_config.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif


using namespace jbxl;
using namespace jbxwl;





CSHMInterFace*	ShmIF = NULL;

D3DXVECTOR3		BP_Vector[MMD_JOINT_NUM];





/*
共有メモリ名
    mPelvis(0), mTorso(1), mChest(2), mNeck(3), mHead(4), mSkull(5), 
	mEyeLeft(6), mEyeRight(7), mBustLeft(8), mBustRight(9),
    mCollarLeft (10), mShoulderLeft (11), mElbowLeft (12), mWristLeft (13), mFingertipLeft (14),
    mCollarRight(15), mShoulderRight(16), mElbowRight(17), mWristRight(18), mFingertipRight(19),
    mHipLeft (20), mKneeLeft (21), mAnkleLeft (22), mFootLeft (23), mToeLeft (24),
    mHipRight(25), mKneeRight(26), mAnkleRight(27), mFootRight(28), mToeRight(29), 
    L_Hand(30), R_Hand(31), Expression(32), Rinions(33: for Control)
*/


static  int  _MMD2SHMJoint[] =		// MMD_JOINT_NUM
{
	 0,  3,  4, 
	16, 17, 18, 11, 12, 13, 
    25, 26, 28, 20, 21, 23, 
	 1, 18, 13,
	28, 23, 16, 11, -1
};





Vector<double> get_joint_position(int num)
{
	Vector<double> vect(0.0, 0.0, 0.0);

	int index = _MMD2SHMJoint[num];
	if (index>=0) {
		std::string shm_name = SHMJointName(index);
		vect = ShmIF->getAnimationPosition(shm_name);
	}

	return vect;
}




Quaternion<double> get_joint_rotation(int num)
{
	Quaternion<double> quat(1.0, 0.0, 0.0, 0.0, 1.0);

	int index = _MMD2SHMJoint[num];
	if (index>=0) {
		std::string shm_name = SHMJointName(index);
		quat = ShmIF->getAnimationRotation(shm_name);
	}

	return quat;
}




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 唯一のアプリケーション オブジェクト
// DEBUGモードで必要
//

CWinApp theApp;

using namespace std;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	// MFC を初期化して、エラーの場合は結果を印刷します。
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0)) {
		// TODO: 必要に応じてエラー コードを変更してください。
		_tprintf(_T("致命的なエラー: MFC の初期化ができませんでした\n"));
		nRetCode = 1;
	}
	else{
		// TODO: アプリケーションの動作を記述するコードをここに挿入してください。
	}

	return nRetCode;
}





/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//

MMD_SHM_API  bool __stdcall OpenNIInit(HWND hWnd, bool EngFlag, LPDIRECT3DDEVICE9 lpDevice, WCHAR* f_path, CHAR* onifilename)
{	
	//DEBUG_INFO("OpenNIInit");

	if (onifilename!=NULL) {
		MessageDLG(_T("注意!?"), _T("ここからファイルを読み込むことは出来ません\nRinionsで読み込んでください"), MB_OK, hWnd);
		return false;
	}

	//
	CString animUUID = _T(SHMIF_DEFAULT_ANIM);

	// Rinions の設定ファイルを読む
	CString config = MakeWorkingFolderPath(_T(RINIONS_CONFIG_FILE), FALSE, _T(RINIOMS_CONFIG_PATH), TRUE);
	char* configpath = ::ts2mbs(config);
	tList* lt = read_index_tList_file(configpath, ' ');
	if (lt!=NULL) {
		animUUID = get_tstr_param_tList(lt, "animationUUID", (LPCTSTR)animUUID);
	}
	free(configpath);


	// 表示＆入力用ダイアログ
	CAnimationDLG* anmdlg = new CAnimationDLG(animUUID);
	if (anmdlg==NULL) return false;
	
	anmdlg->DoModal();
	animUUID = anmdlg->getAnimationUUID();
	delete(anmdlg);
	

	// IFの生成
	char* uuid = ::ts2mbs(animUUID);
	ShmIF = new CSHMInterFace(uuid);
	free(uuid);
	if (ShmIF==NULL) return false;

	return true;
}




MMD_SHM_API  void __stdcall OpenNIClean()
{
	//DEBUG_INFO("OpenNIClean");

	if (ShmIF!=NULL) {
		delete(ShmIF);
		ShmIF = NULL;
	}
}




MMD_SHM_API  void __stdcall OpenNIDrawDepthMap(bool waitflag)
{
	//DEBUG_INFO("OpenNIDrawDepthMap");

	Vector<double> vect;
	D3DXVECTOR3  cshldr;
	
	if (ShmIF==NULL) return;

	//
	ShmIF->checkChannelIndexShm();

	if (ShmIF->isSHMemory) {
		for (int num=0; num<MMD_JOINT_NUM; num++) {
			/*
			if (num==MMD_HEAD_FRONT) {
				Quaternion quat = get_joint_rotation(MMD_NECK);
				vect = quat.getVector();
			}
			else {
				vect = get_joint_position(num);
			}
			*/

			vect = get_joint_position(num);
			
			BP_Vector[num].x =  (float)vect.y*1000.0f;
			BP_Vector[num].y =  (float)vect.z*1000.0f;
			BP_Vector[num].z = -(float)vect.x*1000.0f;


			// 上半身モードでの補正
			if (ShmIF->isProfUpper) {
				if (num==MMD_R_SHOULDER) {
					cshldr.x = (BP_Vector[MMD_R_SHOULDER].x + BP_Vector[MMD_L_SHOULDER].x)*0.50f;
					cshldr.z = (BP_Vector[MMD_R_SHOULDER].z + BP_Vector[MMD_L_SHOULDER].z)*0.50f;
				}
				//
				if (num==MMD_R_HIP) {
					BP_Vector[num].x = BP_Vector[MMD_R_SHOULDER].x;
					BP_Vector[num].y = BP_Vector[MMD_CENTER].y;
					BP_Vector[num].z = BP_Vector[MMD_R_SHOULDER].z;
				}
				else if (num==MMD_L_HIP) {
					BP_Vector[num].x = BP_Vector[MMD_L_SHOULDER].x;
					BP_Vector[num].y = BP_Vector[MMD_CENTER].y;
					BP_Vector[num].z = BP_Vector[MMD_L_SHOULDER].z;
				}
				else if (num==MMD_R_KNEE) {
					BP_Vector[num].x = BP_Vector[MMD_R_SHOULDER].x*0.80f + cshldr.x*0.20f;
					BP_Vector[num].y = BP_Vector[MMD_CENTER].y - 800.0f;
					BP_Vector[num].z = BP_Vector[MMD_R_SHOULDER].z*0.80f + cshldr.z*0.20f;
				}
				else if (num==MMD_L_KNEE) {
					BP_Vector[num].x = BP_Vector[MMD_L_SHOULDER].x*0.80f + cshldr.x*0.20f;
					BP_Vector[num].y = BP_Vector[MMD_CENTER].y - 800.0f;
					BP_Vector[num].z = BP_Vector[MMD_L_SHOULDER].z*0.80f + cshldr.z*0.20f;
				}
				else if (num==MMD_R_FOOT) {
					BP_Vector[num].x = BP_Vector[MMD_R_SHOULDER].x*0.60f + cshldr.x*0.40f;
					BP_Vector[num].y = BP_Vector[MMD_CENTER].y - 1500.0f;
					BP_Vector[num].z = BP_Vector[MMD_R_SHOULDER].z*0.60f + cshldr.z*0.40f;
				}
				else if (num==MMD_L_FOOT) {
					BP_Vector[num].x = BP_Vector[MMD_L_SHOULDER].x*0.60f + cshldr.x*0.40f;
					BP_Vector[num].y = BP_Vector[MMD_CENTER].y - 1500.0f;
					BP_Vector[num].z = BP_Vector[MMD_L_SHOULDER].z*0.60f + cshldr.z*0.40f;
				}
			}
		}
	}
}




MMD_SHM_API  void __stdcall OpenNIDepthTexture(IDirect3DTexture9** lpTex)
{
	*lpTex = NULL;
}




MMD_SHM_API  void __stdcall OpenNIGetSkeltonJointPosition(int num, D3DXVECTOR3* vec)
{
	if (ShmIF==NULL) return;
	
	if (ShmIF->isTracking) {
		if (BP_Vector[num].x==0.0f && BP_Vector[num].y==0.0f && BP_Vector[num].z==0.0f) return;
		*vec = BP_Vector[num];
	}
	//DEBUG_ERR("Pos %d: (%f, %f, %f)", num, vec->z, vec->x, vec->y);
}




MMD_SHM_API  void __stdcall OpenNIIsTracking(bool* lpb)
{
	//DEBUG_INFO("OpenNIIsTracking");

	*lpb = false;

	if (ShmIF==NULL) return;

	if (ShmIF->isTracking) {
		*lpb = true;
	}

	return;
}




MMD_SHM_API  void __stdcall OpenNIGetVersion(float* ver)
{
	*ver = 1.30f;
}



