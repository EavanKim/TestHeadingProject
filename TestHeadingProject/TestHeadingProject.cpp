// TestHeadingProject.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <ws2ipdef.h>

#pragma comment(lib, "ws2_32.lib")
#include <Windows.h>

// 나중에 DLL화 하면 공통헤더로 해결할 패킷 데이터
#pragma pack(push, 1)
struct Header
{
	uint64_t type = 0;
	uint64_t length = 0;
};

template<uint64_t _type, uint64_t _buffersize>
struct SendStruct : public Header
{
	char buffer[ _buffersize ];

	SendStruct()
	{
		type = _type;
		length = _buffersize;
	}
};

typedef SendStruct<1, 43> TestBuffer;
#pragma pack(pop)

uint64_t m_resultsize = 0;
char Buffer[ 1 << 13 ];
char RecvBuffer[ 1 << 13 ];

void PrintMem(char* _ptr, uint64_t _length)
{
	for( uint64_t seek = 0; _length > seek; ++seek )
	{
		printf( "%02X", _ptr[ seek ] );
	}
	printf( "\n" );
}

void WriteResultBuffer( char* _buffer, uint64_t _length )
{
	printf("[Input debug buffer : %s][debug ptr : %llX] \n", _buffer, _buffer );
	printf("[Input debug length : %lld] \n", _length );

	char* destPtr = ( char* )( Buffer + m_resultsize );
	uint64_t length = ( uint64_t )( 1 << 13 ) - m_resultsize;
	memcpy_s( destPtr, length, _buffer, _length );

	m_resultsize += _length;
}

// 지금은 받는 패킷이 하나니까 더 복잡하게 처리 안할 예정.
void ParseHeader( char* _buffer, Header& _parse )
{
	memcpy( &_parse, _buffer, sizeof( Header ) );

	//printf( "Header Pasing Result = [type : %lld][length : %lld] \n", _parse.type, _parse.length );
}

void ParseData(char* _buffer, TestBuffer& _parse )
{
	memcpy( &_parse, _buffer, sizeof( TestBuffer ) );

	//printf( "TestBuffer Pasing Result = [type : %lld][length : %lld][buffer : %lls] \n", _parse.type, _parse.length, _parse.buffer );
}

void ProcessTime( time_t& _start, uint64_t& count, time_t now = time(NULL) )
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

uint64_t ReadData( char* _buffer, uint64_t _totalrecvSize, uint64_t& _counter )
{
	uint64_t processSize = 0;
	uint64_t seek = 0;

	while( 0 != _totalrecvSize )
	{
		if( _totalrecvSize < sizeof( Header ) )
			return processSize;

		char* currPtr = _buffer + seek;
		Header getHeader = {};
		uint64_t type = 0;
		uint64_t packlength = 0;
		ParseHeader( currPtr, getHeader );

		switch( getHeader.type )
		{
			case 1:
			{
				if( _totalrecvSize < sizeof( TestBuffer ) )
					return processSize;
				TestBuffer parseData = {};

				ParseData( currPtr, parseData );
				packlength = sizeof( TestBuffer );
				processSize += packlength;
				printf( parseData.buffer );
			}
				break;
			default:
				printf("!!! Packet Parsing Failure !!! \n");
				return processSize;
		}

		seek = seek + packlength;
		_totalrecvSize = _totalrecvSize - packlength;

		++_counter;
	}

	return processSize;
}

uint64_t ProcessPacket( char* _buffer, uint64_t _totalrecvSize, uint64_t& _counter, uint64_t& _reserveSize, DWORD& _receiveSize, time_t _start )
{
	// 일단 상수로 먼저 짭니다.
	// 어차피 Sync는 테스트 용도에 가까우므로 이 이후에 제대로 작성해야합니다.
	_totalrecvSize += _receiveSize;
	ProcessTime( _start, _counter );
	printf( "[reserveSize : %lld][receiveSize : %lld][m_totalRecv : %lld]TestBuffer[% s] \n", _reserveSize, _receiveSize, _totalrecvSize, _buffer + _reserveSize + 16 );
	//++counter0;


	if( _totalrecvSize < sizeof( Header ) )
	{
		return 0;
	}
	uint64_t* recvBufLengPtr = ( uint64_t* )( _buffer + _reserveSize + 8 );
	uint64_t BufferLength = *recvBufLengPtr;

	return ReadData( _buffer, _totalrecvSize, _counter );
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

int CreateSocket(SOCKET& _listenSock, addrinfo* _info, sockaddr_in& _service )
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
	return 0;
}

int main()
{
	uint64_t m_bufferSize = 0;

	uint64_t m_currentpacketSize = 0;
	uint64_t m_totalRecv = 0;

	WSADATA m_data = {};
	addrinfo* m_info = nullptr;
	SOCKET m_socket = INVALID_SOCKET;

	int result = WSAStartup( MAKEWORD( 2, 2 ), &m_data );
	if( S_OK != result )
	{
		// 초기화 실패
		int sockerror = WSAGetLastError();
		int winerror = GetLastError();
		// exception 객체 생성되면 throw하면서 에러 정보 송신
		return 1;
	}

	sockaddr_in service;
	if( 0 != ConnectInfoCreate( service, m_info ) )
		return 1;

	if( 0 != CreateSocket( m_socket, m_info, service ) )
		return 1;

	DWORD receiveSize = 0;

	freeaddrinfo( m_info );

	SOCKET sessionOpen = INVALID_SOCKET;
	
	if( 0 != WaitClient( m_socket, sessionOpen ) )
		return 1;

	uint64_t counter0 = 0;
	time_t start = time(NULL);
	while( 1 )
	{
		uint64_t reserveSize = ( ( uint64_t )( 1 << 13 ) - m_totalRecv );
		receiveSize = recv( sessionOpen, RecvBuffer + m_totalRecv, reserveSize, 0 );
		if( -1 == receiveSize )
		{
			int sockerror = WSAGetLastError();
			int winerror = GetLastError();
			// 에러 복구가 안되니 무한이 아니라 나가기
			closesocket(sessionOpen);
			WSACleanup();
			return 1;
		}

		uint64_t ProcessLength = ProcessPacket( RecvBuffer, m_totalRecv, counter0, reserveSize, receiveSize, start );

		printf("[Process Size : %lld][m_totalRecv : %lld]", ProcessLength, m_totalRecv );

		if( 0 != ProcessLength )
		{
			if( 0 != m_totalRecv )
			{
				memcpy( RecvBuffer, RecvBuffer + ProcessLength, m_totalRecv );
			}
		}
	}

	//================================================================================================================================================================
}