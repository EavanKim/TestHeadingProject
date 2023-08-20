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
}

CServer_App_v2::CServer_App_v2( )
{
}

CServer_App_v2::~CServer_App_v2( )
{
}
