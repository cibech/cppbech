#pragma once

//��־��
class CRuntimeLogger
{
public:
	CRuntimeLogger(const char* pLogName);
	~CRuntimeLogger(void);

public:
	void WriteLog(const char* pcOpterator, const char* pcBaseURL, int nRetCode, const char* pLog);		//д��־

private:
	FILE* m_pLogFile;						//�ļ�ָ��
	TCHAR m_pcLogFullPath[MAX_PATH];		//�ļ�·��
};
