#pragma once


class PrintLog
{
public:
	PrintLog();
	~PrintLog();

	void InsertLog( std::string _logString );

	template<typename ... Args>
	void InsertLog( const std::string& format, Args ... args )
	{
		InsertLog( string_format( format, args ... ) );
	}

	void FlushLog();

private:
	// web에서 복붙입니다.
	// utf8로 처리하면 한글도 나올 것입니다.
	template<typename ... Args>
	std::string string_format( const std::string& format, Args ... args )
	{
		int size_s = std::snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
		if( size_s <= 0 ) { throw std::runtime_error( "Error during formatting." ); }
		auto size = static_cast< size_t >( size_s );
		std::unique_ptr<char[]> buf( new char[ size ] );
		std::snprintf( buf.get(), size, format.c_str(), args ... );
		return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
	}

	SimpleLock_Win m_lock;
	std::vector<std::string> m_logs = {};
	std::vector<std::string> m_flush = {};
};
