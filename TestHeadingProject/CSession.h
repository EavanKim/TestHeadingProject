#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <ws2ipdef.h>

#pragma comment(lib, "ws2_32.lib")
#include <Windows.h>

class CSession
{
public:
	CSession( SOCKET _session );
	virtual ~CSession();

	// 상속객체가 덮어쓰는 함수
	// CSession 자체는 그냥 Interface화 하는게 나을지도
	virtual void Process();
	virtual void ErrorProcess();
	virtual void ZeroReceiveProcess();

	bool IsConnected();

	void TerminateConnection();

private:
	void PrintTimInfo();

	time_t m_startTime = time( NULL );
	time_t m_currentTime = time( NULL );
	SOCKET m_session = INVALID_SOCKET;
	uint64_t m_processCounter = 0;
	uint64_t m_zeroRecvCount = 0;
	uint64_t m_currentSIze = 0;
	uint64_t m_bufferSIze = 1 << 13;
	char m_RecvBuffer[ 1 << 13 ];
};

