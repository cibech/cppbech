#pragma once

//获取指定DLL的完整路径，该DLL应该与本DLL同目录
BOOL GetExeFileSubDirectory(TCHAR* pCurDirectory, int nBufSize, const TCHAR* pctSubDir);
BOOL GetCurrentFileVersion(CHAR* pcVersion);
BOOL GetMoudleFilePath(const TCHAR* pcModuleName, TCHAR* pcPath, int nSize);
BOOL RemoveFileByPath(const TCHAR* pcFilePath);

//生成GUID
void GenGUID(char* pDest);
void GenGUIDWithSlash(char* pDest);
void GenVerifyFromSeed(char pcSeed[40], char pcVerify[40]);
BOOL GetSha1HashFromFilePath(const wchar_t* pwcFilePath, char pcHash[40]);
LONG GetFileSizeFromPath(const wchar_t* pwcFilePath);
INT CompareVersionString(const char* pcFirst, const char* pcSecond);
BOOL TerminateProcessByName(const char* pcProcessName);
BOOL TerminateMultipleByName(const char* pcProcessNames);
BOOL IsProcessRunningByName(const char* pcProcessName);
BOOL IsStringEndWith(const wchar_t* pcwstrString, const wchar_t* pcwEnds);