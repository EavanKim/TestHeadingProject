#pragma once
class CChatSession_v2 : public Heading::CEventBaseSession_v2
{
public:
	CChatSession_v2( Heading::CSimpleSocket* _sock );
	~CChatSession_v2();

	std::string getName();

private:
	std::string m_nickname = "";
};

