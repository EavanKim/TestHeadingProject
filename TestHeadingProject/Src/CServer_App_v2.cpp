#include "psudoPCH.h"

CServer_App_v2* CServer_App_v2::m_instance = nullptr;



void CServer_App_v2::Initialize( )
{
	if( nullptr == m_instance )
	{
		m_instance = new CServer_App_v2();
	}
}

CServer_App_v2* CServer_App_v2::get( )
{
	return m_instance;
}

void CServer_App_v2::dispose( )
{
	if( nullptr != m_instance )
	{
		delete m_instance;
		m_instance = nullptr;
	}
}

void CServer_App_v2::AcceptSelect( )
{

}

void CServer_App_v2::SessionSelect( )
{
	// IO 신호만 들어오는 위치
	EventManager* evtMgr = EventManager::get();
	if ( nullptr != evtMgr )
	{
		int count = 0;
		WSAEVENT* eventPtr = nullptr;
		if ( m_sessions.PreSelect(eventPtr, count) )
		{
			DWORD result = WSAWaitForMultipleEvents(count, eventPtr, FALSE, 0, FALSE);
			switch ( Heading::WaitObjectCheck(result) )
			{
				case Heading::E_WaitEvent_Result::E_Wait_Reset_EVENTS_ARRAY:
					evtMgr->Recreate_EventInfo();
					break;
				case Heading::E_WaitEvent_Result::E_Wait_OK:
					evtMgr->onSelect(result);
					break;
				case Heading::E_WaitEvent_Result::E_Wait_Delayed:
					break;
			}
		}
	}
}

void CServer_App_v2::SendProcess()
{
	m_sessions.Update();
}

CServer_App_v2::CServer_App_v2( )
{

}

CServer_App_v2::~CServer_App_v2( )
{

}
