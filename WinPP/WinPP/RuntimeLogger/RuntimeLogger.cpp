#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include "RuntimeLogger.h"
#include "../ThreadSync/ThreadSync.h"
#include "../ToolFuncs/ToolFuncs.h"

CRuntimeLogger::CRuntimeLogger(const char* pLogName)
{
	//获取当前日期
	SYSTEMTIME dwTime;
	GetLocalTime(&dwTime);

	//生成日志名
	TCHAR pcLogPath[MAX_PATH] = {0};
	wsprintf(pcLogPath, _T("%s_RuntimeLog_%04d_%02d_%02d.txt"), 
		pLogName,
		dwTime.wYear,
		dwTime.wMonth,
		dwTime.wDay);

	//获取全路径
	memset(m_pcLogFullPath, 0, MAX_PATH);
	GetMoudleFilePath(pcLogPath, m_pcLogFullPath, MAX_PATH);
	
	//生成文件
	_tfopen_s(&m_pLogFile, m_pcLogFullPath, _T("at"));

	//生成启动日志
	WriteLog(NULL, NULL, NULL, "Module Attached");
}

CRuntimeLogger::~CRuntimeLogger(void)
{
	//生成结束日志
	WriteLog(NULL, NULL, NULL, "Module Released");

	//关闭文件
	fclose(m_pLogFile);
}

//写入日志
void CRuntimeLogger::WriteLog(const char* pcOpterator, const char* pcBaseURL, int nRetCode, const char* pLog)
{
	//写日志
	SYSTEMTIME dwTime;
	GetLocalTime(&dwTime);

	char* pcTempBuff = (char*)_blocknew_ne(5000);
	if(!pcTempBuff) return;

	wsprintfA(pcTempBuff, "[%04d-%02d-%02d %02d:%02d:%02d.%03d][THREAD %08d] # %s, %s %d %s \n",
		dwTime.wYear, dwTime.wMonth, dwTime.wDay, dwTime.wHour,dwTime.wMinute,dwTime.wSecond, dwTime.wMilliseconds,
		GetCurrentThreadId(),
		pcOpterator, pcBaseURL, nRetCode, pLog);

	//写入文件
	fputs(pcTempBuff, m_pLogFile);
	fflush(m_pLogFile);

	delete pcTempBuff;
}
