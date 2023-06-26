#pragma once

class CSession
{
public:
	CSession( SOCKET _session, PrintLog* _output = NULL );
	virtual ~CSession();

	// 상속객체가 덮어쓰는 함수
	// CSession 자체는 그냥 Interface화 하는게 나을지도
	virtual void Process();
	virtual void ErrorProcess();
	virtual void ZeroReceiveProcess();

	bool IsConnected();

	void TerminateConnection();

	void TerminateSession();

	static int ThreadProcess( void* _ptr );

	static void SelectRegister( std::unordered_map<SOCKET, CSession*>& _map, fd_set& _set, CSession* _session );
	static void SelectUnregister( std::unordered_map<SOCKET, CSession*>& _map, fd_set& _set, CSession* _session );

	void Work();
	void EventPulse();
	template<typename ... Args>
	void PrintWork( const std::string& format, Args ... args )
	{
		if( m_printOut )
		{
			m_printOut->InsertLog(format, args ...);
		}
		else
		{
			printf(format.c_str(), args ...);
		}
	}

private:
	void PrintTimInfo();

	volatile LONG64 m_threadAlive = 1;
	volatile LONG64 m_threadJobType = 0;
	HANDLE m_event = INVALID_HANDLE_VALUE;
	std::thread* m_sessionThread = nullptr;
	PrintLog* m_printOut = nullptr;
	time_t m_startTime = time( NULL );
	time_t m_currentTime = time( NULL );
	SOCKET m_session = INVALID_SOCKET;
	uint64_t m_processCounter = 0;
	uint64_t m_zeroRecvCount = 0;
	uint64_t m_currentSIze = 0;
	uint64_t m_bufferSIze = 1 << 13;
	char m_RecvBuffer[ 1 << 13 ];
};

