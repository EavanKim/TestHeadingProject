#include "psudoPCH.H"

std::atomic<bool> CServer_App::m_live = true;
WSADATA CServer_App::m_data = {};
Heading::CAcceptThread* CServer_App::m_accept = nullptr;

void CServer_App::InitializeApplication( )
{
	std::string str;
	Heading::WSAErrorString( WSAStartup( MAKEWORD( 2, 2 ), &m_data ), str );
	EventManager::init();
	EventManager::get()->Log(str);
}

void CServer_App::ListenBinding( )
{
	// Thread 만들면서 바로 등록.
	Heading::AcceptThreadInfo* info = new Heading::AcceptThreadInfo();

	info->port = 50000;
	info->liveChecker = ServiceCheck;
	info->onAccept = EventManager::onAccept;

	// info는 안에서 잘 지우는 방향으로 처리.
	// 지역성 있는 메모리는 스택 현황과 정리 상황에 따라 위험하므로 힙을 사용합니다.
	m_accept = new Heading::CAcceptThread( info );
}

void CServer_App::SocketSelecting( )
{
	EventManager* evtMgr = EventManager::get();
	if( nullptr != evtMgr )
	{
		evtMgr->SetAcceptSession();

		if( 0 != evtMgr->GetEventSize( ) )
		{
			DWORD result = WSAWaitForMultipleEvents( evtMgr->GetEventSize( ), evtMgr->GetEventArray( ), FALSE, 0, FALSE );
			switch( Heading::WaitObjectCheck( result ) )
			{
				// 나머지 에러 극복도 구현 해 보기
			case Heading::E_WaitEvent_Result::E_Wait_OK:
				evtMgr->onSelect( result );
				return;
			}
		}
	}
}

void CServer_App::EndProcessing( )
{
	EventManager* mgr = EventManager::get();
	WSASetEvent( mgr->GetEventArray()[0] );
	if(m_accept->joinable() )
		m_accept->join();
	delete m_accept;
	EventManager::get()->Destroy();
	WSACleanup();
}

bool CServer_App::ServiceCheck( )
{
	return m_live;
}

void CServer_App::ServerStop( )
{
	m_live = false;
}

void CServer_App::SystemMessageProcessing( )
{
	MSG msg;
	if( PeekMessageA( &msg, NULL, NULL, NULL, PM_REMOVE ) )
	{
		switch( msg.message )
		{
			case WM_QUIT:
				{
					EndProcessing();
				}
				break;
		}
	}
}
