#include  "stdafx.h"

#include  "ParameterSet.h"
#include  "NiBVHTool.h"

#include  "ExNiLib/ExNiNetwork.h"
#include  "ExNiLib/ExNiSHMemory.h"



using namespace jbxwl;






void  CParameterSet::init(void)
{
	userName		= _T("Rinions_User");
	configFilePath	= makeConfigFilePath();
	configSizePath	= makeConfigSizePath();
	//
#ifdef ENABLE_OPENNI2
	nextSDKLib		= NiSDK_OpenNI2;
#else
	nextSDKLib		= NiSDK_OpenNI;
#endif
	isMirroring		= TRUE;
	isUseImage		= TRUE;
	lineSkeleton	= 1;

	netOutMode		= NETonly;
	netFastMode		= TRUE;
	netLogOnly		= FALSE;
	animationSrvr	= _T(NINET_DEFAULT_SERVER);
	serverPort      = NINET_UDP_SLPORT;
	clientPort		= NINET_UDP_CLPORT;
	groupID			= _T(NINET_DEFAULT_GROUP);
	inAutoBPS		= FALSE;
	inMaxBPS		= 300;	// kbps

	animationUUID   = _T(SHMIF_DEFAULT_ANIM);
	saveDataMode	= FileDataJTXT;
	saveLogLocal	= FALSE;
	saveBVHFormat	= BVH_SAVE_FORMAT_QAV;
	saveBVHFPS		= 30;
	saveDivTime		= 0;
	saveSzScale		= 1.0f;

	saveLogLocal	= FALSE;
	sendLogNet		= FALSE;
	saveNoData		= TRUE;
	saveLogFolder	= _T(".\\Log");

	printPostnMode	= TRUE;
	printRotMxMode	= FALSE;
	printQuateMode	= TRUE;
	printAngleMode	= FALSE;
	printNetwkMode	= FALSE;
	printNtChkMode	= FALSE;

	useDevLED		= FALSE;
	useDevMotor		= FALSE;

	usePosData		= TRUE;
	useRotData		= FALSE;
	useJointConst	= TRUE;

	useFaceDetect	= FALSE;
	enableFaceDetect= FALSE;

	useSpeechReco	= TRUE;
	enableSpeechReco= FALSE;
	confdSpeech		= 0.1f;
	langSpeech		= _T("");

	useNiteSmooth	= FALSE;
	confidence		= 0.70f;
	smoothNITE		= 0.0f;

	useKnctSmooth	= FALSE;
	correction		= 0.5f;
	smoothKNCT		= 0.5f;

	useMvavSmooth	= TRUE;
	mvavType		= MVAV_Expo;
	mvavNum			= 3;

	YaxisCorrect	= 0.0f;
	detectParts		= 2;		// XN_SKEL_PROFILE_ALL;

	//
	outDataPostion	= FALSE;
	outDataQuate	= TRUE;

	outCtrlFPS		= FALSE;
	outAutoFPS		= FALSE;
	outDataFPS		= NI_FPS_MAX;
}




void  CParameterSet::readConfigFile(void)
{
	tList* lt = read_index_tList_file_t(configFilePath, ' ');
	if (lt==NULL) return;

	//
	userName		= get_tstr_param_tList (lt, "userName", (LPCTSTR)userName);
	//
 	nextSDKLib		= (NiSDK_Lib)   get_int_param_tList(lt, "nextSDKLib",  (int)nextSDKLib);

 	netOutMode		= (NiNetOutMode)get_int_param_tList(lt, "netOutMode",  (int)netOutMode);
	saveDataMode 	= (NiFileType)  get_int_param_tList(lt, "saveDataMode",(int)saveDataMode);
	mvavType		= (NiMvAvType)  get_int_param_tList(lt, "mvavType",    (int)mvavType);

	isMirroring		= get_bool_param_tList (lt, "mirroring",	  isMirroring);
	isUseImage	    = get_bool_param_tList (lt, "useImage",		  isUseImage);
	lineSkeleton	= get_int_param_tList  (lt, "lineSkeleton",	  lineSkeleton);

	netFastMode		= get_bool_param_tList (lt, "netFastMode",	  netFastMode);
	netLogOnly		= get_bool_param_tList (lt, "netLogOnly",	  netLogOnly);
	inAutoBPS		= get_bool_param_tList (lt, "inAutoBPS",	  inAutoBPS);

	serverPort		= get_int_param_tList  (lt, "serverPort",	  serverPort);
	clientPort		= get_int_param_tList  (lt, "clientPort",	  clientPort);
	inMaxBPS		= get_int_param_tList  (lt, "inMaxBPS",		  inMaxBPS);

	saveBVHFormat	= get_int_param_tList  (lt, "saveBVHFormat",  saveBVHFormat);
	saveBVHFPS		= get_int_param_tList  (lt, "saveBVHFPS",     saveBVHFPS);
	saveDivTime		= get_int_param_tList  (lt, "saveDivTime",    saveDivTime);
	saveSzScale		= get_float_param_tList(lt, "saveSzScale",    saveSzScale);

	sendLogNet	    = get_bool_param_tList (lt, "sendLogNet",     sendLogNet);
	saveLogLocal	= get_bool_param_tList (lt, "saveLogLocal",   saveLogLocal);
	saveNoData		= get_bool_param_tList (lt, "saveNoData",	  saveNoData);
	saveLogFolder	= get_tstr_param_tList (lt, "saveLogFolder",  (LPCTSTR)saveLogFolder);

	animationUUID	= get_tstr_param_tList (lt, "animationUUID",  (LPCTSTR)animationUUID);
	animationSrvr	= get_tstr_param_tList (lt, "animationSrvr",  (LPCTSTR)animationSrvr);
	groupID			= get_tstr_param_tList (lt, "groupID",		  (LPCTSTR)groupID);

	printPostnMode	= get_bool_param_tList (lt, "printPostnMode", printPostnMode);
	printRotMxMode	= get_bool_param_tList (lt, "printRotMxMode", printRotMxMode);
	printQuateMode	= get_bool_param_tList (lt, "printQuateMode", printQuateMode);
	printAngleMode	= get_bool_param_tList (lt, "printAngleMode", printAngleMode);
	printNetwkMode	= get_bool_param_tList (lt, "printNetwkMode", printNetwkMode);
	printNtChkMode	= get_bool_param_tList (lt, "printNtChkMode", printNtChkMode);

	useDevLED		= get_bool_param_tList (lt, "useDevLED",	  useDevLED);
	useDevMotor		= get_bool_param_tList (lt, "useDevMotor",	  useDevMotor);

	usePosData		= get_bool_param_tList (lt, "usePosData",     usePosData);
	useRotData		= get_bool_param_tList (lt, "useRotData",     useRotData);
	useJointConst	= get_bool_param_tList (lt, "useJointConst",  useJointConst);
	useFaceDetect	= get_bool_param_tList (lt, "useFaceDetect",  useFaceDetect);


	useSpeechReco	= get_bool_param_tList (lt, "useSpeechReco",  useSpeechReco);
	confdSpeech		= get_float_param_tList(lt, "confdSpeech",	  confdSpeech);
	langSpeech		= get_tstr_param_tList (lt, "langSpeech",	  (LPCTSTR)langSpeech);

	useNiteSmooth	= get_bool_param_tList (lt, "useNiteSmooth",  useNiteSmooth);
	useKnctSmooth	= get_bool_param_tList (lt, "useKnctSmooth",  useKnctSmooth);
	useMvavSmooth	= get_bool_param_tList (lt, "useMvavSmooth",  useMvavSmooth);

	confidence		= get_float_param_tList(lt, "confidence",	  confidence);
	smoothNITE		= get_float_param_tList(lt, "smoothNITE",	  smoothNITE);
	correction		= get_float_param_tList(lt, "correction",	  correction);
	smoothKNCT		= get_float_param_tList(lt, "smoothKNCT",	  smoothKNCT);
	YaxisCorrect	= get_float_param_tList(lt, "YaxisCorrect",	  YaxisCorrect);

	mvavNum			= get_int_param_tList  (lt, "mvavNum",		  mvavNum);
	detectParts		= get_int_param_tList  (lt, "detectParts",	  detectParts);

	outDataPostion	= get_bool_param_tList (lt, "outDataPostion", outDataPostion);
	outDataQuate	= get_bool_param_tList (lt, "outDataQuate",	  outDataQuate);
	outCtrlFPS		= get_bool_param_tList (lt, "outCtrlFPS",	  outCtrlFPS);
	outAutoFPS		= get_bool_param_tList (lt, "outAutoFPS",	  outAutoFPS);
	outDataFPS		= get_int_param_tList  (lt, "outDataFPS",	  outDataFPS);

	//
	del_all_tList(&lt);
	return;
}




void  CParameterSet::saveConfigFile(void)
{   
	FILE* fp = tfopen(configFilePath, _T("wb"));
	if (fp==NULL) return;

	/*
	int wx = winsz.right  - winsz.left;
	int wy = winsz.bottom - winsz.top;
	if (winsz.left>0) fprintf(fp, "windowsPosX  %d\n", winsz.left);
	if (winsz.top>0)  fprintf(fp, "windowsPosY  %d\n", winsz.top);
	if (wx>0)		  fprintf(fp, "windowsSizeX %d\n", wx);
	if (wy>0)		  fprintf(fp, "windowsSizeY %d\n", wy);
	*/

	//
	fprintf(fp, "nextSDKLib %d\n", (int)nextSDKLib);

	if (isMirroring)	fprintf(fp, "mirroring %s\n",	"TRUE");
	else				fprintf(fp, "mirroring %s\n",	"FALSE");
	if (isUseImage)		fprintf(fp, "useImage  %s\n",	"TRUE");
	else				fprintf(fp, "useImage  %s\n",	"FALSE");

	if (netFastMode)	fprintf(fp, "netFastMode %s\n", "TRUE");
	else				fprintf(fp, "netFastMode %s\n", "FALSE");
	if (netLogOnly)		fprintf(fp, "netLogOnly %s\n",  "TRUE");
	else				fprintf(fp, "netLogOnly %s\n",  "FALSE");
	if (inAutoBPS)		fprintf(fp, "inAutoBPS %s\n",	"TRUE");
	else				fprintf(fp, "inAutoBPS %s\n",	"FALSE");

	// CString
	Buffer tmp = make_Buffer(LNAME);
	copy_ts2Buffer(userName, &tmp);
	fprintf(fp, "userName %s\n",      (char*)tmp.buf);
	copy_ts2Buffer(langSpeech, &tmp);
	fprintf(fp, "langSpeech %s\n",    (char*)tmp.buf);
	copy_ts2Buffer(animationUUID, &tmp);
	fprintf(fp, "animationUUID %s\n", (char*)tmp.buf);
	copy_ts2Buffer(animationSrvr, &tmp);
	fprintf(fp, "animationSrvr %s\n", (char*)tmp.buf);
	copy_ts2Buffer(groupID, &tmp);
	fprintf(fp, "groupID %s\n",		  (char*)tmp.buf);
	copy_ts2Buffer(saveLogFolder, &tmp);
	fprintf(fp, "saveLogFolder %s\n", (char*)tmp.buf);
	free_Buffer(&tmp);
	/*
	ftprintf(fp, _T("userName %s\n"),      (LPCTSTR)userName);
	ftprintf(fp, _T("animationUUID %s\n"), (LPCTSTR)animationUUID);
	ftprintf(fp, _T("animationSrvr %s\n"), (LPCTSTR)animationSrvr);
	ftprintf(fp, _T("groupID %s\n"),       (LPCTSTR)groupID);
	ftprintf(fp, _T("saveLogFolder %s\n"), (LPCTSTR)saveLogFolder);
	*/

	fprintf(fp, "lineSkeleton %d\n",  lineSkeleton);
	fprintf(fp, "netOutMode %d\n",	  (int)netOutMode);
	fprintf(fp, "serverPort %d\n",	  serverPort);
	fprintf(fp, "clientPort %d\n",	  clientPort);
	fprintf(fp, "inMaxBPS %d\n",	  inMaxBPS);

	fprintf(fp, "saveDataMode %d\n",  (int)saveDataMode);
	fprintf(fp, "saveBVHFormat %d\n", saveBVHFormat);
	fprintf(fp, "saveBVHFPS  %d\n",   saveBVHFPS);
	fprintf(fp, "saveDivTime %d\n",   saveDivTime);
	fprintf(fp, "saveSzScale %f\n",   saveSzScale);

	if (saveLogLocal)	fprintf(fp, "saveLogLocal %s\n",   "TRUE");
	else				fprintf(fp, "saveLogLocal %s\n",   "FALSE");
	if (sendLogNet)		fprintf(fp, "sendLogNet %s\n",     "TRUE");
	else				fprintf(fp, "sendLogNet %s\n",     "FALSE");
	if (saveNoData)	fprintf(fp, "saveNoData %s\n", "TRUE");
	else				fprintf(fp, "saveNoData %s\n", "FALSE");

	//
	if (printPostnMode)	fprintf(fp, "printPostnMode %s\n", "TRUE");
	else				fprintf(fp, "printPostnMode %s\n", "FALSE");
	if (printRotMxMode)	fprintf(fp, "printRotMxMode %s\n", "TRUE");
	else				fprintf(fp, "printRotMxMode %s\n", "FALSE");
	if (printQuateMode)	fprintf(fp, "printQuateMode %s\n", "TRUE");
	else				fprintf(fp, "printQuateMode %s\n", "FALSE");
	if (printAngleMode)	fprintf(fp, "printAngleMode %s\n", "TRUE");
	else				fprintf(fp, "printAngleMode %s\n", "FALSE");
	if (printNetwkMode)	fprintf(fp, "printNetwkMode %s\n", "TRUE");
	else				fprintf(fp, "printNetwkMode %s\n", "FALSE");
	if (printNtChkMode)	fprintf(fp, "printNtChkMode %s\n", "TRUE");
	else				fprintf(fp, "printNtChkMode %s\n", "FALSE");

	if (useDevLED)		fprintf(fp, "useDevLED %s\n",      "TRUE");
	else				fprintf(fp, "useDevLED %s\n",      "FALSE");
	if (useDevMotor)	fprintf(fp, "useDevMotor %s\n",    "TRUE");
	else				fprintf(fp, "useDevMotor %s\n",    "FALSE");

	if (usePosData)		fprintf(fp, "usePosData %s\n",	   "TRUE");
	else				fprintf(fp, "usePosData %s\n",	   "FALSE");
	if (useRotData)		fprintf(fp, "useRotData %s\n",	   "TRUE");
	else				fprintf(fp, "useRotData %s\n",	   "FALSE");

	if (useJointConst)	fprintf(fp, "useJointConst %s\n",  "TRUE");
	else				fprintf(fp, "useJointConst %s\n",  "FALSE");
	if (useFaceDetect)	fprintf(fp, "useFaceDetect %s\n",  "TRUE");
	else				fprintf(fp, "useFaceDetect %s\n",  "FALSE");

	if (useSpeechReco)	fprintf(fp, "useSpeechReco %s\n",  "TRUE");
	else				fprintf(fp, "useSpeechReco %s\n",  "FALSE");
	
	if (useNiteSmooth)	fprintf(fp, "useNiteSmooth %s\n",  "TRUE");
	else				fprintf(fp, "useNiteSmooth %s\n",  "FALSE");
	if (useKnctSmooth)	fprintf(fp, "useKnctSmooth %s\n",  "TRUE");
	else				fprintf(fp, "useKnctSmooth %s\n",  "FALSE");
	if (useMvavSmooth)	fprintf(fp, "useMvavSmooth %s\n",  "TRUE");
	else				fprintf(fp, "useMvavSmooth %s\n",  "FALSE");

	//
	fprintf(fp, "confdSpeech  %f\n", confdSpeech);
	fprintf(fp, "confidence   %f\n", confidence);
	fprintf(fp, "smoothNITE   %f\n", smoothNITE);
	fprintf(fp, "correction   %f\n", correction);
	fprintf(fp, "smoothKNCT   %f\n", smoothKNCT);
	fprintf(fp, "YaxisCorrect %f\n", YaxisCorrect);

	fprintf(fp, "mvavType %d\n",	(int)mvavType);
	fprintf(fp, "mvavNum %d\n",		(int)mvavNum);
	fprintf(fp, "detectParts %d\n", (int)detectParts);

	//
	if (outDataPostion)	fprintf(fp, "outDataPostion %s\n", "TRUE");
	else				fprintf(fp, "outDataPostion %s\n", "FALSE");
	if (outDataQuate)	fprintf(fp, "outDataQuate %s\n",   "TRUE");
	else				fprintf(fp, "outDataQuate %s\n",   "FALSE");
	if (outCtrlFPS)		fprintf(fp, "outCtrlFPS %s\n",	   "TRUE");
	else				fprintf(fp, "outCtrlFPS %s\n",	   "FALSE");
	if (outAutoFPS)		fprintf(fp, "outAutoFPS %s\n",	   "TRUE");
	else				fprintf(fp, "outAutoFPS %s\n",	   "FALSE");
	fprintf(fp, "outDataFPS %d\n", outDataFPS);

	//
	fflush(fp);
	fclose(fp);
	return;
}




void  CParameterSet::readWindowSize(RECT* winsz)
{   
	if (winsz==NULL) return;

	tList* lt = read_index_tList_file_t(configSizePath, ' ');
	if (lt==NULL) return;

	//	
	winsz->left   = (LONG)get_int_param_tList(lt, "windowsPosX",  (int)winsz->left);
	winsz->top	  = (LONG)get_int_param_tList(lt, "windowsPosY",  (int)winsz->top);
	winsz->right  = (LONG)get_int_param_tList(lt, "windowsSizeX", (int)winsz->right);
	winsz->bottom = (LONG)get_int_param_tList(lt, "windowsSizeY", (int)winsz->bottom);

	del_all_tList(&lt);
	return;
}




void  CParameterSet::saveWindowSize(RECT winsz)
{   
	FILE* fp = tfopen(configSizePath, _T("wb"));
	if (fp==NULL) return;

	//
	int wx = winsz.right  - winsz.left;
	int wy = winsz.bottom - winsz.top;
	if (winsz.left>0) fprintf(fp, "windowsPosX  %d\n", winsz.left);
	if (winsz.top>0)  fprintf(fp, "windowsPosY  %d\n", winsz.top);
	if (wx>0)		  fprintf(fp, "windowsSizeX %d\n", wx);
	if (wy>0)		  fprintf(fp, "windowsSizeY %d\n", wy);

	fflush(fp);
	fclose(fp);
	return;
}




CString  CParameterSet::makeConfigFilePath(void)
{
	// Roming
	CString configf = MakeWorkingFolderPath(_T(RINIONS_CONFIG_FILE), FALSE, _T(RINIONS_CONFIG_PATH), TRUE);

	return configf;
}



CString  CParameterSet::makeConfigSizePath(void)
{
	// Roming
	CString configf = MakeWorkingFolderPath(_T(RINIONS_WINSIZE_FILE), FALSE, _T(RINIONS_CONFIG_PATH), TRUE);

	return configf;
}
