#pragma once

typedef std::unordered_map<WSAEVENT, uint64_t>			EventToKeyMap;
typedef std::unordered_map<std::string, uint64_t>		NameToKeyMap;

// Accepter 에서 User 정보를 받아 온 다음에 처리하는 위치이므로
// nickName를 받았을 때만 초기화 해서 Select 시킵니다.
class CChatUser_v2
{
public:
	CChatUser_v2();
	~CChatUser_v2();

	// 실패하면 해제할 수 있도록 결과를 밖에 알려줍니다.
	bool Add( WSAEVENT _event, std::string _nickName, Heading::CSimpleSocket* _socket );

	void Remove( WSAEVENT _event );
	void Remove( std::string _nickName );
	void Remove( uint64_t _sessionKey );
	void Remove( CChatSession_v2* _session );

	std::string				find( WSAEVENT _event );
	WSAEVENT				find( std::string _nickname );
	Heading::CSimpleSocket* find( uint64_t _key );

	bool PreSelect( WSAEVENT* _ptr, int _count );

	void clear();

private:
	std::queue<uint64_t>		m_freeSessionKey;
	CChatSession_v2*			m_sessionArray[ WSA_MAXIMUM_WAIT_EVENTS ] = { nullptr, };
	EventToKeyMap				m_evtToKeyMap;
	NameToKeyMap				m_nameToKeyMap;
	Heading::Array_WSAEvent		m_wsaEvents;
};