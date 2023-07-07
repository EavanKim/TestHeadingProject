
#include "psudoPCH.h"

int main()
{
	WSAData Data = {};
	Heading::start( Data );

	//===================================================================================================================================================================

	try
	{
		bool IsServerLive = true;
		//================================================================================================================================================================

		Heading::bindingInfo mainServer;
		Heading::bindingInfo broadCastServer;

		mainServer.port = 50000;
		broadCastServer.port = 51000;

		Heading::createInfo( mainServer );
		Heading::createInfo( broadCastServer );

		Heading::bind( mainServer );
		Heading::bind( broadCastServer );

		while( IsServerLive )
		{
			Heading::aSelect_Read( mainServer );
			Heading::aSelect_RW( broadCastServer );

			Heading::recv( mainServer );
			Heading::recv( broadCastServer );
			Heading::send( broadCastServer );
		}

		Heading::release( mainServer );
		Heading::release( broadCastServer );

		//================================================================================================================================================================
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
	
	//===================================================================================================================================================================

	Heading::end();
}