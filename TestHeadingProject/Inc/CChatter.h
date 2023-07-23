#pragma once

class CChatter : public Heading::CSelecter
{
public:
	struct ChatData
	{
		uint64_t Key;
	};

	CChatter( );
	virtual ~CChatter( );

	virtual void Set_NewSession( Heading::NewSocketList& _newSocket ) override;

	void Do_PostProcess( );

private:

	CChatBuffer m_chat;
};


