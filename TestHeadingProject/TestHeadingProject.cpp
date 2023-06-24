﻿// TestHeadingProject.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
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
	printf( "[%lld][count : %lld][MPS : %lld] \n", Time, count, MPS );
}

void ProcessPacket()
{

}

void ReadData( char* _buffer, uint64_t _totalrecvSize, uint64_t& _counter )
{
	uint64_t leftSize = _totalrecvSize;

	uint64_t seek = 0;

	while( 0 != leftSize )
	{
		if( leftSize < sizeof( Header ) )
			return;

		char* currPtr = _buffer + seek;
		Header getHeader = {};
		uint64_t type = 0;
		uint64_t packlength = 0;
		ParseHeader( currPtr, getHeader );

		switch( getHeader.type )
		{
			case 1:
			{
				if( leftSize < sizeof( TestBuffer ) )
					return;
				TestBuffer parseData = {};

				ParseData( currPtr, parseData );
				packlength = sizeof( TestBuffer );

				//printf( parseData.buffer );
			}
				break;
			default:
				printf("!!! Packet Parsing Failure !!! \n");
				return;
		}

		seek = seek + packlength;
		leftSize = leftSize - packlength;

		++_counter;
	}
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

	addrinfo createData = {};

	SecureZeroMemory( ( PVOID )&createData, sizeof( addrinfo ) );

	createData.ai_family = AF_INET;
	createData.ai_socktype = SOCK_STREAM;
	createData.ai_protocol = IPPROTO_TCP;
	createData.ai_flags = AI_PASSIVE;

	result = getaddrinfo( nullptr, "50000", &createData, &m_info );
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
	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = htonl( INADDR_ANY );
	service.sin_port = htons( 50000 );


	int loopCounter = 0;
	do
	{
		if( 5 < loopCounter )
		{
			int winerror = GetLastError();

			if( INVALID_SOCKET != m_socket )
			{
				closesocket( m_socket );
				m_socket = INVALID_SOCKET;
			}
			// exception 객체 생성되면 throw하면서 에러 정보 송신
			return 1;
		}

		m_socket = socket( m_info->ai_family, m_info->ai_socktype, m_info->ai_protocol );
		if( INVALID_SOCKET == m_socket )
		{
			continue;
		}

		result = bind( m_socket, ( SOCKADDR* )&service, sizeof( service ) );
		if( result == SOCKET_ERROR )
		{
			int err = 0;
			if( WSAECONNREFUSED == ( err = WSAGetLastError() ) )
			{
				closesocket( m_socket );
				m_socket = INVALID_SOCKET;
				continue;
			}
			wprintf( L"connect failed with error: %d\n", err );
			freeaddrinfo( m_info );
			closesocket( m_socket );
			return 1;
		}
	}
	while( S_OK != result );

	DWORD receiveSize = 0;

	freeaddrinfo( m_info );

	result = listen( m_socket, SOMAXCONN );
	if( result == SOCKET_ERROR )
	{
		printf( "listen failed with error: %d\n", WSAGetLastError() );
		closesocket( m_socket );
		WSACleanup();
		return 1;
	}

	SOCKET sessionOpen = INVALID_SOCKET;
	sessionOpen = accept( m_socket, NULL, NULL );
	if( sessionOpen == INVALID_SOCKET )
	{
		printf( "accept failed with error: %d\n", WSAGetLastError() );
		closesocket( m_socket );
		WSACleanup();
		return 1;
	}

	// No longer need server socket
	closesocket( m_socket );

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
			// 에러복구
			continue;
		}


		{
			time_t now = time( NULL );
			if( UINT64_MAX == counter0 )
			{
				counter0 = 0;
			}
			uint64_t Time = now - start;
			uint64_t Timedevide = ( 0 == Time ? 1 : Time );
			uint64_t MPS = counter0 / Timedevide;

			ProcessTime( start, counter0);
			printf( "[reserveSize : %lld][receiveSize : %lld][m_totalRecv : %lld]TestBuffer[% s] \n", reserveSize, receiveSize, m_totalRecv, RecvBuffer + reserveSize + 16 );
			//++counter0;
		}


		// 일단 상수로 먼저 짭니다.
		// 어차피 Sync는 테스트 용도에 가까우므로 이 이후에 제대로 작성해야합니다.
		m_totalRecv += receiveSize;
		printf("totalRecv : %lld \n", m_totalRecv);
		if( m_totalRecv < sizeof( Header ) )
		{
			continue;
		}
		uint64_t* recvBufLengPtr = ( uint64_t* )( RecvBuffer + reserveSize + 8 );
		uint64_t BufferLength = *recvBufLengPtr;
		m_currentpacketSize = 16 + BufferLength;
		//printf( "Debug MessagePtr [%llX] \n", RecvBuffer + reserveSize + 16 );
		//printf( "Debug Packetptr [%llX] \n", recvBufLengPtr );
		//printf( "Debug Packetlength [%lld][%llX] \n", BufferLength, BufferLength );
		//printf( "Debug PacketSize [%lld][%llX] \n", m_currentpacketSize, m_currentpacketSize );

		ReadData( RecvBuffer + reserveSize, m_totalRecv, counter0 );
	}

	//================================================================================================================================================================
}