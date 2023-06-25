// TestHeadingProject.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>
#include <stdint.h>

#include "define.h"
#include "Util.h"
#include "CPacketParser.h"
#include "TestSock.h"
#include "CSession.h"


uint64_t m_resultsize = 0;
char RecvBuffer[ 1 << 13 ];
bool IsNeedReconnectWait = false;


// 지금은 받는 패킷이 하나니까 더 복잡하게 처리 안할 예정.
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

uint64_t ReadData( char* _buffer, uint64_t& _totalrecvSize, uint64_t& _counter )
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
			case 2:
				IsNeedReconnectWait = true;
				break;
			case 100:
			{
				if( _totalrecvSize < sizeof( TestBuffer ) )
					return processSize;
				TestBuffer parseData = {};

				ParseData( currPtr, parseData );
				packlength = sizeof( TestBuffer );
				//printf( parseData.buffer );
				//printf( "\n" );
			}
				break;
			default:
				printf("!!! Packet Parsing Failure !!! \n");
				return processSize;
		}

		processSize += getHeader.length;
		seek += getHeader.length;
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
	printf( "[reserveSize : %lld][receiveSize : %lld][bufferSize : %lld]TestBuffer[% s] \n", _reserveSize, _receiveSize, _bufferSize, _buffer + _reserveSize );
	//++counter0;


	if( _bufferSize < sizeof( Header ) )
	{
		return 0;
	}
	uint64_t* recvBufLengPtr = ( uint64_t* )( _buffer + _reserveSize + 8 );
	uint64_t BufferLength = *recvBufLengPtr;

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
	_listenSock = INVALID_SOCKET;
	return 0;
}

int main()
{
	uint64_t m_bufferSize = 0;

	uint64_t m_currentpacketSize = 0;

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

	uint64_t NoSignalCheck = 0;
	uint64_t counter0 = 0;
	time_t start = time(NULL);
	const uint64_t BUFFER_SIZE = sizeof( RecvBuffer );
	while( 1 )
	{
		uint64_t reserveSize = ( BUFFER_SIZE - m_bufferSize );
		if( 0 == reserveSize )
		{
			closesocket(sessionOpen);
			sessionOpen = INVALID_SOCKET;
			WSACleanup();
			return 1;
		}

		receiveSize = recv( sessionOpen, RecvBuffer + m_bufferSize, reserveSize, 0 );
		if( SOCKET_ERROR == receiveSize || IsNeedReconnectWait || 1000 < NoSignalCheck )
		{
			int sockerror = WSAGetLastError();
			int winerror = GetLastError();

			closesocket( sessionOpen );
			sessionOpen = INVALID_SOCKET;

			if( 0 != ConnectInfoCreate( service, m_info ) )
				return 1;

			if( 0 != CreateSocket( m_socket, m_info, service ) )
				return 1;

			freeaddrinfo( m_info );


			printf("Ready For New Connect");
			if( 0 != WaitClient( m_socket, sessionOpen ) )
				return 1;

			counter0 = 0;
			start = time( NULL );
			IsNeedReconnectWait = false;
			NoSignalCheck = 0;
			continue;
		}
		if( 0 == receiveSize )
		{
			++NoSignalCheck;
			continue;
		}

		uint64_t ProcessLength = ProcessPacket( RecvBuffer, m_bufferSize, counter0, reserveSize, receiveSize, start );

		if( 0 != ProcessLength )
		{
			if( 0 != m_bufferSize )
			{
				memcpy( RecvBuffer, RecvBuffer + ProcessLength, m_bufferSize );
			}
		}

		printf( "[Process Size : %lld][m_bufferSize : %lld] \n", ProcessLength, m_bufferSize );

		if( UINT64_MAX - 10000 <= counter0 )
		{
			counter0 = 0;
			start = time( NULL );
		}
	}

	//===========================================================================================================================================
	
	//TestSock_Server server(50000);

	//if( 0 != server.CreateInitializeData() )
	//	return 1;

	//server.Binding();
	//CSession* session = server.Wating();

	//if( nullptr == session )
	//	return 1;

	//if( !session->IsConnected() )
	//{
	//	delete session;
	//}

	//while( 1 )
	//{
	//	if( !session->IsConnected() )
	//	{
	//		session = server.Wating();
	//		if( !session->IsConnected() )
	//		{
	//			server.CloseSession( session );
	//			return 1;
	//		}
	//	}

	//	session->Process();
	//}

	WSACleanup();

	//================================================================================================================================================================
}