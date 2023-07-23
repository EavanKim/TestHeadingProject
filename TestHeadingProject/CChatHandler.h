#pragma once

namespace Heading
{
	struct ChatLogNode
	{
		ChatLogNode( std::string _string )
			: m_time64( time(NULL) )
			, m_chat( _string )
		{

		}

		__time64_t m_time64 = 0;
		std::string m_chat = "";
	};

	class CChatHandler : public CPacketHandler
	{
	public:
		// Inherited via CSubProcess
		virtual void Do_Process( SessionData* _data ) override;

	private:
		uint64_t m_size;
		std::queue<ChatLogNode> m_chatLog = {};
	};
}

