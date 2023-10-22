#include "psudoPCH.h"
#include <csignal>

#if _WIN32
#ifdef _DEBUG
#include "crtdbg.h"
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__)
#endif // _DEBUG
#endif

#ifndef _DEBUG                  /* For RELEASE builds */
#define  ALERT_IF2(expr, msg, arg1, arg2)  do {} while (0)
#else                           /* For DEBUG builds   */
#define  ALERT_IF2(expr, msg, arg1, arg2) \
    do { \
        if ((expr) && \
            (1 == _CrtDbgReport(_CRT_ERROR, \
                __FILE__, __LINE__, msg, arg1, arg2))) \
            _CrtDbgBreak( ); \
    } while (0)
#endif

volatile sig_atomic_t exitSig;

void signal_handler(int sig)
{
	signal(sig, signal_handler);
	//CServer_App::ServerStop();
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
	_CrtSetBreakAlloc(170);
	_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_DEBUG );
	_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
	_CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_DEBUG );
	//_CrtSetReportMode(_CRT_ERRCNT, _CRTDBG_MODE_DEBUG);
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	_CrtMemDumpAllObjectsSince( NULL );
#endif

	//{
	//	Heading::CSessionStorage_v2* testStorage = new Heading::CSessionStorage_v2;


	//	for ( int count = 0; 100000 > count; ++count )
	//	{
	//		char* gettestbuff = testStorage->getBuffer();

	//		if ( NULL != gettestbuff )
	//		{
	//			for ( int seek = 0; MAXIMUM_PACKET_DATA_LENGTH > seek; ++seek )
	//				gettestbuff[seek] = count % 10;

	//			testStorage->commitBuffer(MAXIMUM_PACKET_DATA_LENGTH);
	//		}
	//		else
	//		{
	//			printf("Buffer Out!!!");
	//			break;
	//		}
	//	}

	//	delete testStorage;
	//	testStorage = nullptr;
	//}

	//CServer_App::InitializeApplication( );

	//CServer_App::ListenBinding( );

	//try
	//{
	//	while( CServer_App::ServiceCheck( ) )
	//	{
	//		EventManager::get( )->logFlush( );
	//		CServer_App::SocketSelecting( );
	//	}
	//}
	//catch( ... )
	//{
	//	TCHAR* message = nullptr;
	//	FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
	//				   nullptr,
	//				   GetLastError( ),
	//				   MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
	//				   ( TCHAR* ) &message,
	//				   0,
	//				   nullptr );

	//	wprintf( L" LastError String : %hs", message );
	//	LocalFree( message );
	//}

	//CServer_App::ServerStop();
	//CServer_App::EndProcessing( );
#if _WIN32
#ifdef _DEBUG
	_CrtDumpMemoryLeaks( );
#endif
#endif
}