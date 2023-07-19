
#include "psudoPCH.h"
#include "TestHeadingProject.h"
int main()
{
	WSAData Data = {};
	Heading::start( Data );

	// 서버의 핵심은 두가지
	// 1. 모든 소켓이 올바르게 동작할 수 있는 셀렉트 형식인가
	// 2. 데이터를 처리할 때 순서를 지킬 수 있는 마지노선에서 가장 빠르게 처리가 가능한가.
	// 1번은 API 선택과 올바른 선언으로 해결
	// 2번이 서버 프로그래머에게 주어진 업무
	// 2번에 대해서 처리할 때 핵심은 결국 송수신 버퍼 컨트롤
	// 소켓 자체가 가지고 있는 버퍼는 내가 컨트롤 할 영역이 아니니 에러컨트롤만 하고
	// 송 수신 버퍼에 들어온 걸 최단시간에 털기 위한 조건을 고민하기.
	// 여기서 송신 버퍼에 데이터가 들어오는 형태를 셀렉트가 결정하고, 용도에 맞는 셀렉트의 선택이 필요.
	// 링버퍼를 만들어서 정해진 양 안에서 정해진 제한들 (버퍼가 가득 차기 전에는 모두 처리되어야 하는 제한)을 지키며 처리해보는것이 필요할 것.
	// 이번주 내로 링버퍼 처리 시작하기.
	try
	{
		bool IsServerLive = true;
		
		Heading::CAccept_Mgr main;
		Heading::CChatter chat;

		main.Set_NewAcceptPort( 50000 );

		while( IsServerLive )
		{
			Heading::NewSocketList socketlist;
			main.Do_Select( );
			if( main.Get_NewSocket( socketlist ) )
			{
				// 새 연결이 생기면 Client Session 처리
				chat.Set_NewSession( socketlist );
			}

			// 무조건 Select 1회에 1개 소켓이 처리된다고 생각하고 돌리기.
			chat.Do_Select( );
			chat.Do_PostProcess( );
		}
	}
	catch( ... )
	{
		// 크래시하면 죽어버리고, 바로 죽는게 억지로 살리는 것 보단 낫겠지만... 적어도 뭔 일이 벌어졌는지는 볼 수 있도록 합니다.

		TCHAR* message = nullptr;
		FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
					   nullptr,
					   GetLastError(),
					   MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
					   ( TCHAR* )&message,
					   0,
					   nullptr );

		wprintf( L" LastError String : %s", message );
		LocalFree( message );
	}

	Heading::end();
}