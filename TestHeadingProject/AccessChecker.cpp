#include "psudoPCH.h"

AccessChecker::AccessChecker( Heading::CSimpleSocket* _sock )
	: Heading::CEventBaseSession_v2( _sock )
{
	_sock->setCallback(Heading::E_SOCKET_CALLBACK_RECEIVE, AccessChecker::onRecv, this);
}

AccessChecker::~AccessChecker( )
{
}

bool AccessChecker::isNewClient( )
{
	return m_isReceivedAccessData;
}

CChatSession_v2* AccessChecker::newSession( )
{
	CChatSession_v2* result = new CChatSession_v2( m_sock );

	// 연결 확립된 대상이 삭제되지 않도록 빼내기.
	m_sock = nullptr;

	return result;
}

Heading::CSimpleSocket* AccessChecker::endAccessChecker( )
{
	Heading::CSimpleSocket* result = m_sock;
	m_sock = nullptr;

	return result;
}

void AccessChecker::onRecv( void* _simpleSocket, void* _accessChecker )
{
	Heading::CSimpleSocket* socket = static_cast<Heading::CSimpleSocket*>(_simpleSocket);
	AccessChecker* checker = static_cast<AccessChecker*>(_accessChecker);


}
