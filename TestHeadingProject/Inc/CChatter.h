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

	void Do_PostProcess( );

private:

	CChatBuffer m_chat;

};


