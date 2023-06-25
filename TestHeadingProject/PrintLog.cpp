#include "psudoPCH.h"

PrintLog::PrintLog()
{
	m_logs.reserve( 1024 );
	m_flush.reserve( 1024 );
}

PrintLog::~PrintLog()
{
}

void PrintLog::InsertLog( std::string _logString )
{
	// 로그를 남기러 진입하면 무조건 남깁니다.
	m_lock.lock();

	m_logs.push_back( _logString );

	m_lock.unlock();
}

void PrintLog::FlushLog()
{
	if( m_lock.trylock() )
	{
		m_flush.swap(m_logs);
		m_lock.unlock();
	}

	for( std::string& flushLog : m_flush )
	{
		printf(flushLog.c_str());
	}

	m_flush.clear();
	m_flush.reserve( 1024 );
}
