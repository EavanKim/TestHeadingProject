#include "psudoPCH.h"

TestServer_Chat::TestServer_Chat( uint16_t _startPort )
{
	m_proxy = new CSocket_Listen( 0, _startPort );
	m_broadCast = new CSocket_Listen( 1, _startPort + 1000 );
}

TestServer_Chat::~TestServer_Chat()
{
	if( nullptr != m_proxy )
	{
		delete m_proxy;
		m_proxy = nullptr;
	}
	if( nullptr != m_broadCast )
	{
		delete m_broadCast;
		m_broadCast = nullptr;
	}
}

void TestServer_Chat::Ready()
{
	if( nullptr != m_proxy )
		if( !m_proxy->C_Connect() )
			throw "InitializeFailure!! TestServer_Chat::Ready()";
	if( nullptr != m_broadCast )
		if( !m_broadCast->C_Connect() )
			throw "InitializeFailure!! TestServer_Chat::Ready()";
}

void TestServer_Chat::Update()
{
	std::vector<CMessage> recvMessages;
	if( nullptr != m_proxy )
	{
		m_proxy->Select( recvMessages );
	}
	if( nullptr != m_broadCast )
	{
		m_broadCast->Select( recvMessages );
	}

	std::sort( recvMessages.begin(), recvMessages.end() );

	for( CMessage& message : recvMessages )
	{
		m_broadCast->BroadCasting( &message );
		delete message.m_message;
	}

	recvMessages.clear();
}
