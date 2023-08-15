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
	// Thread ¸¸µé¸é¼­ ¹Ù·Î µî·Ï.
	Heading::AcceptThreadInfo* info = new Heading::AcceptThreadInfo();

	info->port = 50000;
	info->liveChecker = ServiceCheck;
	info->onAccept = EventManager::onAccept;

	// info´Â ¾È¿¡¼­ Àß Áö¿ì´Â ¹æÇâÀ¸·Î Ã³¸®.
	// Áö¿ª¼º ÀÖ´Â ¸Þ¸ð¸®´Â ½ºÅÃ ÇöÈ²°ú Á¤¸® »óÈ²¿¡ µû¶ó À§ÇèÇÏ¹Ç·Î ÈüÀ» »ç¿ëÇÕ´Ï´Ù.
	m_accept = new Heading::CAcceptThread( info );
}

void CServer_App::SocketSelecting( )
{
	// IO ì‹ í˜¸ë§Œ ë“¤ì–´ì˜¤ëŠ” ìœ„ì¹˜
	EventManager* evtMgr = EventManager::get();
	if( nullptr != evtMgr )
	{
		evtMgr->SetAcceptSession();

		if( 0 != evtMgr->GetEventSize( ) )
		{
			DWORD result = WSAWaitForMultipleEvents( evtMgr->GetEventSize( ), evtMgr->GetEventArray( ), FALSE, 0, FALSE );
			switch( Heading::WaitObjectCheck( result ) )
			{
			case Heading::E_WaitEvent_Result::E_Wait_Reset_EVENTS_ARRAY:
				evtMgr->Recreate_EventInfo( );
				break;
			// ³ª¸ÓÁö ¿¡·¯ ±Øº¹µµ ±¸Çö ÇØ º¸±â
			case Heading::E_WaitEvent_Result::E_Wait_OK:
				evtMgr->onSelect( result );
				break;
			case Heading::E_WaitEvent_Result::E_Wait_Delayed:
				// ÀÛ¾÷ ´ë±âÁß ·Î±×ÀÌ¹Ç·Î Áö³ª°©´Ï´Ù.
				break;
			}
		}

		evtMgr->FlushSend(); // Àü¼Û ¿À·ù·Î ½ÇÆÐÇÑ °Í Àç½Ãµµ À§Ä¡.
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
