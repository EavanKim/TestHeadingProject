#pragma once
class TestServer_Chat
{
public:
	TestServer_Chat( uint16_t _startPort );
	~TestServer_Chat();

	void Ready();
	void Update();

private:
	time_t m_serverStart = time( NULL );
	std::queue<CMessage_BroadCast*> m_messages = {};

	CSocket_Listen* m_proxy = nullptr;
	CSocket_Listen* m_broadCast = nullptr;
};

