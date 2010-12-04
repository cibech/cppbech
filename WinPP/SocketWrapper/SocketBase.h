//*****************************************************//
//                                                     //
//      创建人: wanyi                                  //
//      创建日期: 2010-11-05                           //
//                                                     //
//*****************************************************//

//==============================================================================
// 版本历史
// 2010-11-05 wanyi. TCP连接socket的基类
//==============================================================================

#ifndef SocketBaseH
#define SocketBaseH

#define SOB_IP_LENGTH   128             //IP字符串长度

class CSocketBase
{
public:
    CSocketBase();
    ~CSocketBase();

public:
    void SetRecvTimeOut(UINT uiMSec);
    void SetSendTimeOut(UINT uiMsec);

    void SetRecvBufferSize(UINT uiByte);
    void SetSendBufferSize(UINT uiByte);

    bool ConnectRemote(const char* pcRemoteIP = NULL, SHORT sPort = 0, USHORT nTimeOutSec = 5);
    bool Reconnect();
    void Disconnect();

    bool SendMsg(const char* pcMsg, USHORT nTimeOutSec = 5);
    bool SendBuffer(char* pBuffer, UINT uiBufferSize, USHORT nTimeOutSec = 5);
    bool RecvOnce(char* pRecvBuffer, UINT uiBufferSize, UINT& uiRecv, USHORT nTimeOutSec = 5);
    bool RecvBuffer(char* pRecvBuffer, UINT uiBufferSize, UINT uiRecvSize, USHORT nTimeOutSec = 5);

	static bool ResolveAddressToIp(const char* pcAddress, char* pcIp);

public:
    SOCKET getRawSocket();
    const char* getRemoteIP();
    SHORT getRemotePort();
    bool IsConnected();

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
