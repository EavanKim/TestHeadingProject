//#include "psudoPCH.H"
//
//std::atomic<bool> CServer_App::m_live = true;
//WSADATA CServer_App::m_data = {};
//Heading::CAcceptThread* CServer_App::m_accept = nullptr;
//Heading::AcceptThreadInfo* CServer_App::m_acceptInfo = nullptr;
//std::mutex CServer_App::m_exitMutex;
//
//void CServer_App::InitializeApplication( )
//{
//	std::string str;
//	Heading::WSAErrorString( WSAStartup( MAKEWORD( 2, 2 ), &m_data ), str );
//	EventManager::init();
//	EventManager::get()->Log(str);
//}
//
//void CServer_App::ListenBinding( )
//{
//	m_acceptInfo = new Heading::AcceptThreadInfo();
//
//	m_acceptInfo->port = 50000;
//	m_acceptInfo->accepter = new Heading::CAccepter(m_acceptInfo->port);
//	m_acceptInfo->liveChecker = ServiceCheck;
//	m_acceptInfo->onAccept = EventManager::onAccept;
//
//	//m_accept = new Heading::CAcceptThread( m_acceptInfo );
//}
//
//void CServer_App::SocketSelecting( )
//{
//	// IO 신호만 들어오는 위치
//	EventManager* evtMgr = EventManager::get();
//	if( nullptr != evtMgr )
//	{
//		evtMgr->SetAcceptSession();
//
//		if( 0 != evtMgr->GetEventSize( ) )
//		{
//			DWORD result = WSAWaitForMultipleEvents( evtMgr->GetEventSize( ), evtMgr->GetEventArray( ), FALSE, 0, FALSE );
//			switch( Heading::WaitObjectCheck( result ) )
//			{
//			case Heading::E_WaitEvent_Result::E_Wait_Reset_EVENTS_ARRAY:
//				evtMgr->Recreate_EventInfo( );
//				break;
//			case Heading::E_WaitEvent_Result::E_Wait_OK:
//				evtMgr->onSelect( result );
//				break;
//			case Heading::E_WaitEvent_Result::E_Wait_Delayed:
//				break;
//			}
//		}
//
//		evtMgr->FlushSend();
//	}
//}
//
//void CServer_App::EndProcessing( )
//{
//	EventManager* mgr = EventManager::get();
//	if ( NULL != mgr )
//	{
//		WSASetEvent( mgr->GetEventArray()[0] );
//		WSASetEvent( m_acceptInfo->accepter->Get_Event() );
//		//if(m_accept->joinable() )
//		//	m_accept->join();
//		//delete m_accept;
//
//		//m_user.clear();
//
//		delete m_acceptInfo->accepter;
//		delete m_acceptInfo;
//
//		EventManager::get()->Destroy();
//		WSACleanup();
//	}
//}
//
//bool CServer_App::ServiceCheck( )
//{
//	return m_live;
//}
//
//void CServer_App::ServerStop( )
//{
//	m_live = false;
//}
