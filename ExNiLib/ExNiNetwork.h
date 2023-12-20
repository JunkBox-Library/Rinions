#pragma once


#include  "ExNiSHMemory.h"
#include  "ExNiJoints.h"
#include  "LogWndFrame.h"

#include  "tools++.h"
#include  "WinTools.h"
#include  "MFCTool.h"

#include  "anm_data.h"			 // from Animation Server 



#define	  NINET_WM_NETFBPS_UPDATE	JBXWL_WM_USER + 100


#define   NINET_DEFAULT_KBPF		3		// k bits/frame. For smallest size with Kinect SDK 
#define   NINET_DEFAULT_GROUP		"NSL_TEST"
#define   NINET_DEFAULT_SERVER		"silver-star.nsl.tuis.ac.jp"

#define   NINET_UDP_SLPORT			ANM_SERVER_PORT
#define	  NINET_UDP_CLPORT			8100
#define   NINET_UDP_TIMEOUT			5		// sec

#define	  NINET_BUFFER_LEN			1600	// 最低 24(64bit)/48(32bit)個のジョイントデータを転送可能
											// OpenNI:11joints, Kinect SDK:15joints

#define   NINET_FRM_RATE_INTVL		1		// interval of packet check (sec)
#define   NINET_KEEP_ALIVE			3		// min
#define   NINET_REGIST_WAIT_TIME	500		// ms

#define	  NINET_WAIT_TIME			200	



using namespace jbxl;
using namespace jbxwl;





class CExNiNetwork
{
public:
	CExNiNetwork(void);
	virtual ~CExNiNetwork(void);

public:
	CParameterSet appParam;

	char*		m_str_server;
	char*		m_str_groupid;
	char*		m_str_animid;

	BOOL		locked_reciever;

	int			recvSocket;
	int			sendSocket;
	unsigned short portNAPT;
	
	struct sockaddr_in serverDataAddr;

	Buffer		recvBuffer;
	Buffer		sendBuffer;
	char		recvKey[ANM_COM_LEN_IDKEY];

	CExNiSHMemory*	sharedMem;
	CLogWndDoc*	pLogDoc;

	int			keepAliveTime;
	float		recvBps;
	float		frameRate;
	int			packetDelay;

	unsigned long  log_ttl_time;
	unsigned short log_lap_time;


public:
	void		setParameter(CParameterSet param);
	void		setSharedMem(CExNiSHMemory* mem)	  { sharedMem = mem;}

	float		getNetFPS(void) { return frameRate;}
	float		getNetBPS(void) { return recvBps;}
	void		clearNetFBPS(void);

	void		openRecvSocket(void);
	void		openSendSocket(void);

	void		closeNetwork(void);
	void		closeRecvSocket(void);
	void		closeSendSocket(void);

	unsigned char serverLogin(void);
	int			serverRegist(int sock, struct sockaddr_in sv_addr, udp_header* udphd);
	void		serverLogout(void);

	void		printNetInfo(udp_header* udphd);

	void		resetLapTimer(void);
	unsigned long getLapTime(void);


public:
	void		sendAnimationData(Vector<float>* posVect, Quaternion<float>* rotQuat, float* jntAngl, NiSDK_Lib lib, int jnum, BOOL trans);

	void		sendAnimationLogStart(NiSDK_Lib lib, int jnum);
	void		sendAnimationLogStop(void);
};






//////////////////////////////////////////////////////////////////////////////////////////////////////
//
// データ送受信
//

UINT  niNetworkRecieveLoop(LPVOID pParam);



