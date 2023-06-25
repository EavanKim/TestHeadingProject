#include <stdint.h>
#include <time.h>
#include <iostream>

#include "define.h"
#include "CSession.h"

extern void PrintMem( char* _ptr, uint64_t _length );

CSession::CSession( SOCKET _session )
	: m_session( _session )
{
	m_currentSIze = 0;
	ZeroMemory( m_RecvBuffer, sizeof( m_RecvBuffer ) );
}

CSession::~CSession()
{
	TerminateConnection();
}

void CSession::Process()
{
	uint64_t seek = 0;
	int readcount = recv(m_session, m_RecvBuffer + m_currentSIze, m_bufferSIze - m_currentSIze, 0 );
	if( -1 == readcount )
	{
		ErrorProcess();
		return;
	}
	if( 0 == readcount )
	{
		ZeroReceiveProcess();
		return;
	}

	m_currentSIze = m_currentSIze + readcount;

	if( m_currentSIze < sizeof( Header ) )
		return;

	while( 0 != m_currentSIze )
	{
		if( m_currentSIze < sizeof( Header ) )
			break;

		char* currPtr = m_RecvBuffer + seek;
		uint64_t type = 0;
		Header* m_header = ( Header* )currPtr;

		switch( m_header->type )
		{
			case 2:
				TerminateConnection();
				break;
			case 100:
			{
				++m_processCounter;
				if( m_currentSIze < sizeof( TestBuffer ) )
					break;

				TestBuffer* parseData = ( TestBuffer* )currPtr;

				seek = seek + parseData->length;
				m_currentSIze = m_currentSIze - parseData->length;

				PrintTimInfo();
				printf( "[Count : %lld] %s \n", m_processCounter, parseData->buffer );
			}
			break;
			default:
				PrintMem(currPtr, sizeof( Header ) );
				printf( "!!! Packet Parsing Failure !!! [type : %lld] \n", m_header->type );
				break;
		}
	}

	printf("BufferSize %lld \n", m_currentSIze );
	if( 0 != m_currentSIze )
	{
		memcpy(m_RecvBuffer, m_RecvBuffer + seek, m_currentSIze );
	}
}

void CSession::ErrorProcess()
{
	TerminateConnection();
}

void CSession::ZeroReceiveProcess()
{
	++m_zeroRecvCount;

	if( 1000 > m_zeroRecvCount )
	{
		TerminateConnection();
	}
}

bool CSession::IsConnected()
{
	if( INVALID_SOCKET == m_session )
	{
		return false;
	}

	if( SOCKET_ERROR == m_session )
	{
		return false;
	}

	return true;
}

void CSession::TerminateConnection()
{
	if( INVALID_SOCKET != m_session )
	{
		closesocket( m_session );
		m_session = INVALID_SOCKET;
	}
}

void CSession::PrintTimInfo()
{
	m_currentTime = time(NULL);
	uint64_t SessionTime = m_currentTime - m_startTime;
	uint64_t MPS = m_processCounter / ( 0 == SessionTime ? 1 : SessionTime);
	printf("[%lld][MPS : %lld] ", SessionTime, MPS );
}

