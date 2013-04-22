#include <objbase.h>
#pragma comment(lib, "ole32.lib")

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#pragma comment( lib, "version.lib")

#include <tchar.h>
#include <AtlConv.h>
#include <process.h>
#include <Tlhelp32.h>

#include "ToolFuncs.h"
#include <string>
#include <vector>

#include "../../../thirdpart/hashtool/hashlibpp.h"

#ifdef _DEBUG
#pragma comment(lib, "../../thirdpart/hashtool/Debug/hashtool.lib")
#else
#pragma comment(lib, "../../thirdpart/hashtool/Release/hashtool.lib")
#endif

BOOL GetCurrentFileVersion(CHAR* pcVersion)
{
	TCHAR pCurDirectory[MAX_PATH] = {0};
	int nRet = ::GetModuleFileName(NULL, pCurDirectory, MAX_PATH);

	if(nRet == 0)
	{
		return FALSE;
	}

	DWORD dwHandle = NULL;
	DWORD dwFileVersionInfoSize = GetFileVersionInfoSize(pCurDirectory, &dwHandle);

	if (NULL == dwFileVersionInfoSize)
		return FALSE;

	// Allocate space to retrieve version information using vector to prevent
	// memory leaks
	std::vector<BYTE> pData(dwFileVersionInfoSize);

	// Retrieves version information for the specified file.
	if (false == GetFileVersionInfo(pCurDirectory
		, dwHandle
		, dwFileVersionInfoSize
		, static_cast<LPVOID>(&pData[0])))
		return FALSE; // Can't retrieve version information.

	// The memory of ptFileInfo is freed when pData is freed.
	VS_FIXEDFILEINFO *ptFileInfo;
	UINT	uintSize;

	// Retrieves version information from the version-information resource
	if (false == VerQueryValue(static_cast<LPVOID>(&pData[0])
		, _T("\\")
		, reinterpret_cast<LPVOID*>(&ptFileInfo)
		, &uintSize))
		return FALSE; // Can't retrieve version information

	USHORT musMajorVersion, musMinorVersion, musReleaseNumber, musBuildNumber;
	// Resolve major, minor, release and build number.
	musMajorVersion = static_cast<unsigned short>(  
		( ptFileInfo->dwFileVersionMS >> 16 ) &0xffff );

	musMinorVersion	= static_cast<unsigned short>( 
		ptFileInfo->dwFileVersionMS &0xffff );

	musReleaseNumber = static_cast<unsigned short>( 
		( ptFileInfo->dwFileVersionLS >> 16 ) &0xffff);

	musBuildNumber = static_cast<unsigned short>( 
		ptFileInfo->dwFileVersionLS & 0xffff);

	wsprintfA(pcVersion, "%d.%d.%d.%d", musMajorVersion, musMinorVersion, musReleaseNumber, musBuildNumber);

	return true;
}

BOOL GetExeFileSubDirectory(TCHAR* pCurDirectory, int nBufSize, const TCHAR* pctSubDir)
{
	int nRet = ::GetModuleFileName(NULL, pCurDirectory, nBufSize);

	if(nRet == 0)
	{
		return FALSE;
	}

	//从路径中消去文件名
	for(int i= nBufSize-1; i > 2; i--)
	{
		if(pCurDirectory[i] != _T('\\'))
		{
			pCurDirectory[i] = _T('\0');
		}
		else
		{
			break;
		}
	}

	//检查是否存在子目录，不存在则创建
	_tcscat(pCurDirectory, pctSubDir);
	if(!::PathIsDirectory(pCurDirectory))
	{
		::CreateDirectory(pCurDirectory, NULL);
	}

	return TRUE;
}

//获取指定DLL的完整路径，该DLL应该与本DLL同目录
BOOL GetMoudleFilePath(const TCHAR* pcModuleName, TCHAR* pcPath, int nSize)
{
	int nRet = ::GetModuleFileName(NULL, pcPath, nSize);

	if(nRet == 0)
	{
		return FALSE;
	}

	//从路径中消去文件名
	for(int i= nSize-1; i > 2; i--)
	{
		if(pcPath[i] != _T('\\'))
		{
			pcPath[i] = _T('\0');
		}
		else
		{
			break;
		}
	}

	//检查是否存在Log目录，不存在则创建
	_tcscat(pcPath, _T("Log"));
	if(!::PathIsDirectory(pcPath))
	{
		::CreateDirectory(pcPath, NULL);
	}

	//加上文件名
	_tcscat(pcPath, _T("\\"));
	_tcscat(pcPath, pcModuleName);

	return TRUE;
}

//删除文件
BOOL RemoveFileByPath(const TCHAR* pcFilePath)
{
	int nRet = _tremove(pcFilePath);

	if(nRet == 0)
	{
		return TRUE;
	}

	return FALSE;
}

//生成GUID
void GenGUIDWithSlash(char* pDest) 
{ 
	GUID guid; 
	if (S_OK == ::CoCreateGuid(&guid)) 
	{ 
		wsprintfA(pDest, "uuid:%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x" 
			, guid.Data1 
			, guid.Data2 
			, guid.Data3 
			, guid.Data4[0], guid.Data4[1] 
		, guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5] 
		, guid.Data4[6], guid.Data4[7] 
		); 
	}
}

//生成GUID
void GenGUID(char* pDest) 
{ 
	GUID guid; 
	if (S_OK == ::CoCreateGuid(&guid)) 
	{ 
		wsprintfA(pDest, "uuid:%08x%04x%04x%02x%02x%02x%02x%02x%02x%02x%02x" 
			, guid.Data1 
			, guid.Data2 
			, guid.Data3 
			, guid.Data4[0], guid.Data4[1] 
		, guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5] 
		, guid.Data4[6], guid.Data4[7] 
		); 
	}
}

void GenVerifyFromSeed(char pcSeed[40], char pcVerify[40])
{
	hashwrapper* sha1 = new sha1wrapper();	
	hashwrapper* md5 = new md5wrapper();

	std::string hash = sha1->getHashFromString(pcSeed);
	UCHAR ucCount = (UCHAR)pcSeed[39];
	ucCount %= 4;
	for(int i=0; i<=ucCount; i++)
	{
		hash = md5->getHashFromString(hash);
	}

	hash = sha1->getHashFromString(hash);
	memset(pcVerify, 0, 40);
	memcpy(pcVerify, hash.c_str(), 40);

	delete sha1;
	delete md5;
}

INT CompareVersionString(const char* pcFirst, const char* pcSecond)
{
	int nVerFirst[4] = {0};
	int nVerSec[4] = {0};

	//参数检查
	if(pcFirst[0] == '\0')
	{
		return -1;
	}
	else if(pcSecond[0] == '\0')
	{
		return 1;
	}

	sscanf(pcFirst, "%d.%d.%d.%d", &nVerFirst[0], &nVerFirst[1], &nVerFirst[2], &nVerFirst[3]);
	sscanf(pcSecond, "%d.%d.%d.%d", &nVerSec[0], &nVerSec[1], &nVerSec[2], &nVerSec[3]);

	for(int i=0; i<4; i++)
	{
		if(nVerFirst[i] > nVerSec[i])
		{
			return 1;
		}
		else if(nVerFirst[i] < nVerSec[i])
		{
			return -1;
		}
	}

	return 0;
}

LONG GetFileSizeFromPath(const wchar_t* pwcFilePath)
{
	FILE* pfile = _wfopen(pwcFilePath, L"rb");

	if(!pfile) return 0;
	
	fseek(pfile, 0, SEEK_END);
	
	LONG lRet = ftell(pfile);
	
	fclose(pfile);

	return lRet;
}

BOOL GetSha1HashFromFilePath(const wchar_t* pwcFilePath, char pcHash[40])
{
	hashwrapper* sha1 = new sha1wrapper();
	BOOL bRet = FALSE;

	try
	{
		USES_CONVERSION;
		std::string hash = sha1->getHashFromFile(W2A(pwcFilePath));

		if(hash.length() == 40)
		{
			bRet = TRUE;
			memcpy(pcHash, hash.c_str(), 40);
		}
	}
	catch(hlException e)
	{
		bRet = FALSE;
	}

	delete sha1;
	return bRet;
}

//结束多个进程
BOOL TerminateMultipleByName(const char* pcProcessNames)
{
	if(strlen(pcProcessNames) <= 0)
	{
		return FALSE;
	}

	char* pcSeps = ",;| ";
	char pcTemp[64] = {0};
	strcpy(pcTemp, pcProcessNames);

	char* pCurPos = strtok(pcTemp, pcSeps);
	
	while(pCurPos)
	{
		TerminateProcessByName(pCurPos);
		pCurPos = strtok(NULL, pcSeps);
	}

	return TRUE;
}

//结束单个进程
BOOL TerminateProcessByName(const char* pcProcessNameA)
{
	if(strlen(pcProcessNameA) <= 0)
	{
		return FALSE;
	}

	BOOL bFound = FALSE;
	BOOL bSuccess = TRUE;
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	USES_CONVERSION;

	TCHAR pcProcessName[64] = {0};
	_tcscpy(pcProcessName, A2T(pcProcessNameA));

	HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if(Process32First(hSnapshot, &entry) == TRUE)
	{
		while(Process32Next(hSnapshot, &entry) == TRUE)
		{
			if(_tcsicmp(entry.szExeFile, pcProcessName) == 0)
			{  
				bFound = TRUE;

				HANDLE hProcess = ::OpenProcess(PROCESS_TERMINATE, FALSE, entry.th32ProcessID);

				BOOL bRet = ::TerminateProcess(hProcess, 0);

				//尝试终结进程，如果失败，等待5s后重试
				if(!bRet)
				{
					::Sleep(5000);
					bRet = ::TerminateProcess(hProcess, 0);

					bSuccess = bRet;
				}

				::CloseHandle(hProcess);
				break;
			}
		}
	}

	CloseHandle(hSnapshot);

	return bSuccess;
}

BOOL IsProcessRunningByName(const char* pcProcessNameA)
{
	BOOL bFound = FALSE;
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	USES_CONVERSION;

	TCHAR pcProcessName[64] = {0};
	_tcscpy(pcProcessName, A2T(pcProcessNameA));

	HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if(Process32First(hSnapshot, &entry) == TRUE)
	{
		while(Process32Next(hSnapshot, &entry) == TRUE)
		{
			if(_tcsicmp(entry.szExeFile, pcProcessName) == 0)
			{  
				bFound = TRUE;
				break;
			}
		}
	}

	CloseHandle(hSnapshot);

	return bFound;
}

BOOL IsStringEndWith(const wchar_t* pcwstrString, const wchar_t* pcwEnds)
{
	int nStringLen = wcslen(pcwstrString);
	int nEndsLen = wcslen(pcwEnds);

	if(nStringLen <= nEndsLen) return FALSE;

	if(_wcsicmp(&pcwstrString[nStringLen-nEndsLen], pcwEnds) == 0)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}