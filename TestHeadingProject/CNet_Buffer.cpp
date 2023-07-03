#include "psudoPCH.h"

CNet_Buffer::CNet_Buffer()
{
}

CNet_Buffer::CNet_Buffer( uint64_t _sessionKey )
{
	ZeroMemory( m_data, DEFAULT_SOCKET_BUFFER_LENGTH );
}

CNet_Buffer::CNet_Buffer( CNet_Buffer& _copy )
{
	m_sessionKey = _copy.m_sessionKey;
	m_seek = _copy.m_seek;
	memcpy_s( m_data, DEFAULT_SOCKET_BUFFER_LENGTH, _copy.m_data, DEFAULT_SOCKET_BUFFER_LENGTH );
}

CNet_Buffer::CNet_Buffer( CNet_Buffer&& _move )
{
	m_sessionKey = _move.m_sessionKey;
	m_seek = _move.m_seek;
	memcpy_s( m_data, DEFAULT_SOCKET_BUFFER_LENGTH, _move.m_data, DEFAULT_SOCKET_BUFFER_LENGTH );
}

CNet_Buffer::~CNet_Buffer()
{
}

void CNet_Buffer::commit( uint64_t _length )
{
	printf( "commit length : %lld \n", _length );
	m_dataSize += _length;
	m_seek = 0;
}

bool CNet_Buffer::get_buffer( char** _buffer, uint64_t* _length )
{
	if( DEFAULT_SOCKET_BUFFER_LENGTH > m_dataSize )
	{
		if( 0 != m_dataSize )
		{
			memcpy_s( m_data, m_dataSize, m_data + m_seek, m_dataSize );
			m_seek = 0;
		}

		*_length = DEFAULT_SOCKET_BUFFER_LENGTH - m_dataSize;
		*_buffer = m_data + m_dataSize;
		printf( "return length : %lld / [%llX / %llX] \n", *_length, m_data, *_buffer );
		return true;
	}

	return false;
}

Header* CNet_Buffer::get_data()
{
	Header* result = nullptr;
	if( m_dataSize >= sizeof( Header ) )
	{
		Header* getHeader = ( Header* )m_data + m_seek;

		if( m_dataSize >= getHeader->length )
		{
			switch( getHeader->type )
			{
				case 1:
					result = new SessionKey();
					break;
				case 2:
					result = new Shutdown();
					break;
				case 3:
					result = new Ping();
					break;
				case 100:
					result = new TestBuffer();
					break;
				case 1000:
					result = new ChatBuffer();
					break;
			}
		}
	}

	if( nullptr != result )
	{
		memcpy_s( result, result->length, m_data + m_seek, result->length );
		m_seek += result->length;
		m_dataSize -= result->length;
	}

	return result;
}

void CNet_Buffer::get_data( std::vector<Header*>* _datas )
{
	m_seek = 0;
	Header* parse = get_data();
	while( nullptr != parse )
	{
		printf("[seek : %lld][data] : %s \n", m_seek, (( SendStruct<0,0>* )parse)->buffer);
		_datas->push_back( parse );
		parse = get_data();
	}
}
