#ifndef UTIL_H__
#define UTIL_H__

void PrintMem( char* _ptr, uint64_t _length )
{
	for( uint64_t seek = 0; _length > seek; ++seek )
	{
		printf( "%02X", _ptr[ seek ] );
	}
	printf( "\n" );
}

void ParseHeader( char* _buffer, Header& _parse )
{
	memcpy( &_parse, _buffer, sizeof( Header ) );
}

void ParseData( char* _buffer, TestBuffer& _parse )
{
	memcpy( &_parse, _buffer, sizeof( TestBuffer ) );
}
#endif