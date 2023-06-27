#pragma once
class CMessage
{
public:
	uint64_t m_sessionKey;
	Header* m_message;
	
	CMessage( uint64_t _sessionKey, Header* _message )
		: m_sessionKey( _sessionKey )
		, m_message( _message )
	{

	}

	CMessage( CMessage& _message )
	{
		m_sessionKey = _message.m_sessionKey;
		m_message = _message.m_message;
	}

	CMessage( CMessage&& _message ) noexcept
	{
		m_sessionKey = _message.m_sessionKey;
		m_message = _message.m_message;
	}

	void operator =( CMessage& _message )
	{
		m_sessionKey = _message.m_sessionKey;
		m_message = _message.m_message;
	}

	void operator =( CMessage&& _message )
	{
		m_sessionKey = _message.m_sessionKey;
		m_message = _message.m_message;
	}

	bool operator <( CMessage& _message )
	{
		return m_message->m_time < _message.m_message->m_time;
	}
	bool operator >( CMessage& _message )
	{
		return m_message->m_time > _message.m_message->m_time;
	}
	bool operator <( CMessage* _message )
	{
		return m_message->m_time < _message->m_message->m_time;
	}
	bool operator >( CMessage* _message )
	{
		return m_message->m_time > _message->m_message->m_time;
	}
};

class CMessage_BroadCast : public CMessage
{
public:
};

