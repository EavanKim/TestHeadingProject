
#include "psudoPCH.h"

int main()
{
	WSAData Data = {};
	Heading::start( Data );

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
			chat.Do_Select( );
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