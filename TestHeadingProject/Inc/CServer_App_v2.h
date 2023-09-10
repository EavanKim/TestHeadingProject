#pragma once
// Select는 2중으로 돌립니다.
// Accept Select 는 Accept 자체 소켓과 신규 연결에 관한 소켓

class CServer_App_v2
{
public:
	static void Initialize();
	static CServer_App_v2* get();
	void dispose();

	void AcceptSelect();
	void SessionSelect();
	void SendProcess();

private:
	CServer_App_v2();
	~CServer_App_v2();
		
	CChatUser_v2 m_sessions;

	std::thread* m_acceptThread = nullptr;

	static CServer_App_v2* m_instance;
};
