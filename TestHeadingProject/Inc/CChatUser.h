#pragma once

class CChatUser
{
public:
	CChatUser();
	~CChatUser();

	void Add( WSAEVENT _event, std::string _nickName );

	void Remove( WSAEVENT _event );
	void Remove( std::string _nickName );

	std::string find( WSAEVENT _event );
	WSAEVENT find( std::string _nickname );

	void clear();

private:
	std::unordered_map<WSAEVENT, std::string> m_eventMap;
	std::unordered_map<std::string, WSAEVENT> m_nicknameMap;
};