#pragma once

namespace Heading
{
	class CChatter : public CSelecter
	{
	public:
		struct ChatData
		{
			uint64_t Key;
		};

		CChatter( );
		virtual ~CChatter( );

		virtual void Set_NewSession( NewSocketList& _newSocket ) override;

		void Do_PostProcess( );

	private:

		CChatBuffer m_chat;
	};
}

