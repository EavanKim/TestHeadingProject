#include "psudoPCH.h"

int main()
{
	E_LOG_LEVEL currentLogLevel = E_LOG_LEVEL::E_LOG_LEVEL_NONE;

#if _DEBUG
	currentLogLevel = E_LOG_LEVEL::E_LOG_LEVEL_DEBUG;
#else
	currentLogLevel = E_LOG_LEVEL::E_LOG_LEVEL_RELEASE;
#endif

	Manager::Init( currentLogLevel , 2 );

	try
	{
		//Manager::Get()->Start(50000);
		Manager::Get()->ChattingStartUp( );
		Manager::Get()->Update();
	}
	catch( ... )
	{
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

	Manager::Get()->Dispose();
}