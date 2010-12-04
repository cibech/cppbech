#include "stdafx.h"

#include "SocketBase.h"

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	WSADATA wsaData;
	::WSAStartup(MAKEWORD(2, 2), &wsaData);

	char myip[20] = {0};

	char buffRecv[128*1024] = { 0 };
	char buffMsg[1024] = {0};
	UINT uiRecved;
	
	CSocketBase::ResolveAddressToIp("www.baidu.com", myip);

	CSocketBase mySocket;
	bool bRet = mySocket.ConnectRemote(myip, 80);

	char msgSend[100] = "GET / HTTP/1.1\r\n\r\n";
	
	mySocket.SendMsg(msgSend);

	bRet = mySocket.RecvOnce(buffMsg, 1024, uiRecved);

	bRet = mySocket.RecvOnce(buffMsg, 1024, uiRecved);


	if(!bRet)
	{
		return -1;
	}

	::WSACleanup();
	
	return 0;
}