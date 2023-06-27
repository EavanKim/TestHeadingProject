#pragma once
class CSocket
{
public:
	CSocket( std::string _ip, std::string _port );
	CSocket( SOCKET _socket );
	virtual ~CSocket();

	void CreateSessionKey();
	virtual bool C_Connect();
	virtual int C_Send( Header* _data );
	virtual int C_Recv();
	virtual bool C_Disconnect();

	template<typename ... Args>
	void PrintWork( const std::string& format, Args ... args )
	{
		if( m_printOut )
		{
			m_printOut->InsertLog( format, args ... );
		}
		else
		{
			printf( format.c_str(), args ... );
		}
	}

protected:
	virtual void CreateInitializeData();
	
	uint64_t m_sessionKey = 0;
	ENUM_SESSION_TYPE m_sessionType = ENUM_SESSION_SYNC;
	std::string m_ip = "";
	std::string m_port = "";
	addrinfo* m_info = nullptr;
	SOCKET m_socket = INVALID_SOCKET;
	char dataBuffer[ DEFAULT_SOCKET_BUFFER_LENGTH ];
	PrintLog* m_printOut = nullptr;
};

