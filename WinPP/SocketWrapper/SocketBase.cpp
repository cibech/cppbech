#include "stdafx.h"

#include <process.h>
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
		m_socket = NULL;
    }

    if(m_SocketEvent)
    {
        WSACloseEvent(m_SocketEvent);
		m_SocketEvent = NULL;
    }
}

void CSocketBase::setRawSocket(SOCKET s)
{
	//�й�SOCKET
	m_socket = s;

	//�����йܵ�SOCKET��Ĭ��������
	m_bIsConnected = true;

	//�����첽�¼�
	if(m_SocketEvent == NULL)
	{
		m_SocketEvent = WSACreateEvent();
	}
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
// ��ʼ�������ӣ�����������
//------------------------------------------------------------------------------
bool CSocketBase::StartListenAndAccept(HANDLE_ACCEPT_THREAD pThreadFunc, BOOL* pExitFlag, USHORT nLoopTimeOutSec)
{
	//�첽�¼�
	if(m_SocketEvent == NULL)
	{
		m_SocketEvent = WSACreateEvent();
	}

	//ע�������¼�
	WSAResetEvent(m_SocketEvent);           //���֮ǰ��δ������¼�
	WSAEventSelect(m_socket, m_SocketEvent, FD_ACCEPT | FD_CLOSE);

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
		DWORD dwRet = WSAWaitForMultipleEvents(1, &m_SocketEvent, FALSE, nLoopTimeOutSec*1000, FALSE);

		if(dwRet == WSA_WAIT_EVENT_0)
		{
			//��������¼�����
			WSANETWORKEVENTS wsaEvents;
			memset(&wsaEvents, 0, sizeof(wsaEvents));

			WSAResetEvent(m_SocketEvent);
			WSAEnumNetworkEvents(m_socket, m_SocketEvent, &wsaEvents);

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

				//��¼������־
				/*
				if(g_pLogger != NULL)
				{
					TCHAR pcLogTemp[128] = {0};
					ntohl(addrRemote.sin_addr.s_addr);
					char* pTempIp = inet_ntoa(addrRemote.sin_addr);

					wsprintf(pcLogTemp, _T("Remote IP %s connected with port %d"), 
						pTempIp, ntohs(addrRemote.sin_port));

					//д��
					g_pLogger->WriteLog(pcLogTemp);
				}*/

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
    WSAResetEvent(m_SocketEvent);
    WSAEventSelect(m_socket, m_SocketEvent, FD_WRITE | FD_CLOSE);

    //���Է���
    int nRet = send(m_socket, pcMsg, (int)strlen(pcMsg), NULL);

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
    WSAResetEvent(m_SocketEvent);
    WSAEventSelect(m_socket, m_SocketEvent, FD_WRITE | FD_CLOSE);

    //����������
    int nSent = 0;

    //�ܷ��ʹ�����Ϊ���ʹ�����һ���޶�
    int nSendTimes = 0;
    int nSendLimitTimes = uiBufferSize / 1000;      //�ٶ���ǰÿ�η��Ϳ϶�������1K
    UINT uiLeftBuffer = uiBufferSize;               //δ������Ļ����С

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
	else if(nRet == 0)		//�Է������ر�����
	{
		m_bIsConnected = false;	
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
    WSAResetEvent(m_SocketEvent);
    WSAEventSelect(m_socket, m_SocketEvent, FD_READ | FD_CLOSE);

    //����������
    int nReceived = 0;

    //�ܽ��մ�����Ϊ���մ�����һ���޶�
    int nRecvTimes = 0;
    int nRecvLimitTimes = uiRecvSize / 1000;      //�ٶ���ǰÿ�ν��տ϶�������1K

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
