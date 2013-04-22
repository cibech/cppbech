#ifndef THREADSYNC_H
#define THREADSYNC_H

class ShareCriticalSection
{
private:
	ShareCriticalSection()
	{
		InitializeCriticalSection(&m_csSec);
	}

	~ShareCriticalSection()
	{
		DeleteCriticalSection(&m_csSec);
	}

	static ShareCriticalSection* m_pInstance;

	CRITICAL_SECTION m_csSec;

public:
	static ShareCriticalSection* Instance()
	{
		if(m_pInstance == NULL)
		{
			m_pInstance = new ShareCriticalSection();
		}

		return m_pInstance;
	}

	static void Release()
	{
		if(m_pInstance)
		{
			delete m_pInstance;
		}
	}

	inline void Enter()
	{
		EnterCriticalSection(&m_csSec);
	}

	inline void Leave()
	{
		LeaveCriticalSection(&m_csSec);
	}
};

class CMyCriticalSection
{
public :
    CMyCriticalSection(void)
    {
    	InitializeCriticalSection(&m_sync);
    }

    ~CMyCriticalSection(void)
    {
		DeleteCriticalSection(&m_sync);
    }

    inline void Enter(void)
    {
    	EnterCriticalSection(&m_sync);
    }

    inline void Leave(void)
    {
    	LeaveCriticalSection(&m_sync);
    }

private:
	CRITICAL_SECTION m_sync;
};

template <class T>
class CMultiThreadSync
{
public:
	
	friend class CThreadSync;

	class CThreadSync
	{
	public:
		CThreadSync(void)
		{
			m_Sync.Enter();
		}

		~CThreadSync(void)
		{
			m_Sync.Leave();
		}
	};

protected:
    static CMyCriticalSection m_Sync;
};

template <class T>
CMyCriticalSection CMultiThreadSync<T>::m_Sync;

#endif
