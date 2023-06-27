#include "psudoPCH.h"

CSocket::CSocket( std::string _ip, std::string _port )
	: m_ip( _ip )
	, m_port( _port )
{
	// 상속한 서버형 소켓에서 잘못 초기화 되지 않도록
	// 값 초기화 외의 아무런 동작도 하면 안됩니다.
	ZeroMemory( dataBuffer, DEFAULT_SOCKET_BUFFER_LENGTH );
}

CSocket::CSocket( SOCKET _socket )
	: m_socket( _socket )
{
	ZeroMemory( dataBuffer, DEFAULT_SOCKET_BUFFER_LENGTH );
}

CSocket::~CSocket()
{
	C_Disconnect();
}

// 계정 정보가 없으니 일단 포트정보로 계정 정보처럼 쓰고
// 그 번호를 세션키로 일단 사용합니다.
void CSocket::CreateSessionKey()
{
	struct sockaddr_in  addr;
	int length = sizeof( addr );
	int result = getsockname( m_socket, (struct sockaddr*)&addr, &length);
	if( S_OK == result )
	{
		m_sessionKey = ntohs( addr.sin_port );
	}
}

bool CSocket::C_Connect()
{
	int result = 0;
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
			return false;
		}

		m_socket = socket( m_info->ai_family, m_info->ai_socktype, m_info->ai_protocol );
		if( INVALID_SOCKET == m_socket )
		{
			continue;
		}

		result = connect( m_socket, m_info->ai_addr, ( int )m_info->ai_addrlen );
		if( result == SOCKET_ERROR )
		{
			int err = 0;
			if( WSAECONNREFUSED == ( err = WSAGetLastError() ) )
			{
				closesocket( m_socket );
				m_socket = INVALID_SOCKET;
			}
			PrintWork( "connect failed with error: %d\n", err );
			freeaddrinfo( m_info );
			continue;
		}
	}
	while( S_OK != result );

	// 연결되면 세션키를 전송합니다.
	// 어디까지나 로컬에서만 가능한 헛짓입니다.
	// 왜냐하면 기본적으로 클라이언트 소켓과 포트는 1:1로 묶기 때문입니다.
	CreateSessionKey();

	SessionKey keySend;
	keySend.sessionKey = m_sessionKey;
	C_Send( &keySend );

	return true;
}

int CSocket::C_Send( Header* _data )
{
	return send( m_socket, ( char* )_data, _data->length, 0 );
}

int CSocket::C_Recv()
{
	return recv( m_socket, dataBuffer, DEFAULT_SOCKET_BUFFER_LENGTH, 0 );
}

bool CSocket::C_Disconnect()
{
	if( INVALID_SOCKET == m_socket )
	{
		closesocket( m_socket );
		m_socket = INVALID_SOCKET;
		freeaddrinfo( m_info );
	}

	return true;
}

void CSocket::CreateInitializeData()
{
	addrinfo createData = {};

	SecureZeroMemory( ( PVOID )&createData, sizeof( addrinfo ) );

	createData.ai_family = AF_UNSPEC;
	createData.ai_socktype = SOCK_STREAM;
	createData.ai_protocol = IPPROTO_TCP;

	int result = getaddrinfo( "127.0.0.1", "50000", &createData, &m_info );
	if( S_OK != result )
	{
		// 초기화 실패
		int sockerror = WSAGetLastError();
		int winerror = GetLastError();
		// exception 객체 생성되면 throw하면서 에러 정보 송신
		throw "CSocket::CreateInitializeData()";
	}
}
