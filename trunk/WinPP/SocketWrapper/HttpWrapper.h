#ifndef HTTPWRAPPER_H
#define HTTPWRAPPER_H

//定义HTTP头各部字节大小
#define HTW_GET_STRING_SIZE					1024
#define HTW_ACCEPT_STRING_SIZE				32
#define HTW_ACCEPT_LAN_SZIE					32
#define HTW_USERAGENT_SIZE					256
#define HTW_ACCEPT_ENCODING					32
#define HTW_HOST_NAME						256
#define HTW_CONNECTION_TYPE					32
#define HWT_SENT_COOKIE						512

class CHttpWrapper
{
public:
	CHttpWrapper(void);
	~CHttpWrapper(void);

public:
	char m_pcGetString[HTW_GET_STRING_SIZE];
	char m_pcAcceptString[HTW_ACCEPT_STRING_SIZE];
	char m_pcAcceptLanguage[HTW_ACCEPT_LAN_SZIE];
	char m_pcUserAgent[HTW_USERAGENT_SIZE];
	char m_pcAcceptEncoding[HTW_ACCEPT_ENCODING];
	char m_pcHostName[HTW_HOST_NAME];
	char m_pcConnectionType[HTW_CONNECTION_TYPE];
	char m_pcSentCookie[HWT_SENT_COOKIE];

private:
	void SetGetString(const char* pcGetPath)
	{
		::memset(m_pcGetString, 0, HTW_GET_STRING_SIZE);
		::strcpy(m_pcGetString, pcGetPath);
	}

	void SetAcceptType(const char* pcAcceptType)
	{
		::memset(m_pcAcceptString, 0, HTW_ACCEPT_STRING_SIZE);
		::strcpy(m_pcAcceptString, pcAcceptType);	
	}

	void SetAcceptLanguage(const char* pcAcceptLan)
	{
		::memset(m_pcAcceptLanguage, 0, HTW_ACCEPT_LAN_SZIE);
		::strcpy(m_pcAcceptLanguage, pcAcceptLan);	
	}

	void SetUserAgent(const char* pcUserAgent)
	{
		::memset(m_pcUserAgent, 0, HTW_USERAGENT_SIZE);
		::strcpy(m_pcUserAgent, pcUserAgent);
	}

	void SetAcceptEncoding(const char* pcAcceptEncoding)
	{
		::memset(m_pcAcceptEncoding, 0, HTW_ACCEPT_ENCODING);
		::strcpy(m_pcAcceptEncoding, pcAcceptEncoding);	
	}

	void SetHost(const char* pcHost)
	{
		::memset(m_pcHostName, 0, HTW_HOST_NAME);
		::strcpy(m_pcHostName, pcHost);	
	}

	void SetConnectionType(const char* pcConnType)
	{
		::memset(m_pcConnectionType, 0, HTW_CONNECTION_TYPE);
		::strcpy(m_pcConnectionType, pcConnType);		
	}

	void SetSentCookie(const char* pcCookie)
	{
		::memset(m_pcSentCookie, 0, HWT_SENT_COOKIE);
		::strcpy(m_pcSentCookie, pcCookie);
	}

public:
	void MakeRequestByURL(const char* pcURL);
};

#endif