//*****************************************************//
//                                                     //
//      ������: wanyi                                  //
//      ��������: 2010-11-05                           //
//                                                     //
//*****************************************************//

//==============================================================================
// �汾��ʷ
// 2010-11-05 wanyi. TCP����socket�Ļ���
//==============================================================================

#ifndef SocketBaseH
#define SocketBaseH

#include <Winsock2.h>

#define _WINSOCKAPI_								//���ǲ���Ҫ�ɵ�winsock.h
#define SOB_IP_LENGTH				128             //IP�ַ�������
#define SOB_DEFAULT_TIMEOUT_SEC		5				//Ĭ�ϵĳ�ʱʱ��

#define SOB_RET_OK					1				//����
#define SOB_RET_FAIL				0				//����
#define SOB_RET_TIMEOUT				2				//��ʱ

typedef unsigned (__stdcall *HANDLE_ACCEPT_THREAD)(void*);		//������������߳�

class CSocketBase
{
public:
    CSocketBase();
	CSocketBase(SOCKET s);
    ~CSocketBase();

public:
    void SetRecvTimeOut(UINT uiMSec);
    void SetSendTimeOut(UINT uiMsec);

    void SetRecvBufferSize(UINT uiByte);
    void SetSendBufferSize(UINT uiByte);

    bool ConnectRemote(const char* pcRemoteIP = NULL, SHORT sPort = 0, USHORT nTimeOutSec = SOB_DEFAULT_TIMEOUT_SEC);
	bool BindOnPort(UINT uiPort);
	bool StartListenAndAccept(HANDLE_ACCEPT_THREAD pThreadFunc, BOOL* pExitFlag = NULL, USHORT nLoopTimeOutSec = SOB_DEFAULT_TIMEOUT_SEC);
    bool Reconnect();
    void Disconnect();

    int SendMsg(const char* pcMsg, USHORT nTimeOutSec = SOB_DEFAULT_TIMEOUT_SEC);
    int SendBuffer(char* pBuffer, UINT uiBufferSize, USHORT nTimeOutSec = SOB_DEFAULT_TIMEOUT_SEC);
    int RecvOnce(char* pRecvBuffer, UINT uiBufferSize, UINT& uiRecv, USHORT nTimeOutSec = SOB_DEFAULT_TIMEOUT_SEC);
    int RecvBuffer(char* pRecvBuffer, UINT uiBufferSize, UINT uiRecvSize, USHORT nTimeOutSec = SOB_DEFAULT_TIMEOUT_SEC);

	static bool ResolveAddressToIp(const char* pcAddress, char* pcIp);
public:
    SOCKET getRawSocket();
	void setRawSocket(SOCKET s);
    const char* getRemoteIP();
    SHORT getRemotePort();
    bool IsConnected();
	void Destroy();

    int m_nLastWSAError;

private:
    SOCKET CreateTCPSocket();

private:
    SOCKET m_socket;
    WSAEVENT m_SocketEvent;

    bool m_bIsConnected;
    char m_pcRemoteIP[SOB_IP_LENGTH];
    SHORT m_sRemotePort;
};

#endif
