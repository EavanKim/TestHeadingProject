#pragma once

namespace Heading
{
	class CChatSession : public CClientSession
	{
	public:
		CChatSession( SOCKET _sock );
		virtual ~CChatSession();
		
		virtual void Update( ) override;
	};
}

