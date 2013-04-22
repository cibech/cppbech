#pragma once

//��ȡָ��DLL������·������DLLӦ���뱾DLLͬĿ¼
BOOL GetExeFileSubDirectory(TCHAR* pCurDirectory, int nBufSize, const TCHAR* pctSubDir);
BOOL GetCurrentFileVersion(CHAR* pcVersion);
BOOL GetMoudleFilePath(const TCHAR* pcModuleName, TCHAR* pcPath, int nSize);
BOOL RemoveFileByPath(const TCHAR* pcFilePath);

//����GUID
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