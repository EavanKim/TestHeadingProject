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

	static CChatUser m_user;
};
