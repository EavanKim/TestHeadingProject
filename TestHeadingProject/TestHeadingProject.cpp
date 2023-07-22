#include "psudoPCH.h"

int main()
{
	Heading::SimpleServerKit::E_LOG_LEVEL currentLogLevel = Heading::SimpleServerKit::E_LOG_LEVEL::E_LOG_LEVEL_NONE;

#if _DEBUG
	currentLogLevel = Heading::SimpleServerKit::E_LOG_LEVEL::E_LOG_LEVEL_DEBUG;
#else
	currentLogLevel = Heading::SimpleServerKit::E_LOG_LEVEL::E_LOG_LEVEL_RELEASE;
#endif

	Heading::SimpleServerKit::Manager::Init( currentLogLevel , 2 );

	try
	{
		Heading::SimpleServerKit::Manager::Get()->Start(50000);

		Heading::SimpleServerKit::Manager::Get()->Update();
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

	Heading::SimpleServerKit::Manager::Get()->Dispose();
}