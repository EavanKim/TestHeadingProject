#pragma once

class CChatSession : public Heading::CClientSession
{
public:
	CChatSession( SOCKET _sock );
	virtual ~CChatSession();

	// Inherited via CClientSession
	virtual void Update( ) override;

private:

};