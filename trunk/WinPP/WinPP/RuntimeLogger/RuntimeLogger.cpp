#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include "RuntimeLogger.h"
#include "../ThreadSync/ThreadSync.h"
#include "../ToolFuncs/ToolFuncs.h"

CRuntimeLogger::CRuntimeLogger(const char* pLogName)
{
	//��ȡ��ǰ����
	SYSTEMTIME dwTime;
	GetLocalTime(&dwTime);

	//������־��
	TCHAR pcLogPath[MAX_PATH] = {0};
	wsprintf(pcLogPath, _T("%s_RuntimeLog_%04d_%02d_%02d.txt"), 
		pLogName,
		dwTime.wYear,
		dwTime.wMonth,
		dwTime.wDay);

	//��ȡȫ·��
	memset(m_pcLogFullPath, 0, MAX_PATH);
	GetMoudleFilePath(pcLogPath, m_pcLogFullPath, MAX_PATH);
	
	//�����ļ�
	_tfopen_s(&m_pLogFile, m_pcLogFullPath, _T("at"));

	//����������־
	WriteLog(NULL, NULL, NULL, "Module Attached");
}

CRuntimeLogger::~CRuntimeLogger(void)
{
	//���ɽ�����־
	WriteLog(NULL, NULL, NULL, "Module Released");

	//�ر��ļ�
	fclose(m_pLogFile);
}

//д����־
void CRuntimeLogger::WriteLog(const char* pcOpterator, const char* pcBaseURL, int nRetCode, const char* pLog)
{
	//д��־
	SYSTEMTIME dwTime;
	GetLocalTime(&dwTime);

	char* pcTempBuff = (char*)_blocknew_ne(5000);
	if(!pcTempBuff) return;

	wsprintfA(pcTempBuff, "[%04d-%02d-%02d %02d:%02d:%02d.%03d][THREAD %08d] # %s, %s %d %s \n",
		dwTime.wYear, dwTime.wMonth, dwTime.wDay, dwTime.wHour,dwTime.wMinute,dwTime.wSecond, dwTime.wMilliseconds,
		GetCurrentThreadId(),
		pcOpterator, pcBaseURL, nRetCode, pLog);

	//д���ļ�
	fputs(pcTempBuff, m_pLogFile);
	fflush(m_pLogFile);

	delete pcTempBuff;
}
