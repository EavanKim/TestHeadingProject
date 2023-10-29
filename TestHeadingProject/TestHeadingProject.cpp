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

	Heading::IHeadingSelecter* select = new Heading::CDefaultSelecter();

	if ( (nullptr == select) && (select->IsReady()) )
		return -1;

	while ( select->IsLive() )
	{
		select->Update();
	}

	// 최종 정리
	select->Dispose();
	delete select;
	select = nullptr;

#if _WIN32
#ifdef _DEBUG
	_CrtDumpMemoryLeaks( );
#endif
#endif
}