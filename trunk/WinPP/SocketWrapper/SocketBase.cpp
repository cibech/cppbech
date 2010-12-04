#include "stdafx.h"

#include "SocketBase.h"

CSocketBase::CSocketBase()
{
    m_socket = NULL;
    m_SocketEvent = NULL;

    m_bIsConnected = false;

    //初始化远程参数
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
// 创建socket
//------------------------------------------------------------------------------
SOCKET CSocketBase::CreateTCPSocket()
{
    //创建socket，这是所有的前提
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    //如果创建出错，我们置其为空
    if(s == INVALID_SOCKET)
    {
        s = NULL;
        m_nLastWSAError = WSAGetLastError();
    }

    return s;    
}

//------------------------------------------------------------------------------
// 设置接受超时
//------------------------------------------------------------------------------
void CSocketBase::SetRecvTimeOut(UINT uiMSec)
{
    UINT uiMseTimeOut = uiMSec;

    setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&uiMseTimeOut, sizeof(uiMseTimeOut));
}

//------------------------------------------------------------------------------
// 设置发送超时
//------------------------------------------------------------------------------
void CSocketBase::SetSendTimeOut(UINT uiMSec)
{
    UINT uiMseTimeOut = uiMSec;

    setsockopt(m_socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&uiMseTimeOut, sizeof(uiMseTimeOut));
}

//------------------------------------------------------------------------------
// 设置接受缓冲
//------------------------------------------------------------------------------
void CSocketBase::SetRecvBufferSize(UINT uiByte)
{
    UINT uiBufferSize = uiByte;

    setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, (char*)&uiBufferSize, sizeof(uiBufferSize));    
}

//------------------------------------------------------------------------------
// 设置发送缓冲
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
// 连接到远程，本方法可以无参调用，表示重连
//------------------------------------------------------------------------------
bool CSocketBase::ConnectRemote(const char* pcRemoteIP, SHORT sPort, USHORT nTimeOutSec)
{
    //如果socket无效则新建，为了可以重复调用
    if(m_socket == NULL)
    {
        m_socket = CreateTCPSocket();
    }

    //设定远程IP
    SOCKADDR_IN addrRemote;
    memset(&addrRemote, 0, sizeof(addrRemote));

    //如果需要，更新参数，否则表示用当前IP重连
    if(pcRemoteIP != NULL && sPort != 0)
    {
        memset(m_pcRemoteIP, 0, SOB_IP_LENGTH);
        strcpy(m_pcRemoteIP, pcRemoteIP);
        m_sRemotePort = sPort;
    }
    
    addrRemote.sin_family = AF_INET;
    addrRemote.sin_addr.S_un.S_addr = inet_addr(m_pcRemoteIP);
    addrRemote.sin_port = htons(m_sRemotePort);

    //异步事件
    if(m_SocketEvent == NULL)
    {
        m_SocketEvent = WSACreateEvent();
    }

    //注册连接事件
    WSAResetEvent(m_SocketEvent);           //清除之前尚未处理的事件
    WSAEventSelect(m_socket, m_SocketEvent, FD_CONNECT | FD_CLOSE);

    //进行连接
    int nRet = connect(m_socket, (SOCKADDR*)&addrRemote, sizeof(addrRemote));

    if(nRet == SOCKET_ERROR)
    {
        m_nLastWSAError = WSAGetLastError();

        //如果发生阻塞
        if(m_nLastWSAError == WSAEWOULDBLOCK)
        {
            DWORD dwRet = WSAWaitForMultipleEvents(1, &m_SocketEvent, FALSE, nTimeOutSec*1000, FALSE);

            //如果网络事件发生
            WSANETWORKEVENTS wsaEvents;
            memset(&wsaEvents, 0, sizeof(wsaEvents));

            if(dwRet == WSA_WAIT_EVENT_0)
            {
                WSAResetEvent(m_SocketEvent);
                WSAEnumNetworkEvents(m_socket, m_SocketEvent, &wsaEvents);

                //如果连接接受并且没有错误发生
                if((wsaEvents.lNetworkEvents & FD_CONNECT) &&
                    (wsaEvents.iErrorCode[FD_CONNECT_BIT] == 0))
                {
                    //实际的调试经历告诉我，这里如果不停顿，会报出令人费解的10022错误
                    //对于同一socket，两次connect间隔太短看来是不行的,至于这里的Magic Number的50，这是实践出的结果，Sleep(1)的一个时间片似乎不够
                    Sleep(50);
                    nRet = connect(m_socket, (SOCKADDR*)&addrRemote, sizeof(addrRemote));

                    if(nRet == SOCKET_ERROR)
                    {
                        m_nLastWSAError = WSAGetLastError();

                        //此种情况实际上已经连接
                        if(m_nLastWSAError == WSAEISCONN)
                        {
                            m_bIsConnected = true;
                        }
                    }
                    else    //阻塞后连接成功
                    {
                        m_bIsConnected = true;
                    }
                }
            }
        }
    }
    else
    {
        //第一次尝试便连接成功
        m_bIsConnected = true;
    }

    //如果连接失败
    if(!m_bIsConnected)
    {
        closesocket(m_socket);
        m_socket = NULL;
    }

    //返回接续结果
    return m_bIsConnected;
}

//------------------------------------------------------------------------------
// 和远程断开连接
//------------------------------------------------------------------------------
void CSocketBase::Disconnect()
{
    if(m_socket == NULL)
    {
        return;
    }

    //如果尚在连接状态则关闭连接
    if(m_bIsConnected)
    {
        shutdown(m_socket, SD_BOTH);

        m_bIsConnected = false;
    }

    closesocket(m_socket);
    m_socket = NULL;
}

//------------------------------------------------------------------------------
// 尝试重连
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
// 发送文本串，一次送完
//------------------------------------------------------------------------------
bool CSocketBase::SendMsg(const char* pcMsg, USHORT nTimeOutSec)
{
    //检查连接状态
    if(!m_bIsConnected)
    {
        return false;
    }

    //发送前注册事件
    WSAResetEvent(m_SocketEvent);
    WSAEventSelect(m_socket, m_SocketEvent, FD_WRITE | FD_CLOSE);

    //尝试发送
    int nRet = send(m_socket, pcMsg, strlen(pcMsg), NULL);

    if(nRet == SOCKET_ERROR)
    {
        m_nLastWSAError = WSAGetLastError();

        //遭遇阻塞
        if(m_nLastWSAError == WSAEWOULDBLOCK)
        {
            DWORD dwRet = WSAWaitForMultipleEvents(1, &m_SocketEvent, FALSE, nTimeOutSec*1000, FALSE);

            //如果网络事件发生
            WSANETWORKEVENTS wsaEvents;
            memset(&wsaEvents, 0, sizeof(wsaEvents));

            if(dwRet == WSA_WAIT_EVENT_0)
            {
                WSAResetEvent(m_SocketEvent);
                WSAEnumNetworkEvents(m_socket, m_SocketEvent, &wsaEvents);

                //如果发送可以进行并且没有错误发生
                if((wsaEvents.lNetworkEvents & FD_WRITE) &&
                    (wsaEvents.iErrorCode[FD_WRITE_BIT] == 0))
                {
                    //再次发送文本
                    nRet = send(m_socket, pcMsg, strlen(pcMsg), NULL);

                    if(nRet > 0)
                    {
                        //如果发送字节大于0，表明发送成功
                        return true;
                    }
                }
            }        
        }
    }
    else
    {
        //第一次便发送成功
        return true;
    }

    //如果上述发送失败
    m_nLastWSAError = WSAGetLastError();
    return false;
}

//------------------------------------------------------------------------------
// 发送缓冲区
//------------------------------------------------------------------------------
bool CSocketBase::SendBuffer(char* pBuffer, UINT uiBufferSize, USHORT nTimeOutSec)
{
    //检查连接状态
    if(!m_bIsConnected)
    {
        return false;
    }

    //发送前注册事件
    WSAResetEvent(m_SocketEvent);
    WSAEventSelect(m_socket, m_SocketEvent, FD_WRITE | FD_CLOSE);

    //发送量计数
    UINT nSent = 0;

    //总发送次数，为发送次数定一个限额
    int nSendTimes = 0;
    int nSendLimitTimes = uiBufferSize / 1000;      //假定当前每次发送肯定不少于1K
    UINT uiLeftBuffer = uiBufferSize;               //未发送完的缓存大小

    //发送游标
    char* pcSentPos = pBuffer;

    //直到所有的缓冲都发送完毕
    while(nSent < uiBufferSize)
    {
        //检查发送次数是否超限
        if(nSendTimes > nSendLimitTimes)
        {
            break;
        }

        int nRet = send(m_socket, pcSentPos, uiLeftBuffer, NULL);

        if(nRet == SOCKET_ERROR)
        {
            m_nLastWSAError = WSAGetLastError();

            //遭遇阻塞
            if(m_nLastWSAError == WSAEWOULDBLOCK)
            {
                DWORD dwRet = WSAWaitForMultipleEvents(1, &m_SocketEvent, FALSE, nTimeOutSec*1000, FALSE);

                //如果网络事件发生
                WSANETWORKEVENTS wsaEvents;
                memset(&wsaEvents, 0, sizeof(wsaEvents));

                if(dwRet == WSA_WAIT_EVENT_0)
                {
                    WSAResetEvent(m_SocketEvent);
                    WSAEnumNetworkEvents(m_socket, m_SocketEvent, &wsaEvents);

                    //如果发送可以进行并且没有错误发生
                    if((wsaEvents.lNetworkEvents & FD_WRITE) &&
                        (wsaEvents.iErrorCode[FD_WRITE_BIT] == 0))
                    {
                        //再次发送文本
                        nRet = send(m_socket, pcSentPos, uiLeftBuffer, NULL);

                        if(nRet > 0)
                        {
                            //如果发送字节大于0，表明发送成功
                            nSendTimes++;

                            nSent += nRet;
                            uiLeftBuffer -= nRet;
                            pcSentPos += nRet;
                        }
                        else
                        {
                            m_nLastWSAError = WSAGetLastError();

                            //如果接收到事件发送时也阻塞了，则等待一个周期后重试
                            if(m_nLastWSAError == WSAEWOULDBLOCK)
                            {
                                Sleep(1);        
                            }
                            else
                            {
                                //对于其他错误，直接退出
                                break;
                            }
                        }
                    }
                }
            }
            else
            {
                //遇到阻塞之外的错误，直接退出
                break;
            }
        }
        else
        {
            //发送成功，累加发送量，更新游标
            nSendTimes++;
            
            nSent += nRet;
            uiLeftBuffer -= nRet;
            pcSentPos += nRet;
        }
    }

    //如果发送完成
    if(nSent == uiBufferSize)
    {
        return true;
    }

    //没能成功返回
    m_nLastWSAError = WSAGetLastError();
    return false;
}

//------------------------------------------------------------------------------
// 接受文本串，一次接受
//------------------------------------------------------------------------------
bool CSocketBase::RecvOnce(char* pRecvBuffer, UINT uiBufferSize, UINT& uiRecv, USHORT nTimeOutSec)
{
    //检查连接状态
    if(!m_bIsConnected)
    {
        return false;
    }

    //发送前注册事件
    WSAResetEvent(m_SocketEvent);
    WSAEventSelect(m_socket, m_SocketEvent, FD_READ | FD_CLOSE);

    //尝试接收
    int nRet = recv(m_socket, pRecvBuffer, uiBufferSize, NULL);

    if(nRet == SOCKET_ERROR)
    {
        m_nLastWSAError = WSAGetLastError();

        //遭遇阻塞
        if(m_nLastWSAError == WSAEWOULDBLOCK)
        {
            DWORD dwRet = WSAWaitForMultipleEvents(1, &m_SocketEvent, FALSE, nTimeOutSec*1000, FALSE);

            //如果网络事件发生
            WSANETWORKEVENTS wsaEvents;
            memset(&wsaEvents, 0, sizeof(wsaEvents));

            if(dwRet == WSA_WAIT_EVENT_0)
            {
                WSAResetEvent(m_SocketEvent);
                int nEnum = WSAEnumNetworkEvents(m_socket, m_SocketEvent, &wsaEvents);

                //如果接受可以进行并且没有错误发生
                if((wsaEvents.lNetworkEvents & FD_READ) &&
                    (wsaEvents.iErrorCode[FD_READ_BIT] == 0))
                {
                    //再次接受文本
                    nRet = recv(m_socket, pRecvBuffer, uiBufferSize, NULL);

                    if(nRet > 0)
                    {
                        //如果发送字节大于0，表明发送成功
                        uiRecv = nRet;
                        return true;
                    }
                }
            }        
        }
    }
    else
    {
        //第一次便接收成功
        uiRecv = nRet;
        return true;
    }

    //如果上述接收失败
    m_nLastWSAError = WSAGetLastError();
    return false;
}

//------------------------------------------------------------------------------
// 接收缓冲区，缓冲应该比待接受量要大一点才安全
//------------------------------------------------------------------------------
bool CSocketBase::RecvBuffer(char* pRecvBuffer, UINT uiBufferSize, UINT uiRecvSize, USHORT nTimeOutSec)
{
    //检查连接状态
    if(!m_bIsConnected)
    {
        return false;
    }

    //接收前注册事件
    WSAResetEvent(m_SocketEvent);
    WSAEventSelect(m_socket, m_SocketEvent, FD_READ | FD_CLOSE);

    //接收量计数
    UINT nReceived = 0;

    //总接收次数，为接收次数定一个限额
    int nRecvTimes = 0;
    int nRecvLimitTimes = uiRecvSize / 1000;      //假定当前每次接收肯定不少于1K

    //接收游标
    char* pcRecvPos = pRecvBuffer;

    //直到接收到足够指定量的缓冲
    while(nReceived < uiRecvSize)
    {
        //检查接收次数是否超限
        if(nRecvTimes > nRecvLimitTimes)
        {
            break;
        }

        int nRet = recv(m_socket, pcRecvPos, uiBufferSize, NULL);

        if(nRet == SOCKET_ERROR)
        {
            m_nLastWSAError = WSAGetLastError();

            //遭遇阻塞
            if(m_nLastWSAError == WSAEWOULDBLOCK)
            {
                DWORD dwRet = WSAWaitForMultipleEvents(1, &m_SocketEvent, FALSE, nTimeOutSec*1000, FALSE);

                //如果网络事件发生
                WSANETWORKEVENTS wsaEvents;
                memset(&wsaEvents, 0, sizeof(wsaEvents));

                if(dwRet == WSA_WAIT_EVENT_0)
                {
                    WSAResetEvent(m_SocketEvent);
                    WSAEnumNetworkEvents(m_socket, m_SocketEvent, &wsaEvents);

                    //如果接收可以进行并且没有错误发生
                    if((wsaEvents.lNetworkEvents & FD_READ) &&
                        (wsaEvents.iErrorCode[FD_READ_BIT] == 0))
                    {
                        //再次接收
                        nRet = recv(m_socket, pcRecvPos, uiBufferSize, NULL);

                        if(nRet > 0)
                        {
                            //如果接收字节大于0，表明发送成功
                            nRecvTimes++;

                            nReceived += nRet;
                            uiBufferSize -= nRet;                            
                            pcRecvPos += nRet;
                        }
                        else
                        {
                            m_nLastWSAError = WSAGetLastError();

                            //如果接收到事件发送时也阻塞了，则等待一个周期后重试
                            if(m_nLastWSAError == WSAEWOULDBLOCK)
                            {
                                Sleep(1);        
                            }
                            else
                            {
                                //对于其他错误，直接退出
                                break;
                            }
                        }
                    }
                }
            }
            else
            {
                //遇到阻塞之外的错误，直接退出
                break;
            }
        }
        else
        {
            //接收成功，累加接受量，更新游标
            nRecvTimes++;
            
            nReceived += nRet;
            uiBufferSize -= nRet;
            pcRecvPos += nRet;
        }
    }

    //如果接收完成
    if(nReceived == uiRecvSize)
    {
        return true;
    }

    //没能成功返回
    m_nLastWSAError = WSAGetLastError();
    return false;
}

//------------------------------------------------------------------------------
// 转换网址为IP
//------------------------------------------------------------------------------
bool CSocketBase::ResolveAddressToIp(const char* pcAddress, char* pcIp)
{
	addrinfo adiHints, *padiResult;
	int	nRet;

	memset(&adiHints, 0, sizeof(addrinfo));

	//仅解析IPV4的地址
	adiHints.ai_flags = AI_CANONNAME;
	adiHints.ai_family = AF_INET;
	adiHints.ai_socktype = SOCK_STREAM;
	adiHints.ai_protocol = IPPROTO_TCP;

	//转换地址
	nRet = ::getaddrinfo(pcAddress, NULL, &adiHints, &padiResult);
	
	//检查结果
	if(nRet != 0) 
	{
		freeaddrinfo(padiResult);
		return false;
	}
	
	//拷贝结果,只拷贝第一个
	if(padiResult->ai_addr != NULL)
	{
		::strcpy(pcIp, inet_ntoa(((sockaddr_in*)padiResult->ai_addr)->sin_addr));
	}
	
	freeaddrinfo(padiResult);
	return true;
}