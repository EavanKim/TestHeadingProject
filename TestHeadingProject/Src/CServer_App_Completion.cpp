#include "psudoPCH.h"

CServer_App_Completion* CServer_App_Completion::m_instance = nullptr;

void CServer_App_Completion::Initialize()
{
	if ( nullptr == m_instance )
	{
		m_instance = new CServer_App_Completion();
	}
}

CServer_App_Completion* CServer_App_Completion::get()
{
	return m_instance;
}

void CServer_App_Completion::dispose()
{
	if ( nullptr != m_instance )
	{
		delete m_instance;
		m_instance = nullptr;
	}
}

void CServer_App_Completion::Prepare()
{

	m_completionHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	m_accepter = new Heading::CAccepter(50000);
	m_accepter->Bind();
}

void CServer_App_Completion::Process_Accept()
{
	sockaddr_in addr_in;
	SOCKET newClient = m_accepter->CreateConnect(addr_in);

	CreateIoCompletionPort((HANDLE)newClient, m_completionHandle, newClient, 0);

	SessionInfo* info = new SessionInfo; // 예제 코드는 이거 죄다 누수하고 있는데 관리자 만들기. 아마 Overlapped가 0번이라서 Overlapped 주소를 SessionInfo 형으로 받아 처리하는 것으로 추측
	info->sock_ = newClient;
	info->wsaBuf_.buf = info->storage_.allocate<CHAR>(MAXIMUM_PACKET_DATA_LENGTH); // 세션 관련 데이터는 한 데 몰아놨다가 한 방에 날리려고 하니까 사이즈 고민 해 보기. (예상 서버 스팩 램에서 데이터 할당량 / 필요 최저한도 동시접속자수)
	info->wsaBuf_.len = MAXIMUM_PACKET_DATA_LENGTH;

	DWORD flags = 0;
	DWORD recvBytes = 0;
	DWORD retVal = WSARecv(newClient, &info->wsaBuf_, 1, &recvBytes, &flags, &info->overlapped_, NULL);
	// 여기도 에러 관련 작업 추가하기.
	// 이렇게 연결해놓고 GetMessage Pump 처럼 GetQueuedCompletionStatus 를 써서 하나씩 꺼내서 처리하는 구조
}

CServer_App_Completion::CServer_App_Completion()
{
	WSADATA wsa;
	if ( S_OK != WSAStartup(MAKEWORD(2, 2), &wsa) )
	{
		// 에러 관련 처리 추가 작업하기
	}
}

CServer_App_Completion::~CServer_App_Completion()
{
	if( nullptr != m_accepter )
	{
		delete m_accepter;
		m_accepter = nullptr;
	}

	WSACleanup();
}
