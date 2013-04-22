#pragma once

//日志类
class CRuntimeLogger
{
public:
	CRuntimeLogger(const char* pLogName);
	~CRuntimeLogger(void);

public:
	void WriteLog(const char* pcOpterator, const char* pcBaseURL, int nRetCode, const char* pLog);		//写日志

private:
	FILE* m_pLogFile;						//文件指针
	TCHAR m_pcLogFullPath[MAX_PATH];		//文件路径
};
