#include "psudoPCH.h"
#include "CChatSession_v2.h"

CChatSession_v2::CChatSession_v2( Heading::CSimpleSocket* _sock )
	: Heading::CEventBaseSession_v2( _sock )
{
}

CChatSession_v2::~CChatSession_v2( )
{
}

std::string CChatSession_v2::getName( )
{
	return m_nickname;
}
