#pragma once


#include  "NiFileTool.h"
#include  "ExportNiData.h"



#define  NI_RECORDE_TEMP_FILE  ".Rinion_temp_file.rin.$$$"


#define  NI_FILE_PLAYER_STOP  0
#define  NI_FILE_PLAYER_START 1
#define  NI_FILE_PLAYER_GO    2
#define  NI_FILE_PLAYER_PAUSE 3
#define  NI_FILE_PLAYER_RESET 4



using namespace jbxl;
using namespace jbxwl;




typedef struct _file_dev_parameter 
{
	BOOL	repeat;
	BOOL	calc_quat;
	BOOL	mirroring;
	BOOL	init_pos;
	float   size_scale;
	float	time_scale;
	//
	int		start_frame;
	int*	controler;
	CEdit*	counter;
	//
} FileDevParam;







class  CExNiFileDev : public CNiFileTool
{
public:
	CExNiFileDev(void) { init();}
	CExNiFileDev(NiSDK_Lib lib):CNiFileTool(lib) { init();}

	virtual ~CExNiFileDev(void);


public:
	CParameterSet appParam;

	float  len_scale;
	BOOL   init_pos;
	BOOL   mirroring;

	Vector<float>	vect_fwrd;


public:
	void	init(void);

	CString tempFilePath;
	CExportNiData* niExportData;


public:
	void	setParameter(CParameterSet param) { appParam = param;}

	void	setTempFilePath(void) { tempFilePath = MakeWorkingFolderPath(_T(NI_RECORDE_TEMP_FILE), TRUE, _T("\\NSL\\Rinions\\"), TRUE);}
	void	deleteRecordeTempFile(void) { if (tempFilePath!=_T("")) tunlink(tempFilePath);}
	
	BOOL	openTempJointsFile(void)	    { return open_wfile(tempFilePath, TRUE);}	// 強制上書き
	void	closeTempJointsFile(void)	    { close_wfile();}
	void	writeTempJointsFileHeader(void) { write_header(NULL);}

	BOOL	saveAsJText(LPCTSTR backup, BOOL force);
	BOOL	saveAsBVH  (LPCTSTR backup, BVHSaveParam param);

	int		convertJointsData(CExNiNetwork* net, FileDevParam param);
	void	correctPosVect(Vector<float>* posvect, int frame, int cntrlr);
	void	calcRotQaut(Vector<float>* posvect, Quaternion<float>* rotquat);

	virtual void	Vector2Quaternion(Vector<float>* vt, Quaternion<float>* qt);


// Log
public:
	FILE*	pLogFile;

	BOOL	openLogFile(LPCTSTR path);
	void	closeLogFile(void);
	void	writeLogData(Vector<float>* pos, Quaternion<float>* rot, float* agl, BOOL mrr, NiSDK_Lib lib=NiSDK_Default);;

};
