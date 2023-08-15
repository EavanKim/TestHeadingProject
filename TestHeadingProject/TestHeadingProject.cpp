#include "psudoPCH.h"
#include <csignal>

#if _WIN32
	#ifdef _DEBUG
		#include "crtdbg.h"
		#define new new( _CLIENT_BLOCK, __FILE__, __LINE__)
	#endif // _DEBUG
#endif

volatile sig_atomic_t exitSig;

void signal_handler(int sig)
{
	signal(sig, signal_handler);
	CServer_App::ServerStop();
	CServer_App::EndProcessing( );
#if _WIN32
#ifdef _DEBUG
	_CrtDumpMemoryLeaks( );
#endif
#endif
	exitSig= 1;
}

int main()
{
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
#ifdef SIGBREAK
	signal(SIGBREAK, signal_handler);
#endif

#ifdef _DEBUG
	//_CrtSetBreakAlloc( 175 );
	_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_DEBUG );
	_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
	_CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_DEBUG );
	//_CrtSetReportMode(_CRT_ERRCNT, _CRTDBG_MODE_DEBUG); // 에러 출력. CRT_ERRCNT 값 문제로 보이는데 확인 해 보기.
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	//_CrtMemDumpAllObjectsSince();
#endif

	CServer_App::InitializeApplication( );

	CServer_App::ListenBinding( );
	try
	{
		while( CServer_App::ServiceCheck( ) )
		{
			EventManager::get( )->logFlush( );
			CServer_App::SocketSelecting( );
		}
	}
	catch( ... )
	{
		TCHAR* message = nullptr;
		FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
					   nullptr,
					   GetLastError( ),
					   MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
					   ( TCHAR* ) &message,
					   0,
					   nullptr );

		wprintf( L" LastError String : %s", message );
		LocalFree( message );
	}

	CServer_App::EndProcessing( );

#if _WIN32
	#ifdef _DEBUG
		_CrtDumpMemoryLeaks( );
	#endif
#endif
}