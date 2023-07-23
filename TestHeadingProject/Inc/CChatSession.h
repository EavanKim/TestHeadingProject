#pragma once

class CChatSession : public Heading::CClientSession
{
public:
	CChatSession( SOCKET _sock );
	virtual ~CChatSession( );

	virtual void Update( ) override;
};


