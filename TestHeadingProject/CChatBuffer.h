#pragma once

struct CChatData
{
	CChatData( uint64_t evt, Heading::Header* _buf )
		: event( evt )
		, buffer( _buf )
	{
	}

	uint64_t event;
	Heading::Header* buffer;
};

class CChatBuffer
{
public:
	void InsertDatas( uint64_t _key, Heading::Header* _data );
	void GetDatas( uint64_t _key, std::vector<Heading::Header*>& _datas );
	void Clear( );

private:
	std::vector<CChatData> m_data;
};

