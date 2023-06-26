#include "psudoPCH.h"

//extern void PrintMem( char* _ptr, uint64_t _length );

CSession::CSession( SOCKET _session, PrintLog* _output )
	: m_session( _session )
	, m_printOut( _output )
{
	m_currentSIze = 0;
	ZeroMemory( m_RecvBuffer, sizeof( m_RecvBuffer ) );
	m_event = CreateEvent( NULL, FALSE, FALSE, NULL );

	// 당장은 PrintLog가 없으면
	// 동기 처리이므로 PrintLog로 분기합니다.
	if( nullptr != _output )
		m_sessionThread = new std::thread( CSession::ThreadProcess, this );
}

CSession::~CSession()
{
	TerminateConnection();
}

void CSession::Process()
{
	uint64_t seek = 0;
	int readcount = recv(m_session, m_RecvBuffer + m_currentSIze, m_bufferSIze - m_currentSIze, 0 );
	if( -1 == readcount )
	{
		ErrorProcess();
		return;
	}
	if( 0 == readcount )
	{
		ZeroReceiveProcess();
		return;
	}

	m_currentSIze = m_currentSIze + readcount;

	if( m_currentSIze < sizeof( Header ) )
		return;

	while( 0 != m_currentSIze )
	{
		if( m_currentSIze < sizeof( Header ) )
			break;

		char* currPtr = m_RecvBuffer + seek;
		uint64_t type = 0;
		Header* m_header = ( Header* )currPtr;

		switch( m_header->type )
		{
			case 2:
				TerminateConnection();
				break;
			case 100:
			{
				++m_processCounter;
				if( m_currentSIze < sizeof( TestBuffer ) )
					break;

				TestBuffer* parseData = ( TestBuffer* )currPtr;

				seek = seek + parseData->length;
				m_currentSIze = m_currentSIze - parseData->length;

				PrintTimInfo();
				PrintWork( "[Count : %lld] %s \n", m_processCounter, parseData->buffer );
			}
			break;
			default:
				Util::PrintMem(currPtr, sizeof( Header ) );
				PrintWork( "!!! Packet Parsing Failure !!! [type : %lld] \n", m_header->type );
				break;
		}
	}

	PrintWork("BufferSize %lld \n", m_currentSIze );
	if( 0 != m_currentSIze )
	{
		memcpy(m_RecvBuffer, m_RecvBuffer + seek, m_currentSIze );
	}
}

void CSession::ErrorProcess()
{
	TerminateConnection();
}

void CSession::ZeroReceiveProcess()
{
	++m_zeroRecvCount;

	if( 1000 > m_zeroRecvCount )
	{
		TerminateConnection();
	}
}

bool CSession::IsConnected()
{
	if( INVALID_SOCKET == m_session )
	{
		return false;
	}

	if( SOCKET_ERROR == m_session )
	{
		return false;
	}

	return true;
}

void CSession::TerminateConnection()
{
	if( INVALID_SOCKET != m_session )
	{
		closesocket( m_session );
		m_session = INVALID_SOCKET;
	}

	// 여기서 관리하는 생명주기가 아니므로 NULL 처리만 합니다.
	m_printOut = NULL;
}

void CSession::TerminateSession()
{
	// Abort 되면 어디서 종료될 지 모르므로
	// 테스트에선 낭비 같더라도 join시킵니다.
	// 무조건 모든 진도를 끝까지 진행합니다.
	InterlockedExchange64( &m_threadAlive, 0 );

	// Thread가 Event 대기중일테니
	// Alive 0으로 event 진행시킵니다.
	SetEvent( m_event );

	// 정상적으로 진행되면 joinable 대기에서 데드락이 걸리지 않습니다.
	if( (NULL != m_sessionThread ) && m_sessionThread->joinable() )
	{
		m_sessionThread->join();
		delete m_sessionThread;
		m_sessionThread = nullptr;
	}

	if( INVALID_HANDLE_VALUE )
	{
		CloseHandle( m_event );
		m_event = INVALID_HANDLE_VALUE;
	}
}

int CSession::ThreadProcess( void* _ptr )
{
	CSession* session = (CSession*)_ptr;

	// 기본적으로 세팅이 되었더라도, 첫 Recv를 받기 전일테니
	// 무조건 Thread 준비 후 1회 대기한 다음
	// Select 결과로 Event에 Set이 들어오면 지나갑니다.
	WaitForSingleObject( session->m_event, INFINITE );

	// while 체크에 들어가기 전에
	// 이 Thread Process에서 사용하는 session의 m_threadAlive의 값이 1이라면
	// return 에서 함수 진입 전 값인 1이 나옵니다.
	// 혹시 0으로 바꾼 다음 while 체크되면 루프를 끝냅니다.
	while( InterlockedCompareExchange64( &session->m_threadAlive, 0, 0 ) )
	{
		session->Process();

		// 여기서 대기하다가,
		// 잠든 Thread를 깨운다음, m_threadAlive 체크를 했을 때 0이면
		// while을 탈출하고 종료해서
		// joinable 체크를 false로 만듭니다.
		// 살아나는게 늦거나 우선권 문제가 생기더라도
		// joinable이 끝날테니 join 대기할 수 있습니다.
		WaitForSingleObject( session->m_event, INFINITE );
	}

	return 0;
}

void CSession::SelectRegister( std::unordered_map<SOCKET, CSession*>& _map, fd_set& _set, CSession* _session )
{

	std::unordered_map<SOCKET, CSession*>::const_iterator iter = _map.find( _session->m_session );
	if( _map.end() == iter )
	{
		_map.insert( std::make_pair( _session->m_session, _session ) );
		// SOCKET이 Private니 fd_set을 가져와서 등록합니다.
		FD_SET( _session->m_session, &_set );
	}
	else
	{
		//여기 진입했다면 문제가 있습니다.
		// 일단 fd_set에서도 삭제시키고
		// Session을 닫습니다.
		FD_CLR( _session->m_session, &_set );
		delete _session;
	}
}

void CSession::SelectUnregister( std::unordered_map<SOCKET, CSession*>& _map, fd_set& _set, CSession* _session )
{
	FD_CLR( _session->m_session, &_set );
	std::unordered_map<SOCKET, CSession*>::const_iterator iter = _map.find( _session->m_session );

	if( _map.end() != iter )
	{
		_map.erase( iter );
	}

	delete _session;
}

void CSession::Work()
{
	if( m_printOut )
	{
		EventPulse();
	}
	else
	{
		Process();
	}
}

void CSession::EventPulse()
{
	SetEvent( m_event );
}

void CSession::PrintTimInfo()
{
	m_currentTime = time(NULL);
	uint64_t SessionTime = m_currentTime - m_startTime;
	uint64_t MPS = m_processCounter / ( 0 == SessionTime ? 1 : SessionTime);
	PrintWork("[%lld][MPS : %lld] ", SessionTime, MPS );
}

