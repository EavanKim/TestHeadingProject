#include "psudoPCH.h"

int main()
{
	CServer_App::InitializeApplication();

	CServer_App::ListenBinding();
	try
	{
		while( CServer_App::ServiceCheck( ) )
		{
			CServer_App::SocketSelecting();
		}
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

	CServer_App::EndProcessing();
}