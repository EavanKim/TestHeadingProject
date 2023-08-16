#pragma once
class AccessChecker : public Heading::CEventBaseSession_v2
{
public:
	AccessChecker( Heading::CSimpleSocket* _sock );
	~AccessChecker( );

	bool isNewClient( );

	// 기존 연결정보가 없을 때 세션을 새로 만들기
	// 1차적으로는 연결만 되면 되므로 이전 세션 정보 체크 없이 새 세션 생성부터 하기.
	CChatSession_v2* newSession( );
	Heading::CSimpleSocket* endAccessChecker( );

	static void onRecv(void* _simpleSocket, void* _accessChecker);

private:
	bool m_isReceivedAccessData = false;
};

