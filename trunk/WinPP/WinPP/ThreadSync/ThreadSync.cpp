#include <windows.h>

#include "ThreadSync.h"

ShareCriticalSection* ShareCriticalSection::m_pInstance = NULL;

void* _blocknew_ne_wait(size_t sz, int nWaitSec)
{
	void* pBuffer = NULL;

	UINT uiAllocCount = 0;
	while(!pBuffer)
	{
		try
		{
			pBuffer = new BYTE[sz];
		}
		catch(...)
		{
			uiAllocCount++;
			Sleep(10);
		}

		//在两秒内反复申请
		if(uiAllocCount >= (nWaitSec*100))
		{
			break;
		}
	}

	return pBuffer;
}

void* _blocknew_ne(size_t sz)
{
	return _blocknew_ne_wait(sz, 0);
}

