#include "psudoPCH.h"

// 서버가 ~~~를 한다
enum E_SYSTEM_EVENTS
{
	E_SYSTEM_ACCEPT,
	E_SYSTEM_SELECT,
	E_SYSTEM_RECV,
	E_SYSTEM_BROADCAST,
	E_SYSTEM_MAX
};

// 패킷으로 ~~~를 한다
enum E_PACKET_EVENTS
{
	E_PACKET_CHATTING,
	E_PACKET_WISPERING,
	E_PACKET_ENTER,
	E_PACKET_EXIT,
	E_PACKET_MAX
};

int (*SystemEvents[E_SYSTEM_MAX])(void*);
bool (*PacketEvents[E_PACKET_MAX])(void*);

int main()
{


	try
	{

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
}