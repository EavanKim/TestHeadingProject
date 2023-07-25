#include "psudoPCH.H"

std::atomic<bool> CServer_App::m_live = true;
WSADATA CServer_App::m_data = {};

void CServer_App::InitializeApplication( )
{
	std::string str;
	Heading::WSAErrorString( WSAStartup( MAKEWORD( 2, 2 ), &m_data ), str );
	EventManager::init();
}

void CServer_App::ListenBinding( )
{
	EventManager::get()->AddBindPort( 50000 );
}

void CServer_App::ClientAccepting( )
{

}

void CServer_App::SocketSelecting( )
{

}

void CServer_App::PacketProcessing( )
{

}

void CServer_App::EndProcessing( )
{
	EventManager::get()->Dispose();
	WSACleanup();
}

bool CServer_App::ServiceCheck( )
{
	return m_live;
}
