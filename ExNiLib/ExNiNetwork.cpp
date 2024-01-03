
#include  "stdafx.h"

#include  "ExNiNetwork.h"
#include  "buffer.h"
#include  "network.h"



/*
使用するパラメータ
	appParam.clientPort			CSetNetwork
	appParam.animationUUID		CSetAnimation
	appParam.printNetwkMode		CSetLogMode
	appParam.printNtChkMode		CSetLogMode
*/




CExNiNetwork::CExNiNetwork(void)
{
	recvSocket		= 0;
	sendSocket		= 0;
	portNAPT		= 0;

	m_str_server	= NULL;
	m_str_groupid	= NULL;
	m_str_animid	= NULL;

	sharedMem		= NULL;
	pLogDoc			= NULL;
	locked_reciever = FALSE;;

	recvBps  		= 0.0f;
	frameRate		= 0.0f;
	packetDelay		= 0;

	appParam.init();
	keepAliveTime = NINET_KEEP_ALIVE;

	memset(recvKey, 0, ANM_COM_LEN_IDKEY);
	memset(&serverDataAddr, 0, sizeof(struct sockaddr_in));

	recvBuffer = make_Buffer(NINET_BUFFER_LEN);
	sendBuffer = make_Buffer(NINET_BUFFER_LEN);

	// Networkの初期化 see network.c
	init_network();
}



CExNiNetwork::~CExNiNetwork(void)
{
	DEBUG_INFO("DESTRUCTOR: CExNiNetwork");

	if (recvKey[0]!='\0') serverLogout();

	closeRecvSocket();
	closeSendSocket();

	while (locked_reciever) {
		::Sleep(NINET_WAIT_TIME);
		DisPatcher();
	}

	free_Buffer(&recvBuffer);
	free_Buffer(&sendBuffer);

	//
	if (m_str_server!=NULL) {
		::free(m_str_server);
		m_str_server = NULL;
	}
	if (m_str_groupid!=NULL) {
		::free(m_str_groupid);
		m_str_groupid = NULL;
	}
	if (m_str_animid!=NULL) {
		::free(m_str_animid);
		m_str_animid = NULL;
	}
}



void  CExNiNetwork::clearNetFBPS(void)
{
	recvBps   = 0.0f;
	frameRate = 0.0f;
	SendWinMessage(NINET_WM_NETFBPS_UPDATE);
}



void  CExNiNetwork::setParameter(CParameterSet param)
{ 
	appParam = param;

	if (appParam.animationSrvr!=_T("")) {
		if (m_str_server!=NULL) ::free(m_str_server);
		m_str_server = ts2mbs(appParam.animationSrvr);
	}
	if (appParam.groupID!=_T("")) {
		if (m_str_groupid!=NULL) ::free(m_str_groupid);
		m_str_groupid = ts2mbs(appParam.groupID);
	}
	if (appParam.animationUUID!=_T("")) {
		if (m_str_animid!=NULL) ::free(m_str_animid);
		m_str_animid = ts2mbs(appParam.animationUUID);
	}
}





////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//

unsigned char  CExNiNetwork::serverLogin(void)
{
	int    cc;
	struct sockaddr_in sv_addr;

	memset(&serverDataAddr, 0, sizeof(struct sockaddr_in));
	memset(recvKey, 0, ANM_COM_LEN_IDKEY);
	portNAPT = 0;

	udp_header udphd;
	memset(&udphd, 0, sizeof(udp_header));
	udphd.com[0] = ANM_COM_REQ_LOGIN;
	udphd.com[1] = ANM_COM_REPLY_ERROR;
	memcpy(udphd.key, m_str_groupid, ANM_COM_LEN_IDKEY);

	//
	int sock = udp_client_socket(m_str_server, appParam.serverPort, &sv_addr);

	sock = udp_bind(sock, appParam.clientPort);
	if (sock<=0) return ANM_COM_ERROR_SOCKET;

	cc = udp_send(sock, (char*)&udphd, sizeof(udp_header), &sv_addr);
	if (cc<=0) {
		socket_close(sock);
		return ANM_COM_REPLY_ERROR;
	}
	//
	cc = recv_wait(sock, NINET_UDP_TIMEOUT);
	if (!cc) {
		socket_close(sock);
		return ANM_COM_ERROR_SERVER_DOWN;
	}
	//
	cc = udp_recv(sock, (char*)&udphd, sizeof(udp_header), &sv_addr);
	if (cc<=0 || udphd.com[1]!=ANM_COM_REPLY_OK) {
		if (cc<0) udphd.com[1] = ANM_COM_ERROR_SERVER_DOWN;
		socket_close(sock);
		return udphd.com[1];
	}

	::Sleep(NINET_REGIST_WAIT_TIME);	// wait server

	serverDataAddr = sv_addr;
	serverDataAddr.sin_port = udphd.port;
	memcpy(recvKey, udphd.key, ANM_COM_LEN_IDKEY);

	cc = serverRegist(sock, serverDataAddr, &udphd);	
	if (cc<0) serverLogout();

	socket_close(sock);

	portNAPT = ntohs(udphd.port);
	return udphd.com[1];
}




int  CExNiNetwork::serverRegist(int sock, struct sockaddr_in sv_addr, udp_header* udphd)
{
	udphd->com[0] = ANM_COM_REQ_REGIST;
	udphd->com[1] = ANM_COM_REPLY_ERROR;
	udphd->port = 0;
	memcpy(udphd->uuid, m_str_animid, ANM_COM_LEN_UUID);

	int cc = udp_send(sock, (char*)udphd, sizeof(udp_header), &sv_addr);
	if (cc<=0) {
		socket_close(sock);
		udphd->com[1] = ANM_COM_REPLY_ERROR;
		return 1;
	}

	cc = recv_wait(sock, NINET_UDP_TIMEOUT);	
	if (!cc) {
		socket_close(sock);
		udphd->com[1] = ANM_COM_ERROR_TIMEOUT;
		return 2;
	}

	cc = udp_recv(sock, (char*)udphd, sizeof(udp_header), &sv_addr);
	if (cc<0) return cc;

	return 0;
}




void  CExNiNetwork::serverLogout(void)
{
	udp_header udphd;
	if (serverDataAddr.sin_port==0) return;

	memset(&udphd, 0, sizeof(udp_header));
	udphd.port = htons(portNAPT);
	memcpy(udphd.key, recvKey, ANM_COM_LEN_IDKEY);
	memcpy(udphd.uuid, m_str_animid, ANM_COM_LEN_UUID);

	int sock = (int)socket(AF_INET, SOCK_DGRAM, 0);

	//
	udphd.com[0] = ANM_COM_REQ_DELETE;
	int cc = udp_send(sock, (char*)&udphd, sizeof(udp_header), &serverDataAddr);
	//
	udphd.com[0] = ANM_COM_REQ_LOGOUT;
	cc = udp_send(sock, (char*)&udphd, sizeof(udp_header), &serverDataAddr);
	
	socket_close(sock);

	closeNetwork();
}




void  CExNiNetwork::closeNetwork(void)
{
	closeSendSocket();
	closeRecvSocket();

	if (sharedMem!=NULL) {
		sharedMem->clearNetworkAnimationIndex();
		sharedMem->clearNetworkAnimationData();
	}

	memset(&serverDataAddr, 0, sizeof(struct sockaddr_in));
	memset(recvKey, 0, ANM_COM_LEN_IDKEY);
}





void  CExNiNetwork::printNetInfo(udp_header* udphd)
{
	if (pLogDoc==NULL || udphd==NULL) return;
	pLogDoc->lock();

	if (appParam.printNetwkMode) {
		pLogDoc->printFormat("NET: %s  (%2d) [%d]\n", udphd->uuid, ntohs(udphd->num), ntohs(udphd->tsz));
	}

	if (appParam.printNtChkMode && (frameRate>0.0f || packetDelay>0)) {
		int n = sharedMem->getNetworkAnimationIndexNum();
		pLogDoc->printFormat("NET: CHECK: %4.1ffps/%4.1fkbps %4dms (%1d) [%s]\n", frameRate, recvBps, packetDelay, n, udphd->key);
	}

	pLogDoc->unlock();

	return;
}





void  CExNiNetwork::openRecvSocket(void)
{
	recvSocket = udp_server_socket((int)appParam.clientPort);
	return;
}




void  CExNiNetwork::openSendSocket(void)
{
	sendSocket = (int)socket(AF_INET, SOCK_DGRAM, 0);
	return;
}




void  CExNiNetwork::closeRecvSocket(void)
{
	if (recvSocket>0) {
		socket_close(recvSocket);
		recvSocket = 0;
	}
}




void  CExNiNetwork::closeSendSocket(void)
{
	if (sendSocket>0) {
		socket_close(sendSocket);
		sendSocket = 0;
	}
}




void  CExNiNetwork::resetLapTimer(void)
{
	log_ttl_time = 0;
	log_lap_time = 0;
}



unsigned long int  CExNiNetwork::getLapTime(void)
{
	unsigned short ctime;
	log_ttl_time += GetMsecondsLapTimer(log_lap_time, &ctime);
	log_lap_time  = ctime;

	return log_ttl_time;
}



//
// Network Log
//
void  CExNiNetwork::sendAnimationLogStart(NiSDK_Lib lib, int joints_num)
{
	clear_Buffer(&sendBuffer);
	udp_header* udphd = (udp_header*)sendBuffer.buf;
	char* datptr = (char*)(sendBuffer.buf+sizeof(udp_header));
	
	unsigned short* mode = NULL;
	int num = 0;
	int len = 0;

	//
	// User Name
	mode = (unsigned short*)datptr;
	mode[0] = ANM_COM_DATA_NAME;
	//
	Buffer buf = make_Buffer(LNAME);
	copy_ts2Buffer(appParam.userName, &buf);
	memcpy(datptr+4, buf.buf, ANM_COM_LEN_NAME+1);	// +1 -> '\0'
	free_Buffer(&buf);
	//
	int sz = ANM_COM_LEN_NAME + 5;		// + 1 + 4
	datptr += sz;
	len += sz;
	mode[1] = (unsigned short)sz;	// データ長
	num++;

	/*
	// Position Title
	mode = (unsigned short*)datptr;
	mode[0] = ANM_COM_DATA_TITLE;
	sz = (int)strlen(NI_LOG_POS_TITLE) + 1;
	memcpy(datptr+4, NI_LOG_POS_TITLE, sz);
	//
	sz += 4;
	datptr += sz;
	len += sz;
	mode[1] = (unsigned short)sz;	// データ長
	num++;

	//
	// Joint Name
	for (int j=0; j<joints_num; j++) {
		//
		n = Ni2SDKRotJointNum(j, lib);
		if (n>=0) {
			std::string name = NiSDK2JointName(n, lib);		
			mode = (unsigned short*)datptr;
			mode[0] = ANM_COM_DATA_TITLE;
			sz = (int)strlen(name.c_str()) + 1;
			memcpy(datptr+4, name.c_str(), sz);
			//
			sz += 4;
			datptr += sz;
			len += sz;
			mode[1] = (unsigned short)sz;	// データ長
			num++;
		}
	}
	*/


	//
	if (num>0) {
		udphd->com[0] = ANM_COM_REQ_LOG_START;
		memcpy(udphd->key,  recvKey,      ANM_COM_LEN_IDKEY);
		memcpy(udphd->uuid, m_str_animid, ANM_COM_LEN_UUID);

		udphd->msec = htons(GetMsecondsTimer());
		udphd->port = htons(portNAPT);
		udphd->num  = htons((unsigned short)num);
//		udphd->sz   = htons((unsigned short)4);

		sendBuffer.vldsz = sizeof(udp_header) + len;
		udp_send_Buffer(sendSocket, &sendBuffer, &serverDataAddr);
	}
}






void  CExNiNetwork::sendAnimationLogStop(void)
{
	clear_Buffer(&sendBuffer);
	udp_header* udphd = (udp_header*)sendBuffer.buf;
	char* datptr = (char*)(sendBuffer.buf+sizeof(udp_header));
	
	unsigned short* mode = NULL;
	int len;


	// User Name
	mode = (unsigned short*)datptr;
	mode[0] = ANM_COM_DATA_NAME;
	//
	Buffer buf = make_Buffer(LNAME);
	copy_ts2Buffer(appParam.userName, &buf);
	memcpy(datptr+4, buf.buf, ANM_COM_LEN_NAME+1);	// +1 -> '\0'
	free_Buffer(&buf);
	//
	len = ANM_COM_LEN_NAME + 5;		// + 1 + 4
	mode[1] = (unsigned short)len;	// データ長


	//
	udphd->com[0] = ANM_COM_REQ_LOG_STOP;
	memcpy(udphd->key,  recvKey,      ANM_COM_LEN_IDKEY);
	memcpy(udphd->uuid, m_str_animid, ANM_COM_LEN_UUID);

	udphd->msec = htons(GetMsecondsTimer());
	udphd->port = htons(portNAPT);
	udphd->num  = htons((unsigned short)1);
//	udphd->sz   = htons((unsigned short)4);

	sendBuffer.vldsz = sizeof(udp_header) + len;
	udp_send_Buffer(sendSocket, &sendBuffer, &serverDataAddr);
}






void  CExNiNetwork::sendAnimationData(Vector<float>* posVect, Quaternion<float>* rotQuat, float* jntAngl, NiSDK_Lib lib, int jnum, BOOL trans)
{
	clear_Buffer(&sendBuffer);
	udp_header* udphd = (udp_header*)sendBuffer.buf;
	float* datptr = (float*)(sendBuffer.buf+sizeof(udp_header));
	
	unsigned short* mode = NULL;
	int num = 0;
	int len = 0;

	//
	unsigned short state = 0;
	if (appParam.detectParts==NI_PROFILE_UPPER) {
		state |= NI_SDK_PROFILE_UPPER;
	}
	if (sharedMem->isTracking) {
		state |= NI_SDK_IS_TRACKING;
	}
	//
	if (lib==NiSDK_None) {					// from Data File. 
		state |= NI_SDK_AVATAR_TRAILER;		// has other information (trailer data) of avatar
	}										// be processed by ProgcenslExternalAnimation.cpp

	//
	for (int j=0; j<jnum; j++) {
		//
		mode = (unsigned short*)datptr;
		memset(mode, 0, sizeof(unsigned short)*2);

		// Position
		if (appParam.outDataPostion) {
			int n = NiSDK2SHMPosJointNum(j, lib);
			if (n>=0 && posVect[j].c>=0.0) {
				mode[0] = state | ANM_COM_DATA_POSITION;
				mode[1] = n;
				datptr[1] = posVect[j].x;
				datptr[2] = posVect[j].y;
				datptr[3] = posVect[j].z;
				datptr += 3;
				len += 3;
			}
		}
		else if (j==0 && appParam.outDataQuate) {
			int n = NiSDK2SHMPosJointNum(0, lib);
			if (n>=0 && posVect[0].c>=0.0) {
				mode[0] = state | ANM_COM_DATA_POSITION;
				mode[1] = n;
				datptr[1] = posVect[0].x;
				datptr[2] = posVect[0].y;
				datptr[3] = posVect[0].z;
				datptr += 3;
				len += 3;
			}
		}

		// Rotation
		if (appParam.outDataQuate) {
			int n = NiSDK2SHMRotJointNum(j, lib);
			if (n>=0 && rotQuat[j].c>=0.0) {
				mode[0]|= state | ANM_COM_DATA_ROTATION;
				mode[1] = n;	// if (mode[1]==0) mode[1] = n;
				datptr[1] = rotQuat[j].x;
				datptr[2] = rotQuat[j].y;
				datptr[3] = rotQuat[j].z;
				datptr[4] = rotQuat[j].s;
				datptr += 4;
				len += 4;
			}
		}

		if ((mode[0]&0x00ff)!=0x0000) {
			num++;
			datptr++;
			len++;
			//DEBUG_INFO("NET = %d: %s", j, NiSDK2JointName(j, NiSDK_None).c_str());
		}
	}
	//
	unsigned short jdatasz = (unsigned short)(sizeof(udp_header) + len*sizeof(float));

	//
	if (jntAngl==NULL && trans) {
		if (num>0) {
			udphd->com[0] = ANM_COM_REQ_TRANSFER;

			memcpy(udphd->key,  recvKey,      ANM_COM_LEN_IDKEY);
			memcpy(udphd->uuid, m_str_animid, ANM_COM_LEN_UUID);

			udphd->msec = htons(GetMsecondsTimer());
			udphd->port = htons(portNAPT);
			udphd->num  = htons((unsigned short)num);
			udphd->tsz  = htons(jdatasz);
	
			sendBuffer.vldsz = (int)jdatasz;
			udp_send_Buffer(sendSocket, &sendBuffer, &serverDataAddr);
		}
		return;
	}


	////////////////////////////
	//
	// + Log Data
	//
	int n, sz;

	datptr = (float*)(sendBuffer.buf+jdatasz);

	// User Name
	mode = (unsigned short*)datptr;
	mode[0] = ANM_COM_DATA_NAME;
	//
	Buffer buf = make_Buffer(LNAME);
	copy_ts2Buffer(appParam.userName, &buf);
	memcpy(&datptr[1], buf.buf, ANM_COM_LEN_NAME+1);	// +1 -> '\0'
	free_Buffer(&buf);
	//
	sz = (ANM_COM_LEN_NAME+sizeof(float))/sizeof(float) + 1;
	datptr += sz;
	len += sz;
	mode[1] = (unsigned short)(sz*sizeof(float));	// データ長
	num++;

	// Lap Time
	unsigned long int laptime = getLapTime();
	mode = (unsigned short*)datptr;
	mode[0] = ANM_COM_DATA_LAPTIME;
	*((unsigned long int*)(&datptr[1])) = laptime;
	sz = (sizeof(unsigned long int)+sizeof(float)-1)/sizeof(float) + 1;
	datptr += sz;
	len += sz;
	mode[1] = (unsigned short)(sz*sizeof(float));	// データ長
	num++;

	// Angle between Joints
	for (int j=0; j<jnum; j++) {
		n = NiSDK2SHMRotJointNum(j, lib);
		if (n>=0 && rotQuat[j].c>=0.0) {
			mode = (unsigned short*)datptr;
			mode[0] = ANM_COM_DATA_ANGLE;
			mode[1] = n;
			datptr[1] = jntAngl[j];
			datptr += 2;
			len += 2;
			num++;
		}
	}

	//
	if (num>0) {
		if (trans) udphd->com[0] = ANM_COM_REQ_TRANSFER_LOG;
		else	   udphd->com[0] = ANM_COM_REQ_LOG;

		memcpy(udphd->key,  recvKey,      ANM_COM_LEN_IDKEY);
		memcpy(udphd->uuid, m_str_animid, ANM_COM_LEN_UUID);

		udphd->msec = htons(GetMsecondsTimer());
		udphd->port = htons(portNAPT);
		udphd->num  = htons((unsigned short)num);
		udphd->tsz  = htons(jdatasz);

		sendBuffer.vldsz = sizeof(udp_header) + len*sizeof(float);
		udp_send_Buffer(sendSocket, &sendBuffer, &serverDataAddr);
	}

	return;
}




////////////////////////////////////////////////////////////////////////////////////////
//
// スレッド
//

//
// データ受信
//

UINT  niNetworkRecieveLoop(LPVOID pParam)
{
	if (pParam==NULL) return 1;
	CExNiNetwork* niNetwork = (CExNiNetwork*)pParam;
	if (niNetwork->recvSocket<=0 || niNetwork->sharedMem==NULL) return 1;	

	struct sockaddr_in ds_addr;
	int num = 0;
	int rsz = 0;
	unsigned short tmptime;
	unsigned short frmtime = GetMsecondsTimer();
	
	CParameterSet* appParam = &(niNetwork->appParam);
	int maxbps = appParam->inMaxBPS;
	int kbpf   = NINET_DEFAULT_KBPF;		// k bits/frame

	//
	niNetwork->locked_reciever = TRUE;

	// Keep Alive and Hole Punching
	udp_header alive;
	memset(&alive, 0, sizeof(udp_header));
	alive.com[0] = ANM_COM_REQ_ALIVE;
	alive.port   = htons(niNetwork->portNAPT);
	alive.tsz    = (unsigned short)(maxbps/kbpf);		// k bit/frame
	memcpy(alive.key,  niNetwork->recvKey,      ANM_COM_LEN_IDKEY);
	memcpy(alive.uuid, niNetwork->m_str_animid, ANM_COM_LEN_UUID);

	time_t nowtime;
	time_t alvtime = time(NULL);
	udp_send(niNetwork->recvSocket, (char*)&alive, sizeof(udp_header), &niNetwork->serverDataAddr);	


	//
	while (niNetwork->recvSocket>0)
	{
		int cc = recv_wait(niNetwork->recvSocket, NINET_UDP_TIMEOUT);
		
		if (cc) {
			if (niNetwork->recvSocket<=0) break;
			cc = udp_recv_Buffer(niNetwork->recvSocket, &(niNetwork->recvBuffer), &ds_addr);
			if (cc>0) {
				char* ptr = (char*)niNetwork->recvBuffer.buf;
				udp_header* udphd = (udp_header*)ptr;
				//DEBUG_INFO("RECEIVE COM = %02x", udphd->com[0]);

				if (!strncasecmp((char*)udphd->key, niNetwork->recvKey, ANM_COM_LEN_IDKEY)) {
					//
					int joints = (int)ntohs(udphd->num);
					niNetwork->recvBps     = 0.0f;
					niNetwork->frameRate   = 0.0f;
					niNetwork->packetDelay = 0;
					
					// FPS & BPS
					rsz+= niNetwork->recvBuffer.vldsz;
					num++;
					float laptm = GetMsecondsLapTimer(frmtime, &tmptime)/1000.0f;
					if ((int)laptm>=NINET_FRM_RATE_INTVL) {
						niNetwork->recvBps   = rsz*8/1000.0f/laptm;
						niNetwork->frameRate = num/laptm;
						kbpf = (int)(rsz*8/1000.0f/num);
						frmtime = tmptime;
						num = rsz = 0;
						SendWinMessage(NINET_WM_NETFBPS_UPDATE);

						if (!strncasecmp(niNetwork->m_str_animid, udphd->uuid, ANM_COM_LEN_UUID)) {
							unsigned short tm = ntohs(udphd->msec);
							niNetwork->packetDelay = (int)GetMsecondsLapTimer(tm);
						}
					}
	
					//
					if (udphd->com[0]==ANM_COM_REQ_DELETE) {
						niNetwork->sharedMem->clearNetworkAnimation(udphd->uuid);
						
						if (!strncasecmp(niNetwork->m_str_animid, udphd->uuid, ANM_COM_LEN_UUID)) {
							SendWinMessage(JBXWL_WM_NETWORK_CLOSE);
							break;
						}
					}
					// 共有メモリに書き込み
					else {
						ptr += sizeof(udp_header);
						niNetwork->sharedMem->updateNetworkAnimation(ptr, udphd->uuid, joints);
					}
				
					// Print
					if (niNetwork->pLogDoc!=NULL) niNetwork->printNetInfo(udphd);
				}
			}
		}
		
		// Time Out
		else {
			niNetwork->clearNetFBPS();
		}

		// Keep Alive, QOS Request and UDP Hole Punching
		nowtime = time(NULL);
		if (nowtime-alvtime>niNetwork->keepAliveTime*60 || maxbps!=appParam->inMaxBPS) {
			alvtime = nowtime;
			maxbps  = appParam->inMaxBPS;

			// Band width request
			if (kbpf==0) kbpf = NINET_DEFAULT_KBPF;
			alive.tsz = (unsigned short)(maxbps/kbpf);		// k bit/frame
			//
			udp_send(niNetwork->recvSocket, (char*)&alive, sizeof(udp_header), &niNetwork->serverDataAddr);
		}
	}

	niNetwork->clearNetFBPS();
	niNetwork->locked_reciever = FALSE;
	return 0;
}



