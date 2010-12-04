#include "stdafx.h"

#include "SocketBase.h"

CSocketBase::CSocketBase()
{
    m_socket = NULL;
    m_SocketEvent = NULL;

    m_bIsConnected = false;

    //��ʼ��Զ�̲���
    memset(m_pcRemoteIP, 0, SOB_IP_LENGTH);
    m_sRemotePort = 0;
}

CSocketBase::~CSocketBase()
{
    if(m_socket)
    {
        closesocket(m_socket);
    }

    if(m_SocketEvent)
    {
        WSACloseEvent(m_SocketEvent);
    }
}

//------------------------------------------------------------------------------
// ����socket
//------------------------------------------------------------------------------
SOCKET CSocketBase::CreateTCPSocket()
{
    //����socket���������е�ǰ��
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    //�������������������Ϊ��
    if(s == INVALID_SOCKET)
    {
        s = NULL;
        m_nLastWSAError = WSAGetLastError();
    }

    return s;    
}

//------------------------------------------------------------------------------
// ���ý��ܳ�ʱ
//------------------------------------------------------------------------------
void CSocketBase::SetRecvTimeOut(UINT uiMSec)
{
    UINT uiMseTimeOut = uiMSec;

    setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&uiMseTimeOut, sizeof(uiMseTimeOut));
}

//------------------------------------------------------------------------------
// ���÷��ͳ�ʱ
//------------------------------------------------------------------------------
void CSocketBase::SetSendTimeOut(UINT uiMSec)
{
    UINT uiMseTimeOut = uiMSec;

    setsockopt(m_socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&uiMseTimeOut, sizeof(uiMseTimeOut));
}

//------------------------------------------------------------------------------
// ���ý��ܻ���
//------------------------------------------------------------------------------
void CSocketBase::SetRecvBufferSize(UINT uiByte)
{
    UINT uiBufferSize = uiByte;

    setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, (char*)&uiBufferSize, sizeof(uiBufferSize));    
}

//------------------------------------------------------------------------------
// ���÷��ͻ���
//------------------------------------------------------------------------------
void CSocketBase::SetSendBufferSize(UINT uiByte)
{
    UINT uiBufferSize = uiByte;

    setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, (char*)&uiBufferSize, sizeof(uiBufferSize));    
}

//------------------------------------------------------------------------------
// socket
//------------------------------------------------------------------------------
SOCKET CSocketBase::getRawSocket()
{
    return m_socket;
}

//------------------------------------------------------------------------------
// ���ӵ�Զ�̣������������޲ε��ã���ʾ����
//------------------------------------------------------------------------------
bool CSocketBase::ConnectRemote(const char* pcRemoteIP, SHORT sPort, USHORT nTimeOutSec)
{
    //���socket��Ч���½���Ϊ�˿����ظ�����
    if(m_socket == NULL)
    {
        m_socket = CreateTCPSocket();
    }

    //�趨Զ��IP
    SOCKADDR_IN addrRemote;
    memset(&addrRemote, 0, sizeof(addrRemote));

    //�����Ҫ�����²����������ʾ�õ�ǰIP����
    if(pcRemoteIP != NULL && sPort != 0)
    {
        memset(m_pcRemoteIP, 0, SOB_IP_LENGTH);
        strcpy(m_pcRemoteIP, pcRemoteIP);
        m_sRemotePort = sPort;
    }
    
    addrRemote.sin_family = AF_INET;
    addrRemote.sin_addr.S_un.S_addr = inet_addr(m_pcRemoteIP);
    addrRemote.sin_port = htons(m_sRemotePort);

    //�첽�¼�
    if(m_SocketEvent == NULL)
    {
        m_SocketEvent = WSACreateEvent();
    }

    //ע�������¼�
    WSAResetEvent(m_SocketEvent);           //���֮ǰ��δ������¼�
    WSAEventSelect(m_socket, m_SocketEvent, FD_CONNECT | FD_CLOSE);

    //��������
    int nRet = connect(m_socket, (SOCKADDR*)&addrRemote, sizeof(addrRemote));

    if(nRet == SOCKET_ERROR)
    {
        m_nLastWSAError = WSAGetLastError();

        //�����������
        if(m_nLastWSAError == WSAEWOULDBLOCK)
        {
            DWORD dwRet = WSAWaitForMultipleEvents(1, &m_SocketEvent, FALSE, nTimeOutSec*1000, FALSE);

            //��������¼�����
            WSANETWORKEVENTS wsaEvents;
            memset(&wsaEvents, 0, sizeof(wsaEvents));

            if(dwRet == WSA_WAIT_EVENT_0)
            {
                WSAResetEvent(m_SocketEvent);
                WSAEnumNetworkEvents(m_socket, m_SocketEvent, &wsaEvents);

                //������ӽ��ܲ���û�д�����
                if((wsaEvents.lNetworkEvents & FD_CONNECT) &&
                    (wsaEvents.iErrorCode[FD_CONNECT_BIT] == 0))
                {
                    //ʵ�ʵĵ��Ծ��������ң����������ͣ�٣��ᱨ�����˷ѽ��10022����
                    //����ͬһsocket������connect���̫�̿����ǲ��е�,���������Magic Number��50������ʵ�����Ľ����Sleep(1)��һ��ʱ��Ƭ�ƺ�����
                    Sleep(50);
                    nRet = connect(m_socket, (SOCKADDR*)&addrRemote, sizeof(addrRemote));

                    if(nRet == SOCKET_ERROR)
                    {
                        m_nLastWSAError = WSAGetLastError();

                        //�������ʵ�����Ѿ�����
                        if(m_nLastWSAError == WSAEISCONN)
                        {
                            m_bIsConnected = true;
                        }
                    }
                    else    //���������ӳɹ�
                    {
                        m_bIsConnected = true;
                    }
                }
            }
        }
    }
    else
    {
        //��һ�γ��Ա����ӳɹ�
        m_bIsConnected = true;
    }

    //�������ʧ��
    if(!m_bIsConnected)
    {
        closesocket(m_socket);
        m_socket = NULL;
    }

    //���ؽ������
    return m_bIsConnected;
}

//------------------------------------------------------------------------------
// ��Զ�̶Ͽ�����
//------------------------------------------------------------------------------
void CSocketBase::Disconnect()
{
    if(m_socket == NULL)
    {
        return;
    }

    //�����������״̬��ر�����
    if(m_bIsConnected)
    {
        shutdown(m_socket, SD_BOTH);

        m_bIsConnected = false;
    }

    closesocket(m_socket);
    m_socket = NULL;
}

//------------------------------------------------------------------------------
// ��������
//------------------------------------------------------------------------------
bool CSocketBase::Reconnect()
{
    Disconnect();

    Sleep(100);

    return ConnectRemote();    
}

bool CSocketBase::IsConnected()
{
    return m_bIsConnected;
}

const char* CSocketBase::getRemoteIP()
{
    if(m_pcRemoteIP[0] == 0)
    {
        return NULL;
    }

    return m_pcRemoteIP;
}

SHORT CSocketBase::getRemotePort()
{
    return m_sRemotePort;    
}

//------------------------------------------------------------------------------
// �����ı�����һ������
//------------------------------------------------------------------------------
bool CSocketBase::SendMsg(const char* pcMsg, USHORT nTimeOutSec)
{
    //�������״̬
    if(!m_bIsConnected)
    {
        return false;
    }

    //����ǰע���¼�
    WSAResetEvent(m_SocketEvent);
    WSAEventSelect(m_socket, m_SocketEvent, FD_WRITE | FD_CLOSE);

    //���Է���
    int nRet = send(m_socket, pcMsg, strlen(pcMsg), NULL);

    if(nRet == SOCKET_ERROR)
    {
        m_nLastWSAError = WSAGetLastError();

        //��������
        if(m_nLastWSAError == WSAEWOULDBLOCK)
        {
            DWORD dwRet = WSAWaitForMultipleEvents(1, &m_SocketEvent, FALSE, nTimeOutSec*1000, FALSE);

            //��������¼�����
            WSANETWORKEVENTS wsaEvents;
            memset(&wsaEvents, 0, sizeof(wsaEvents));

            if(dwRet == WSA_WAIT_EVENT_0)
            {
                WSAResetEvent(m_SocketEvent);
                WSAEnumNetworkEvents(m_socket, m_SocketEvent, &wsaEvents);

                //������Ϳ��Խ��в���û�д�����
                if((wsaEvents.lNetworkEvents & FD_WRITE) &&
                    (wsaEvents.iErrorCode[FD_WRITE_BIT] == 0))
                {
                    //�ٴη����ı�
                    nRet = send(m_socket, pcMsg, strlen(pcMsg), NULL);

                    if(nRet > 0)
                    {
                        //��������ֽڴ���0���������ͳɹ�
                        return true;
                    }
                }
            }        
        }
    }
    else
    {
        //��һ�α㷢�ͳɹ�
        return true;
    }

    //�����������ʧ��
    m_nLastWSAError = WSAGetLastError();
    return false;
}

//------------------------------------------------------------------------------
// ���ͻ�����
//------------------------------------------------------------------------------
bool CSocketBase::SendBuffer(char* pBuffer, UINT uiBufferSize, USHORT nTimeOutSec)
{
    //�������״̬
    if(!m_bIsConnected)
    {
        return false;
    }

    //����ǰע���¼�
    WSAResetEvent(m_SocketEvent);
    WSAEventSelect(m_socket, m_SocketEvent, FD_WRITE | FD_CLOSE);

    //����������
    UINT nSent = 0;

    //�ܷ��ʹ�����Ϊ���ʹ�����һ���޶�
    int nSendTimes = 0;
    int nSendLimitTimes = uiBufferSize / 1000;      //�ٶ���ǰÿ�η��Ϳ϶�������1K
    UINT uiLeftBuffer = uiBufferSize;               //δ������Ļ����С

    //�����α�
    char* pcSentPos = pBuffer;

    //ֱ�����еĻ��嶼�������
    while(nSent < uiBufferSize)
    {
        //��鷢�ʹ����Ƿ���
        if(nSendTimes > nSendLimitTimes)
        {
            break;
        }

        int nRet = send(m_socket, pcSentPos, uiLeftBuffer, NULL);

        if(nRet == SOCKET_ERROR)
        {
            m_nLastWSAError = WSAGetLastError();

            //��������
            if(m_nLastWSAError == WSAEWOULDBLOCK)
            {
                DWORD dwRet = WSAWaitForMultipleEvents(1, &m_SocketEvent, FALSE, nTimeOutSec*1000, FALSE);

                //��������¼�����
                WSANETWORKEVENTS wsaEvents;
                memset(&wsaEvents, 0, sizeof(wsaEvents));

                if(dwRet == WSA_WAIT_EVENT_0)
                {
                    WSAResetEvent(m_SocketEvent);
                    WSAEnumNetworkEvents(m_socket, m_SocketEvent, &wsaEvents);

                    //������Ϳ��Խ��в���û�д�����
                    if((wsaEvents.lNetworkEvents & FD_WRITE) &&
                        (wsaEvents.iErrorCode[FD_WRITE_BIT] == 0))
                    {
                        //�ٴη����ı�
                        nRet = send(m_socket, pcSentPos, uiLeftBuffer, NULL);

                        if(nRet > 0)
                        {
                            //��������ֽڴ���0���������ͳɹ�
                            nSendTimes++;

                            nSent += nRet;
                            uiLeftBuffer -= nRet;
                            pcSentPos += nRet;
                        }
                        else
                        {
                            m_nLastWSAError = WSAGetLastError();

                            //������յ��¼�����ʱҲ�����ˣ���ȴ�һ�����ں�����
                            if(m_nLastWSAError == WSAEWOULDBLOCK)
                            {
                                Sleep(1);        
                            }
                            else
                            {
                                //������������ֱ���˳�
                                break;
                            }
                        }
                    }
                }
            }
            else
            {
                //��������֮��Ĵ���ֱ���˳�
                break;
            }
        }
        else
        {
            //���ͳɹ����ۼӷ������������α�
            nSendTimes++;
            
            nSent += nRet;
            uiLeftBuffer -= nRet;
            pcSentPos += nRet;
        }
    }

    //����������
    if(nSent == uiBufferSize)
    {
        return true;
    }

    //û�ܳɹ�����
    m_nLastWSAError = WSAGetLastError();
    return false;
}

//------------------------------------------------------------------------------
// �����ı�����һ�ν���
//------------------------------------------------------------------------------
bool CSocketBase::RecvOnce(char* pRecvBuffer, UINT uiBufferSize, UINT& uiRecv, USHORT nTimeOutSec)
{
    //�������״̬
    if(!m_bIsConnected)
    {
        return false;
    }

    //����ǰע���¼�
    WSAResetEvent(m_SocketEvent);
    WSAEventSelect(m_socket, m_SocketEvent, FD_READ | FD_CLOSE);

    //���Խ���
    int nRet = recv(m_socket, pRecvBuffer, uiBufferSize, NULL);

    if(nRet == SOCKET_ERROR)
    {
        m_nLastWSAError = WSAGetLastError();

        //��������
        if(m_nLastWSAError == WSAEWOULDBLOCK)
        {
            DWORD dwRet = WSAWaitForMultipleEvents(1, &m_SocketEvent, FALSE, nTimeOutSec*1000, FALSE);

            //��������¼�����
            WSANETWORKEVENTS wsaEvents;
            memset(&wsaEvents, 0, sizeof(wsaEvents));

            if(dwRet == WSA_WAIT_EVENT_0)
            {
                WSAResetEvent(m_SocketEvent);
                int nEnum = WSAEnumNetworkEvents(m_socket, m_SocketEvent, &wsaEvents);

                //������ܿ��Խ��в���û�д�����
                if((wsaEvents.lNetworkEvents & FD_READ) &&
                    (wsaEvents.iErrorCode[FD_READ_BIT] == 0))
                {
                    //�ٴν����ı�
                    nRet = recv(m_socket, pRecvBuffer, uiBufferSize, NULL);

                    if(nRet > 0)
                    {
                        //��������ֽڴ���0���������ͳɹ�
                        uiRecv = nRet;
                        return true;
                    }
                }
            }        
        }
    }
    else
    {
        //��һ�α���ճɹ�
        uiRecv = nRet;
        return true;
    }

    //�����������ʧ��
    m_nLastWSAError = WSAGetLastError();
    return false;
}

//------------------------------------------------------------------------------
// ���ջ�����������Ӧ�ñȴ�������Ҫ��һ��Ű�ȫ
//------------------------------------------------------------------------------
bool CSocketBase::RecvBuffer(char* pRecvBuffer, UINT uiBufferSize, UINT uiRecvSize, USHORT nTimeOutSec)
{
    //�������״̬
    if(!m_bIsConnected)
    {
        return false;
    }

    //����ǰע���¼�
    WSAResetEvent(m_SocketEvent);
    WSAEventSelect(m_socket, m_SocketEvent, FD_READ | FD_CLOSE);

    //����������
    UINT nReceived = 0;

    //�ܽ��մ�����Ϊ���մ�����һ���޶�
    int nRecvTimes = 0;
    int nRecvLimitTimes = uiRecvSize / 1000;      //�ٶ���ǰÿ�ν��տ϶�������1K

    //�����α�
    char* pcRecvPos = pRecvBuffer;

    //ֱ�����յ��㹻ָ�����Ļ���
    while(nReceived < uiRecvSize)
    {
        //�����մ����Ƿ���
        if(nRecvTimes > nRecvLimitTimes)
        {
            break;
        }

        int nRet = recv(m_socket, pcRecvPos, uiBufferSize, NULL);

        if(nRet == SOCKET_ERROR)
        {
            m_nLastWSAError = WSAGetLastError();

            //��������
            if(m_nLastWSAError == WSAEWOULDBLOCK)
            {
                DWORD dwRet = WSAWaitForMultipleEvents(1, &m_SocketEvent, FALSE, nTimeOutSec*1000, FALSE);

                //��������¼�����
                WSANETWORKEVENTS wsaEvents;
                memset(&wsaEvents, 0, sizeof(wsaEvents));

                if(dwRet == WSA_WAIT_EVENT_0)
                {
                    WSAResetEvent(m_SocketEvent);
                    WSAEnumNetworkEvents(m_socket, m_SocketEvent, &wsaEvents);

                    //������տ��Խ��в���û�д�����
                    if((wsaEvents.lNetworkEvents & FD_READ) &&
                        (wsaEvents.iErrorCode[FD_READ_BIT] == 0))
                    {
                        //�ٴν���
                        nRet = recv(m_socket, pcRecvPos, uiBufferSize, NULL);

                        if(nRet > 0)
                        {
                            //��������ֽڴ���0���������ͳɹ�
                            nRecvTimes++;

                            nReceived += nRet;
                            uiBufferSize -= nRet;                            
                            pcRecvPos += nRet;
                        }
                        else
                        {
                            m_nLastWSAError = WSAGetLastError();

                            //������յ��¼�����ʱҲ�����ˣ���ȴ�һ�����ں�����
                            if(m_nLastWSAError == WSAEWOULDBLOCK)
                            {
                                Sleep(1);        
                            }
                            else
                            {
                                //������������ֱ���˳�
                                break;
                            }
                        }
                    }
                }
            }
            else
            {
                //��������֮��Ĵ���ֱ���˳�
                break;
            }
        }
        else
        {
            //���ճɹ����ۼӽ������������α�
            nRecvTimes++;
            
            nReceived += nRet;
            uiBufferSize -= nRet;
            pcRecvPos += nRet;
        }
    }

    //����������
    if(nReceived == uiRecvSize)
    {
        return true;
    }

    //û�ܳɹ�����
    m_nLastWSAError = WSAGetLastError();
    return false;
}

//------------------------------------------------------------------------------
// ת����ַΪIP
//------------------------------------------------------------------------------
bool CSocketBase::ResolveAddressToIp(const char* pcAddress, char* pcIp)
{
	addrinfo adiHints, *padiResult;
	int	nRet;

	memset(&adiHints, 0, sizeof(addrinfo));

	//������IPV4�ĵ�ַ
	adiHints.ai_flags = AI_CANONNAME;
	adiHints.ai_family = AF_INET;
	adiHints.ai_socktype = SOCK_STREAM;
	adiHints.ai_protocol = IPPROTO_TCP;

	//ת����ַ
	nRet = ::getaddrinfo(pcAddress, NULL, &adiHints, &padiResult);
	
	//�����
	if(nRet != 0) 
	{
		freeaddrinfo(padiResult);
		return false;
	}
	
	//�������,ֻ������һ��
	if(padiResult->ai_addr != NULL)
	{
		::strcpy(pcIp, inet_ntoa(((sockaddr_in*)padiResult->ai_addr)->sin_addr));
	}
	
	freeaddrinfo(padiResult);
	return true;
}