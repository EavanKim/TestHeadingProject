#pragma once

namespace Heading
{
	struct CChatData
	{
		CChatData(uint64_t evt, Header* _buf)
			: event(evt)
			, buffer(_buf)
		{
		}

		uint64_t event;
		Header* buffer;
	};

	class CChatBuffer
	{
	public:
		void InsertDatas( uint64_t _key, Header* _data );
		void GetDatas( uint64_t _key, std::vector<Header*>& _datas );
		void Clear( );

	private:
		std::vector<CChatData> m_data;
	};
}

