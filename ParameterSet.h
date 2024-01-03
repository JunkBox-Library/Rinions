#pragma once


#include  "Rinions_config.h"
#include  "NiToolWin.h"


using namespace  jbxwl;




enum  NiNetOutMode
{
	NETandLOCAL,
	NETonly 
};



enum  NiFileType
{
	FileDataUnknown,
	FileDataNone,
	FileDataONI,
	FileDataBVH,
	FileDataJTXT,
	FileDataVMD2
};



enum  NiMvAvType 
{
	MVAV_None,
	MVAV_Simple,
	MVAV_Weight,
	MVAV_Expo
};



enum  NiInputDevice 
{
	NiDevice_None,
	NiDevice_NI,
	NiDevice_File
};





////////////////////////////////////////////////////////////////////////////////////
//
// CParameterSet
//

class CParameterSet
{
public:
	CParameterSet() { init();}
	virtual	~CParameterSet() {}

	void		init(void);

	void		readConfigFile(void);
	void		saveConfigFile(void);

	void		readWindowSize(RECT* winsz);
	void		saveWindowSize(RECT  winsz);

	CString		makeConfigFilePath(void);
	CString		makeConfigSizePath(void);


public:
	CString		 userName;
	CString		 configFilePath; 
	CString		 configSizePath; 

	// from RINIONS_CONFIG_FILE
	NiSDK_Lib	 nextSDKLib;

	BOOL		 isMirroring;		// ミラーリングを行う．
	BOOL		 isUseImage;		// カメライメージを使用する．
	int			 lineSkeleton;

	NiNetOutMode netOutMode;
	BOOL		 netFastMode;
	BOOL		 netLogOnly;
	CString		 animationSrvr;
	int			 serverPort;
	int			 clientPort;
	CString		 groupID;
	BOOL		 inAutoBPS;
	int			 inMaxBPS;

	CString		 animationUUID;

	NiFileType   saveDataMode;
	BOOL		 saveNoData;
	int			 saveBVHFormat;
	int			 saveBVHFPS;
	int			 saveDivTime;
	double		 saveSzScale;

	BOOL		 saveLogLocal;
	BOOL		 sendLogNet;
	CString		 saveLogFolder;

	BOOL		 printPostnMode;
	BOOL		 printRotMxMode;
	BOOL		 printQuateMode;
	BOOL		 printAngleMode;
	BOOL		 printNetwkMode;
	BOOL		 printNtChkMode;

	BOOL		 useDevLED;
	BOOL		 useDevMotor;

	BOOL		 usePosData;
	BOOL		 useRotData;
	BOOL		 useJointConst;


	// Face
	BOOL		 useFaceDetect;
	BOOL		 enableFaceDetect;

	// Speech
	BOOL		 useSpeechReco;
	BOOL		 enableSpeechReco;
	double		 confdSpeech;
	CString		 langSpeech;

	// Smooth
	BOOL		 useNiteSmooth;
	double		 confidence;
	double		 smoothNITE;

	BOOL		 useKnctSmooth;
	double		 correction;
	double		 smoothKNCT;

	BOOL		 useMvavSmooth;
	NiMvAvType	 mvavType;
	int			 mvavNum;

	//
	double		 YaxisCorrect;
	int			 detectParts;

	//
	BOOL		 outDataPostion;
	BOOL		 outDataQuate;

	BOOL		 outCtrlFPS;
	BOOL		 outAutoFPS;
	int			 outDataFPS;
};



