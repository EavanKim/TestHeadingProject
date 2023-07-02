// TestHeadingProject.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "psudoPCH.h"


uint64_t m_resultsize = 0;
char RecvBuffer[ 1 << 13 ];
bool IsNeedReconnectWait = false;

//================================================================================================================================================================
// 지금은 받는 패킷이 하나니까 더 복잡하게 처리 안할 예정.
void ProcessTime( time_t& _start, uint64_t& count, time_t now = time( NULL ) )
{
	if( UINT64_MAX == count )
	{
		count = 0;
	}
	uint64_t Time = now - _start;
	uint64_t Timedevide = ( 0 == Time ? 1 : Time );
	uint64_t MPS = count / Timedevide;
	printf( "[%lld][count : %lld][MPS : %lld] ", Time, count, MPS );
}

uint64_t ReadData( char* _buffer, uint64_t& _totalrecvSize, uint64_t& _counter )
{
	uint64_t processSize = 0;
	uint64_t lastLength = 0;

	while( 0 != _totalrecvSize )
	{
		if( _totalrecvSize < sizeof( Header ) )
			return processSize;

		char* currPtr = _buffer + lastLength;
		Header getHeader = {};
		uint64_t type = 0;
		uint64_t packlength = 0;
		Util::ParseHeader( currPtr, getHeader );

		switch( getHeader.type )
		{
			case 2:
				IsNeedReconnectWait = true;
				break;
			case 100:
			{
				if( _totalrecvSize < sizeof( TestBuffer ) )
					return processSize;
				TestBuffer parseData = {};

				Util::ParseData( currPtr, parseData );
				packlength = sizeof( TestBuffer );
				//printf( parseData.buffer );
				//printf( "\n" );
			}
			break;
			default:
				printf( "!!! Packet Parsing Failure !!! \n" );
				return processSize;
		}

		processSize += getHeader.length;
		lastLength += getHeader.length;
		_totalrecvSize -= getHeader.length;
		//printf("%lld \n", _totalrecvSize);

		++_counter;
	}

	return processSize;
}

uint64_t ProcessPacket( char* _buffer, uint64_t& _bufferSize, uint64_t& _counter, uint64_t& _reserveSize, DWORD& _receiveSize, time_t _start )
{
	// 일단 상수로 먼저 짭니다.
	// 어차피 Sync는 테스트 용도에 가까우므로 이 이후에 제대로 작성해야합니다.
	_bufferSize += _receiveSize;
	ProcessTime( _start, _counter );
	printf( "[reserveSize : %lld][receiveSize : %lld][bufferSize : %ulld]TestBuffer[% s] \n", _reserveSize, _receiveSize, _bufferSize, _buffer + _reserveSize );
	//++counter0;


	if( _bufferSize < sizeof( Header ) )
	{
		return 0;
	}

	return ReadData( _buffer, _bufferSize, _counter );
}

int ConnectInfoCreate( sockaddr_in& _result, addrinfo*& m_info )
{
	addrinfo createData = {};

	SecureZeroMemory( ( PVOID )&createData, sizeof( addrinfo ) );

	createData.ai_family = AF_INET;
	createData.ai_socktype = SOCK_STREAM;
	createData.ai_protocol = IPPROTO_TCP;
	createData.ai_flags = AI_PASSIVE;

	int result = getaddrinfo( nullptr, "50000", &createData, &m_info );
	if( S_OK != result )
	{
		// 초기화 실패
		int sockerror = WSAGetLastError();
		int winerror = GetLastError();
		// exception 객체 생성되면 throw하면서 에러 정보 송신
		return 1;
	}
	sockaddr_storage storage;
	memset( &storage, 0, sizeof storage );
	// The socket address to be passed to bind
	_result.sin_family = AF_INET;
	_result.sin_addr.s_addr = htonl( INADDR_ANY );
	_result.sin_port = htons( 50000 );

	return 0;
}

int CreateSocket( SOCKET& _listenSock, addrinfo* _info, sockaddr_in& _service )
{
	int result = 0;
	int loopCounter = 0;
	do
	{
		if( 5 < loopCounter )
		{
			int winerror = GetLastError();

			if( INVALID_SOCKET != _listenSock )
			{
				closesocket( _listenSock );
				_listenSock = INVALID_SOCKET;
			}
			// exception 객체 생성되면 throw하면서 에러 정보 송신
			return 1;
		}

		_listenSock = socket( _info->ai_family, _info->ai_socktype, _info->ai_protocol );
		if( INVALID_SOCKET == _listenSock )
		{
			continue;
		}

		result = bind( _listenSock, ( SOCKADDR* )&_service, sizeof( _service ) );
		if( result == SOCKET_ERROR )
		{
			int err = 0;
			if( WSAECONNREFUSED == ( err = WSAGetLastError() ) )
			{
				closesocket( _listenSock );
				_listenSock = INVALID_SOCKET;
				continue;
			}
			wprintf( L"connect failed with error: %d\n", err );
			freeaddrinfo( _info );
			closesocket( _listenSock );
			return 1;
		}
	}
	while( S_OK != result );

	return 0;
}

int WaitClient( SOCKET& _listenSock, SOCKET& _session )
{
	int result = listen( _listenSock, SOMAXCONN );
	if( result == SOCKET_ERROR )
	{
		printf( "listen failed with error: %d\n", WSAGetLastError() );
		closesocket( _listenSock );
		WSACleanup();
		return 1;
	}

	_session = accept( _listenSock, NULL, NULL );
	if( _session == INVALID_SOCKET )
	{
		printf( "accept failed with error: %d\n", WSAGetLastError() );
		closesocket( _listenSock );
		WSACleanup();
		return 1;
	}


	// No longer need server socket
	closesocket( _listenSock );
	_listenSock = INVALID_SOCKET;
	return 0;
}

void DoRawCode()
{
	//sockaddr_in service;
		//if( 0 != ConnectInfoCreate( service, m_info ) )
		//	return 1;
		//
		//if( 0 != CreateSocket( socket, m_info, service ) )
		//	return 1;
		//
		//DWORD receiveSize = 0;
		//
		//freeaddrinfo( m_info );
		//
		//SOCKET sessionOpen = INVALID_SOCKET;
		//
		//if( 0 != WaitClient( socket, sessionOpen ) )
		//	return 1;
		//
		//uint64_t NoSignalCheck = 0;
		//uint64_t counter0 = 0;
		//time_t start = time(NULL);
		//const uint64_t BUFFER_SIZE = sizeof( RecvBuffer );
		//while( 1 )
		//{
		//	uint64_t reserveSize = ( BUFFER_SIZE - bufferSize );
		//	if( 0 == reserveSize )
		//	{
		//		closesocket(sessionOpen);
		//		sessionOpen = INVALID_SOCKET;
		//		WSACleanup();
		//		return 1;
		//	}
		//
		//	receiveSize = recv( sessionOpen, RecvBuffer + bufferSize, reserveSize, 0 );
		//	if( SOCKET_ERROR == receiveSize || IsNeedReconnectWait || 1000 < NoSignalCheck )
		//	{
		//		int sockerror = WSAGetLastError();
		//		int winerror = GetLastError();
		//
		//		closesocket( sessionOpen );
		//		sessionOpen = INVALID_SOCKET;
		//
		//		if( 0 != ConnectInfoCreate( service, m_info ) )
		//			return 1;
		//
		//		if( 0 != CreateSocket( socket, m_info, service ) )
		//			return 1;
		//
		//		freeaddrinfo( m_info );
		//
		//
		//		printf("Ready For New Connect");
		//		if( 0 != WaitClient( socket, sessionOpen ) )
		//			return 1;
		//
		//		counter0 = 0;
		//		start = time( NULL );
		//		IsNeedReconnectWait = false;
		//		NoSignalCheck = 0;
		//		continue;
		//	}
		//	if( 0 == receiveSize )
		//	{
		//		++NoSignalCheck;
		//		continue;
		//	}
		//
		//	uint64_t ProcessLength = ProcessPacket( RecvBuffer, bufferSize, counter0, reserveSize, receiveSize, start );
		//
		//	if( 0 != ProcessLength )
		//	{
		//		if( 0 != bufferSize )
		//		{
		//			memcpy( RecvBuffer, RecvBuffer + ProcessLength, bufferSize );
		//		}
		//	}
		//
		//	printf( "[Process Size : %lld][bufferSize : %lld] \n", ProcessLength, bufferSize );
		//
		//	if( UINT64_MAX - 10000 <= counter0 )
		//	{
		//		counter0 = 0;
		//		start = time( NULL );
		//	}
		//}
}

//================================================================================================================================================================

void DoClassCode_First()
{
	//TestSock_Server server(50000);
		//
		//if( 0 != server.CreateInitializeData() )
		//	return 1;
		//
		//server.Binding();
		//CSession* session = server.Wating();
		//
		//if( nullptr == session )
		//	return 1;
		//
		//if( !session->IsConnected() )
		//{
		//	delete session;
		//}
		//
		//while( 1 )
		//{
		//	if( NULL == session || !session->IsConnected() )
		//	{
		//		session = server.Wating();
		//		if( !session->IsConnected() )
		//		{
		//			server.CloseSession( session );
		//			return 1;
		//		}
		//	}
		//
		//	session->Process();
		//}
}

//================================================================================================================================================================

void DoClassCode_Second()
{
	//TestServer_Select selectServer( 50000 );

	//if( 0 != selectServer.CreateInitializeData() )
	//	return 1;

	//selectServer.ListenBind();

	//while( 1 )
	//{
	//	selectServer.Update();
	//}
}

//================================================================================================================================================================

struct SimpleSession
{
	SimpleSession()
	{
		ZeroMemory( this, sizeof( SimpleSession ) );
	}

	SimpleSession( uint64_t _sesssionKey )
		: m_buffer( _sesssionKey )
	{
		ZeroMemory( this, sizeof( SimpleSession ) );
	}

	SimpleSession( SimpleSession& _copy )
		: m_sesssionKey( _copy.m_sesssionKey )
		, m_buffer( _copy.m_buffer )
	{
	}

	SimpleSession( SimpleSession&& _move )
		: m_sesssionKey( _move.m_sesssionKey )
		, m_buffer( _move.m_buffer )
	{
	}

	uint64_t		m_sesssionKey	= 0;
	CNet_Buffer		m_buffer;
};

struct SimpleSocket
{
	SOCKET _sock;
};

SOCKET				sock_bind			= INVALID_SOCKET;
SOCKET				sock_broadCast		= INVALID_SOCKET;
uint16_t			port_bind			= 50000;
uint16_t			port_broadCast		= 51000;
uint64_t			sessionNum			= 0;
sockaddr_in			listenIn_bind		= {};
sockaddr_in			listenIn_broadCast	= {};
fd_set				fd_bind				= {};
fd_set				fd_broadCast		= {};
fd_set				fd_temp				= {};
std::vector<SOCKET> clients_bind		= {};
std::vector<SOCKET> clients_broadCast	= {};

void fn_printTime()
{
	static time_t	startTime	= time(NULL);
	time_t			currentTime = time(NULL);

	printf("%lld", currentTime - startTime);
}

int fn_message_send( SOCKET& _socket, Header* _message )
{
	return send( _socket, ( char* )_message, _message->length, 0 );
}

void Setaddr( uint16_t _port, sockaddr_in& _listenIn )
{
	sockaddr_storage storage;
	memset( &storage, 0, sizeof storage );
	// The socket address to be passed to bind
	_listenIn.sin_family = AF_INET;
	_listenIn.sin_addr.s_addr = htonl( INADDR_ANY );
	_listenIn.sin_port = htons( _port );
}

SOCKET fn_bind( sockaddr_in& _listenIn )
{
	SOCKET result		= INVALID_SOCKET;
	int returnValue		= 0;
	int loopCounter		= 0;

	// 새로 바인딩하면 초기화해버리기
	if( INVALID_SOCKET != result )
	{
		closesocket( result );
		result = INVALID_SOCKET;
	}

	do
	{
		if( 5 < loopCounter )
		{
			int winerror = GetLastError();
			// exception 객체 생성되면 throw하면서 에러 정보 송신
			return INVALID_SOCKET;
		}

		result = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
		if( INVALID_SOCKET == result )
		{
			continue;
		}

		returnValue = bind( result, ( SOCKADDR* )&_listenIn, sizeof( _listenIn ) );
		if( returnValue == SOCKET_ERROR )
		{
			int err = 0;
			if( WSAECONNREFUSED == ( err = WSAGetLastError() ) )
			{
				closesocket( result );
				result = INVALID_SOCKET;
				continue;
			}
			printf( "connect failed with error: %d\n", err );
			return INVALID_SOCKET;
		}
	}
	while( S_OK != returnValue );


	return result;
}

void fn_ready( SOCKET _bind, fd_set& _inSet )
{
	FD_ZERO( &_inSet );

	if( SOCKET_ERROR == listen( _bind, 5 ) )
		return;

	FD_SET( _bind, &_inSet );
}

void fn_select( const fd_set& _readfds, fd_set& _temp )
{
	_temp = _readfds;
	uint64_t fd_num = select( 0, &_temp, NULL, NULL, NULL );
}

void fn_select_passThrough( const fd_set& _readfds, fd_set& _temp, uint32_t _wait = 5 )
{
	struct timeval tv;
	tv.tv_sec = _wait;
	tv.tv_usec = 0;
	_temp = _readfds;
	uint64_t fd_num = select( 0, &_temp, NULL, NULL, &tv );
}

void fn_create(const SOCKET _newConn, fd_set& _set, std::unordered_map<SOCKET, SimpleSession>& _sockList)
{
	SOCKET newThing = accept( _newConn, NULL, NULL );
	if( INVALID_SOCKET != newThing )
	{
		++sessionNum;
		_sockList.insert( std::make_pair( newThing, SimpleSession( sessionNum ) ) );
		FD_SET( newThing, &_set );
	}
}

void fn_parse( const SOCKET _newConn, SimpleSession& _data, std::vector<Header*>& _recvMessage, std::vector<SOCKET> _disconnectList )
{
	uint64_t seek		= 0;
	uint64_t currentSize = 0;
	char* data = nullptr;
	uint64_t length = 0;
	if( _data.m_buffer.get_buffer( &data, &length ) )
	{
		int readcount = recv( _newConn, data, length, 0 );
		if( -1 == readcount )
		{
			_disconnectList.push_back( _newConn );
			return;
		}
		if( 0 == readcount )
		{
			_data.m_buffer.get_data( &_recvMessage );
			return;
		}

		_data.m_buffer.commit( readcount );
		_data.m_buffer.get_data( &_recvMessage );
	}
}

void fn_process_recv( const fd_set& _set, std::vector<Header*>& _recvMessage, std::unordered_map<SOCKET, SimpleSession>& _sockList, fd_set& _originset, const SOCKET _CreateTarget )
{
	std::vector<SOCKET> removeList;
	for( uint64_t count = 0; _set.fd_count > count; ++count )
	{
		SOCKET currSock = _set.fd_array[ count ];

		if( FD_ISSET( currSock, &_set ) )
		{
			if( currSock == _CreateTarget )
			{
				fn_create( currSock, _originset, _sockList );
			}
			else
			{
				fn_parse( currSock, _sockList[ currSock ], _recvMessage, removeList );
			}
		}
	}

	for( SOCKET del : removeList )
	{
		_sockList.erase( del );
	}
}

void fn_broadCast( const std::unordered_map<SOCKET, SimpleSession>& _sockList, std::vector<Header*>& _recvMessage )
{
	std::unordered_map<SOCKET, SimpleSession>::const_iterator end = _sockList.end();
	for( std::unordered_map<SOCKET, SimpleSession>::const_iterator iter = _sockList.begin(); end != iter; ++iter )
	{
		for( Header*& sendData : _recvMessage )
			send( iter->first, ( char* )sendData, sendData->length, 0 );
	}

	for( Header*& sendData : _recvMessage )
		delete sendData;

	_recvMessage.clear();
}

//================================================================================================================================================================

typedef std::unordered_map<SOCKET, SimpleSession> socketMap;

int main()
{

	uint64_t	bufferSize = 0;
	uint64_t	currentpacketSize = 0;
	WSADATA		data = {};
	socketMap	bindMap = {};
	socketMap	broadCastMap = {};

	int result = WSAStartup( MAKEWORD( 2, 2 ), &data );
	if( S_OK != result )
	{
		// 초기화 실패
		int sockerror = WSAGetLastError();
		int winerror = GetLastError();
		// exception 객체 생성되면 throw하면서 에러 정보 송신
		return 1;
	}

	//===================================================================================================================================================================

	try
	{
		// 주석 된 것들은 설령 살리더라도 한 번씩 손봐야 합니다.
		// 클래스 스팩이 계속 바뀌고 있습니다.
		//===========================================================================================================================================
		//
		//	DoRawCode();
		//
		//===========================================================================================================================================
		//
		//	DoClassCode_First();
		//
		//================================================================================================================================================================
		//
		//	DoClassCode_Second();
		//================================================================================================================================================================
		// 
		//// 자체 소켓 규약.
		//// 최초에 Port를 정하고나면 뒤로 10개까지는 Reserve입니다.
		//// 10개 단위로 끊어주세요
		//
		//// DoClassCode_Second와 내용은 같지만
		//// 클래스 스팩이 바뀌고 나서의 작업입니다.
		//TestServer_Chat selectServer( 50000 );
		//selectServer.Ready();
		//
		//while( 1 )
		//{
		//	selectServer.Update();
		//}
		//
		//================================================================================================================================================================

		//================================================================================================================================================================

		bool					processLive = true;
		std::vector<Header*>	recvList = {};

		Setaddr(port_bind, listenIn_bind);
		Setaddr(port_broadCast, listenIn_broadCast);

		sock_bind		= fn_bind( listenIn_bind );
		sock_broadCast	= fn_bind( listenIn_broadCast );

		fn_ready( sock_bind, fd_bind );
		fn_ready( sock_broadCast, fd_broadCast );

		while( processLive )
		{
			fn_select_passThrough(fd_bind, fd_temp, 1 );
			fn_process_recv( fd_temp, recvList, bindMap, fd_bind, sock_bind );
			fn_select_passThrough( fd_broadCast, fd_temp, 1 );
			fn_process_recv( fd_temp, recvList, broadCastMap, fd_broadCast, sock_broadCast );

			fn_broadCast( broadCastMap, recvList );
		}

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

	WSACleanup();
}