#include "SocketBase.h"

#include <Ws2tcpip.h>
#include <process.h>

void CSocketBase::InitSocketLib()
{
	//��ʼ��SOCKET����
	WORD wVersionRequested;
	WSADATA wsaData;
	int nErr;

	wVersionRequested = MAKEWORD(2, 2);

	nErr = WSAStartup(wVersionRequested, &wsaData);
	if (nErr != 0) 
	{
		return;
	}
}

void CSocketBase::ReleaseSocketLib()
{
	//����SOCKET����
	WSACleanup();
}

CSocketBase::CSocketBase()
{
    m_socket = NULL;
    m_SocketWriteEvent = NULL;
	m_SocketReadEvent = NULL;

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
		m_socket = NULL;
    }

    if(m_SocketWriteEvent || m_SocketReadEvent)
    {
        WSACloseEvent(m_SocketWriteEvent);
		WSACloseEvent(m_SocketReadEvent);
		m_SocketWriteEvent = NULL;
		m_SocketReadEvent = NULL;
    }
}

bool CSocketBase::setRawSocket(SOCKET s)
{
	//�й�SOCKET
	m_socket = s;

	//�����йܵ�SOCKET��Ĭ��������
	m_bIsConnected = true;

	//�����첽�¼�
	if(m_SocketWriteEvent == NULL)
	{
		m_SocketWriteEvent = WSACreateEvent();
	}

	if(m_SocketReadEvent == NULL)
	{
		m_SocketReadEvent = WSACreateEvent();
	}

	//��ȡԶ����Ϣ
	struct sockaddr_in addrPeer;
	int nLen = sizeof(addrPeer);
	if(getpeername(s, (struct sockaddr*)&addrPeer, &nLen) == SOCKET_ERROR)
	{
		return false;
	}

	strcpy(m_pcRemoteIP, inet_ntoa(addrPeer.sin_addr));
	m_sRemotePort = ntohs(addrPeer.sin_port);

	return true;
}

void CSocketBase::Destroy()
{
	this->~CSocketBase();
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

	//����SOCKET����
	const char chOpt = 1;
	int nRet = setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &chOpt, sizeof(char));

    return s;    
}

//------------------------------------------------------------------------------
// ����socket
//------------------------------------------------------------------------------
SOCKET CSocketBase::CreateUDPSocket()
{
	//����socket���������е�ǰ��
	SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

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
// �󶨵�TCP�˿�
//------------------------------------------------------------------------------
bool CSocketBase::BindOnPort(UINT uiPort)
{
	//���socket��Ч���½���Ϊ�˿����ظ�����
	if(m_socket == NULL)
	{
		m_socket = CreateTCPSocket();
	}	

	//�������ؼ���
	SOCKADDR_IN addrLocal;
	memset(&addrLocal, 0, sizeof(addrLocal));

	addrLocal.sin_family = AF_INET;
	addrLocal.sin_addr.s_addr = htonl(INADDR_ANY);
	addrLocal.sin_port = htons(uiPort);

	//��
	int nRet = bind(m_socket, (PSOCKADDR)&addrLocal, sizeof(addrLocal));

	//Ψһ��ԭ���Ƕ˿ڱ�ռ��
	if(nRet == SOCKET_ERROR)
	{
		m_nLastWSAError = WSAGetLastError();
		return false;
	}

	return true;
}

//------------------------------------------------------------------------------
// �󶨵�UDP�˿�
//------------------------------------------------------------------------------
bool CSocketBase::BindOnUDPPort(UINT uiPort)
{
	//���socket��Ч���½���Ϊ�˿����ظ�����
	if(m_socket == NULL)
	{
		m_socket = CreateUDPSocket();
	}	

	//�������ؼ���
	SOCKADDR_IN addrLocal;
	memset(&addrLocal, 0, sizeof(addrLocal));

	addrLocal.sin_family = AF_INET;
	addrLocal.sin_addr.s_addr = htonl(INADDR_ANY);
	addrLocal.sin_port = htons(uiPort);

	//��
	int nRet = bind(m_socket, (PSOCKADDR)&addrLocal, sizeof(addrLocal));

	//Ψһ��ԭ���Ƕ˿ڱ�ռ��
	if(nRet == SOCKET_ERROR)
	{
		m_nLastWSAError = WSAGetLastError();
		return false;
	}

	return true;	
}

//------------------------------------------------------------------------------
// ��ʼ�������ӣ�����������
//------------------------------------------------------------------------------
bool CSocketBase::StartListenAndAccept(HANDLE_ACCEPT_THREAD pThreadFunc, BOOL* pExitFlag, USHORT nLoopTimeOutSec)
{
	//�첽�¼�
	if(m_SocketReadEvent == NULL)
	{
		m_SocketReadEvent = WSACreateEvent();
	}

	//ע�������¼�
	WSAResetEvent(m_SocketReadEvent);           //���֮ǰ��δ������¼�
	WSAEventSelect(m_socket, m_SocketReadEvent, FD_ACCEPT | FD_CLOSE);

	//����
	int nRet = listen(m_socket, 5);

	//Ψһ��ԭ���Ƕ˿ڱ�ռ��
	if(nRet == SOCKET_ERROR)
	{
		m_nLastWSAError = WSAGetLastError();
		return false;
	}

	//�ȴ�����
	while((pExitFlag == NULL ? TRUE : *pExitFlag))
	{
		DWORD dwRet = WSAWaitForMultipleEvents(1, &m_SocketReadEvent, FALSE, nLoopTimeOutSec*1000, FALSE);

		if(dwRet == WSA_WAIT_EVENT_0)
		{
			//��������¼�����
			WSANETWORKEVENTS wsaEvents;
			memset(&wsaEvents, 0, sizeof(wsaEvents));

			WSAResetEvent(m_SocketReadEvent);
			WSAEnumNetworkEvents(m_socket, m_SocketReadEvent, &wsaEvents);

			if((wsaEvents.lNetworkEvents & FD_ACCEPT) &&
				(wsaEvents.iErrorCode[FD_ACCEPT_BIT] == 0))
			{
				//��¼Զ�̵�ַ
				SOCKADDR_IN addrRemote;
				memset(&addrRemote, 0, sizeof(addrRemote));
				int nAddrSize = sizeof(addrRemote);

				SOCKET sockRemote = accept(m_socket, (PSOCKADDR)&addrRemote, &nAddrSize);

				//��Ч����
				if(sockRemote == INVALID_SOCKET)
				{
					m_nLastWSAError = WSAGetLastError();

					//���̳߳�������
					continue;
				}

				//�����´����߳�
				HANDLE hThread;
				unsigned unThreadID;

				hThread = (HANDLE)_beginthreadex(NULL, 0, pThreadFunc, (void*)sockRemote, 0, &unThreadID);

				//�Ѿ�����Ҫ��HANDLE
				CloseHandle(hThread);
			}
		}
		else
		{
			//�ȴ���ʱ�����¿�ʼ
			continue;
		}
	}

	return true;
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
    if(m_SocketWriteEvent == NULL)
    {
        m_SocketWriteEvent = WSACreateEvent();
    }

    //ע�������¼�
    WSAResetEvent(m_SocketWriteEvent);           //���֮ǰ��δ������¼�
    WSAEventSelect(m_socket, m_SocketWriteEvent, FD_CONNECT | FD_CLOSE);

    //��������
    int nRet = connect(m_socket, (SOCKADDR*)&addrRemote, sizeof(addrRemote));

    if(nRet == SOCKET_ERROR)
    {
        m_nLastWSAError = WSAGetLastError();

        //�����������
        if(m_nLastWSAError == WSAEWOULDBLOCK)
        {
            DWORD dwRet = WSAWaitForMultipleEvents(1, &m_SocketWriteEvent, FALSE, nTimeOutSec*1000, FALSE);

            //��������¼�����
            WSANETWORKEVENTS wsaEvents;
            memset(&wsaEvents, 0, sizeof(wsaEvents));

            if(dwRet == WSA_WAIT_EVENT_0)
            {
                WSAResetEvent(m_SocketWriteEvent);
                WSAEnumNetworkEvents(m_socket, m_SocketWriteEvent, &wsaEvents);

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
        else
        {
           m_bIsConnected = false;
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

ULONG CSocketBase::getRemoteULIP()
{
	return inet_addr(m_pcRemoteIP);
}

SHORT CSocketBase::getRemotePort()
{
    return m_sRemotePort;    
}

//------------------------------------------------------------------------------
// �����ı�����һ������
//------------------------------------------------------------------------------
int CSocketBase::SendMsg(const char* pcMsg, USHORT nTimeOutSec)
{
	bool bIsTimeOut = false;

    //�������״̬
    if(!m_bIsConnected)
    {
        return SOB_RET_FAIL;
    }

    //����ǰע���¼�
    WSAResetEvent(m_SocketWriteEvent);
    WSAEventSelect(m_socket, m_SocketWriteEvent, FD_WRITE | FD_CLOSE);

    //���Է���
    int nRet = send(m_socket, pcMsg, (int)strlen(pcMsg), NULL);

    if(nRet == SOCKET_ERROR)
    {
        m_nLastWSAError = WSAGetLastError();

        //��������
        if(m_nLastWSAError == WSAEWOULDBLOCK)
        {
            DWORD dwRet = WSAWaitForMultipleEvents(1, &m_SocketWriteEvent, FALSE, nTimeOutSec*1000, FALSE);

            //��������¼�����
            WSANETWORKEVENTS wsaEvents;
            memset(&wsaEvents, 0, sizeof(wsaEvents));

            if(dwRet == WSA_WAIT_EVENT_0)
            {
                WSAResetEvent(m_SocketWriteEvent);
                WSAEnumNetworkEvents(m_socket, m_SocketWriteEvent, &wsaEvents);

                //������Ϳ��Խ��в���û�д�����
                if((wsaEvents.lNetworkEvents & FD_WRITE) &&
                    (wsaEvents.iErrorCode[FD_WRITE_BIT] == 0))
                {
                    //�ٴη����ı�
                    nRet = (int)send(m_socket, pcMsg, (int)strlen(pcMsg), NULL);

                    if(nRet > 0)
                    {
                        //��������ֽڴ���0���������ͳɹ�
                        return SOB_RET_OK;
                    }
                }
            }
			else
			{
				//��ʱ
				bIsTimeOut = true;
			}
        }
        else
        {
          m_bIsConnected = false;
        }
    }
    else
    {
        //��һ�α㷢�ͳɹ�
        return SOB_RET_OK;
    }

	//�����ʱ
	if(bIsTimeOut)
	{
		return SOB_RET_TIMEOUT;
	}

    //�����������ʧ��
    m_nLastWSAError = WSAGetLastError();
    return false;
}

//------------------------------------------------------------------------------
// ���ͻ�����
//------------------------------------------------------------------------------
int CSocketBase::SendBuffer(char* pBuffer, UINT uiBufferSize, USHORT nTimeOutSec)
{
	bool bIsTimeOut = false;

    //�������״̬
    if(!m_bIsConnected)
    {
        return SOB_RET_FAIL;
    }

    //����ǰע���¼�
    WSAResetEvent(m_SocketWriteEvent);
    WSAEventSelect(m_socket, m_SocketWriteEvent, FD_WRITE | FD_CLOSE);

    //����������
    int nSent = 0;

    //�ܷ��ʹ�����Ϊ���ʹ�����һ���޶�
    int nSendTimes = 0;
    int nSendLimitTimes = (int)((float)uiBufferSize / 500 + 1.5);      //�ٶ���ǰÿ�η��Ϳ϶�������500�ֽ�
    UINT uiLeftBuffer = uiBufferSize;									//δ������Ļ����С

    //�����α�
    char* pcSentPos = pBuffer;

    //ֱ�����еĻ��嶼�������
    while(nSent < (int)uiBufferSize)
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
                DWORD dwRet = WSAWaitForMultipleEvents(1, &m_SocketWriteEvent, FALSE, nTimeOutSec*1000, FALSE);

                //��������¼�����
                WSANETWORKEVENTS wsaEvents;
                memset(&wsaEvents, 0, sizeof(wsaEvents));

                if(dwRet == WSA_WAIT_EVENT_0)
                {
                    WSAResetEvent(m_SocketWriteEvent);
                    WSAEnumNetworkEvents(m_socket, m_SocketWriteEvent, &wsaEvents);

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
				else
				{
					//��ʱ
					bIsTimeOut = true;
					break;
				}
            }
            else
            {
                //��������֮��Ĵ���ֱ���˳�
                m_bIsConnected = false;
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
        return SOB_RET_OK;
    }

	//�����ʱ
	if(bIsTimeOut)
	{
		return SOB_RET_TIMEOUT;
	}

    //û�ܳɹ�����
    m_nLastWSAError = WSAGetLastError();
    return SOB_RET_FAIL;
}

//------------------------------------------------------------------------------
// ����UDP���壬�����ڵײ����ݱ���Χ��
//------------------------------------------------------------------------------
int CSocketBase::SendUDPBuffer(const char* pcIP, SHORT sPort, char* pBuffer, UINT uiBufferSize, USHORT nTimeOutSec)
{
	if(m_socket == NULL)
	{
		m_socket = CreateUDPSocket();
	}

	bool bIsTimeOut = false;

	//����������
	int nSent = 0;

	//�ܷ��ʹ�����Ϊ���ʹ�����һ���޶�
	int nSendTimes = 0;
	int nSendLimitTimes = nTimeOutSec * 1000 / 100;						//��������������ȴ�100ms���ط�
	UINT uiLeftBuffer = uiBufferSize;									//δ������Ļ����С

	//ת��Զ�̵�ַ
	SOCKADDR_IN addrRemote;
	memset(&addrRemote, 0, sizeof(addrRemote));

	addrRemote.sin_family = AF_INET;
	addrRemote.sin_addr.s_addr = inet_addr(pcIP);
	addrRemote.sin_port = htons(sPort);

	//�����α�
	char* pcSentPos = pBuffer;

	//ֱ�����еĻ��嶼�������
	while(nSent < (int)uiBufferSize)
	{
		//��鷢�ʹ����Ƿ���
		if(nSendTimes > nSendLimitTimes)
		{
			bIsTimeOut = true;
			break;
		}

		int nRet = sendto(m_socket, pcSentPos, uiLeftBuffer, NULL, (PSOCKADDR)&addrRemote, sizeof(addrRemote));

		if(nRet == SOCKET_ERROR)
		{
			m_nLastWSAError = WSAGetLastError();
			break;
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
		return SOB_RET_OK;
	}

	//�����ʱ
	if(bIsTimeOut)
	{
		return SOB_RET_TIMEOUT;
	}

	//û�ܳɹ�����
	m_nLastWSAError = WSAGetLastError();
	return SOB_RET_FAIL;
}

//------------------------------------------------------------------------------
// ����UDP����
//------------------------------------------------------------------------------
int CSocketBase::RecvUDPMsg(char* pBuffer, UINT uiBufferSize, UINT& uiRecv, char* pcIP, USHORT& uPort, USHORT nTimeOutSec)
{
	bool bIsTimeOut = false;

	//����ǰע���¼�
	HANDLE socketEvent = WSACreateEvent();
	WSAResetEvent(socketEvent);
	WSAEventSelect(m_socket, socketEvent, FD_READ);

	//Զ����Ϣ
	SOCKADDR_IN addrRemote;
	int nAddrLen = sizeof(addrRemote);
	memset(&addrRemote, 0, nAddrLen);

	//���Խ���
	int nRet = recvfrom(m_socket, pBuffer, uiBufferSize, NULL, (PSOCKADDR)&addrRemote, &nAddrLen);

	if(nRet == SOCKET_ERROR)
	{
		m_nLastWSAError = WSAGetLastError();

		//��������
		if(m_nLastWSAError == WSAEWOULDBLOCK)
		{
			DWORD dwRet = WSAWaitForMultipleEvents(1, &socketEvent, FALSE, nTimeOutSec*1000, FALSE);

			//��������¼�����
			WSANETWORKEVENTS wsaEvents;
			memset(&wsaEvents, 0, sizeof(wsaEvents));

			if(dwRet == WSA_WAIT_EVENT_0)
			{
				WSAResetEvent(socketEvent);
				int nEnum = WSAEnumNetworkEvents(m_socket, socketEvent, &wsaEvents);

				//������ܿ��Խ��в���û�д�����
				if((wsaEvents.lNetworkEvents & FD_READ) &&
					(wsaEvents.iErrorCode[FD_READ_BIT] == 0))
				{
					//�ٴν����ı�
					nRet = recvfrom(m_socket, pBuffer, uiBufferSize, NULL, (PSOCKADDR)&addrRemote, &nAddrLen);

					if(nRet > 0)
					{
						//����ֽڴ���0���������ճɹ�
						uiRecv = nRet;

						//����IP�Ͷ˿�
						strcpy(pcIP, inet_ntoa(addrRemote.sin_addr));
						uPort = ntohs(addrRemote.sin_port);

						WSACloseEvent(socketEvent);
						return SOB_RET_OK;
					}
				}
			}
			else
			{
				bIsTimeOut = true;
			}
		}
	}
	else
	{
		//��һ�α���ճɹ�
		uiRecv = nRet;

		//����IP�Ͷ˿�
		strcpy(pcIP, inet_ntoa(addrRemote.sin_addr));
		uPort = ntohs(addrRemote.sin_port);

		WSACloseEvent(socketEvent);
		return SOB_RET_OK;
	}

	//�����ʱ
	if(bIsTimeOut)
	{
		WSACloseEvent(socketEvent);
		return SOB_RET_TIMEOUT;
	}

	//�����������ʧ��
	m_nLastWSAError = WSAGetLastError();

	WSACloseEvent(socketEvent);
	return SOB_RET_FAIL;
}

//------------------------------------------------------------------------------
// �����ı�����һ�ν���
//------------------------------------------------------------------------------
int CSocketBase::RecvOnce(char* pRecvBuffer, UINT uiBufferSize, UINT& uiRecv, USHORT nTimeOutSec)
{
	bool bIsTimeOut = false;

    //�������״̬
    if(!m_bIsConnected)
    {
        return SOB_RET_FAIL;
    }

    //����ǰע���¼�
    WSAResetEvent(m_SocketReadEvent);
    WSAEventSelect(m_socket, m_SocketReadEvent, FD_READ | FD_CLOSE);

    //���Խ���
    int nRet = recv(m_socket, pRecvBuffer, uiBufferSize, NULL);

    if(nRet == SOCKET_ERROR)
    {
        m_nLastWSAError = WSAGetLastError();

        //��������
        if(m_nLastWSAError == WSAEWOULDBLOCK)
        {
            DWORD dwRet = WSAWaitForMultipleEvents(1, &m_SocketReadEvent, FALSE, nTimeOutSec*1000, FALSE);

            //��������¼�����
            WSANETWORKEVENTS wsaEvents;
            memset(&wsaEvents, 0, sizeof(wsaEvents));

            if(dwRet == WSA_WAIT_EVENT_0)
            {
                WSAResetEvent(m_SocketReadEvent);
                int nEnum = WSAEnumNetworkEvents(m_socket, m_SocketReadEvent, &wsaEvents);

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
                        return SOB_RET_OK;
                    }
                }
            }
			else
			{
				bIsTimeOut = true;
			}
        }
        else
        {
           m_bIsConnected = false;
        }
    }
    else
    {
        //��һ�α���ճɹ�
        uiRecv = nRet;
        return SOB_RET_OK;
    }

	//�����ʱ
	if(bIsTimeOut)
	{
		return SOB_RET_TIMEOUT;
	}

    //�����������ʧ��
    m_nLastWSAError = WSAGetLastError();
    return SOB_RET_FAIL;
}

//------------------------------------------------------------------------------
// ���ջ�����������Ӧ�ñȴ�������Ҫ��һ��Ű�ȫ
//------------------------------------------------------------------------------
int CSocketBase::RecvBuffer(char* pRecvBuffer, UINT uiBufferSize, UINT uiRecvSize, USHORT nTimeOutSec)
{
	bool bIsTimeOut = false;

    //�������״̬
    if(!m_bIsConnected)
    {
        return SOB_RET_FAIL;
    }

    //����ǰע���¼�
    WSAResetEvent(m_SocketReadEvent);
    WSAEventSelect(m_socket, m_SocketReadEvent, FD_READ | FD_CLOSE);

    //����������
    int nReceived = 0;

    //�ܽ��մ�����Ϊ���մ�����һ���޶�
    int nRecvTimes = 0;
    int nRecvLimitTimes = (int)((float)uiRecvSize / 500 + 1.5);      //�ٶ���ǰÿ�ν��տ϶�������500�ֽ�

    //�����α�
    char* pcRecvPos = pRecvBuffer;

    //ֱ�����յ��㹻ָ�����Ļ���
    while(nReceived < (int)uiRecvSize)
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
                DWORD dwRet = WSAWaitForMultipleEvents(1, &m_SocketReadEvent, FALSE, nTimeOutSec*1000, FALSE);

                //��������¼�����
                WSANETWORKEVENTS wsaEvents;
                memset(&wsaEvents, 0, sizeof(wsaEvents));

                if(dwRet == WSA_WAIT_EVENT_0)
                {
                    WSAResetEvent(m_SocketReadEvent);
                    WSAEnumNetworkEvents(m_socket, m_SocketReadEvent, &wsaEvents);

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
				else
				{
					//��ʱ
					bIsTimeOut = true;
					break;
				}
            }
            else
            {
                //��������֮��Ĵ���ֱ���˳�
                m_bIsConnected = false;
                break;
            }
        }
		else if(nRet == 0)		//�Է������ر�����
		{
			m_bIsConnected = false;
			break;	
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
        return SOB_RET_OK;
    }

	//�����ʱ
	if(bIsTimeOut)
	{
		return SOB_RET_TIMEOUT;
	}

    //û�ܳɹ�����
    m_nLastWSAError = WSAGetLastError();
    return SOB_RET_FAIL;
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
