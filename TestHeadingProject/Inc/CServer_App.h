#pragma once

class CServer_App
{
public:
	static void InitializeApplication();
	static void ListenBinding();
	static void SocketSelecting();
	static void EndProcessing();

	static bool ServiceCheck();

	static void ServerStop();
private:
	static std::atomic<bool> m_live;

	static WSADATA m_data;

	static Heading::CAcceptThread* m_accept;

	static concurrency::concurrent_unordered_map<WSAEVENT, Heading::CSimpleSocket*>							m_eventMap;

	// 
	static concurrency::concurrent_unordered_map<Heading::CSimpleSocket*, Heading::CEventBaseSession_v2*>	m_socketMap;

	// 연결되었던 정보가 있는 맵
	// 비동기로 틱을 처리하면서 Connection 체크를 실패하는 Session이 유지된 기간이 오래되면 삭제
	static concurrency::concurrent_unordered_map<uint64_t, Heading::CEventBaseSession_v2*>					m_sessionMap;
};

