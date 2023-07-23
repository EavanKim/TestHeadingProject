#include "psudoPCH.h"

namespace Heading
{
	void CChatBuffer::InsertDatas( uint64_t _key, Header* _data )
	{
		m_data.push_back( CChatData( _key, _data ) );
	}

	void CChatBuffer::GetDatas( uint64_t _key, std::vector<Header*>& _datas )
	{
		for( CChatData& data : m_data )
		{
			if( _key != data.event )
			{
				_datas.push_back( data.buffer );
			}
		}
	}

	void CChatBuffer::Clear( )
	{
		for( CChatData& data : m_data )
			delete data.buffer;

		m_data.clear( );
	}
}
