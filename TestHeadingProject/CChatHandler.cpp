#include "psudoPCH.h"

namespace Heading
{
	void CChatHandler::Do_Process( SessionData* _data )
	{
		switch( _data->m_message->type )
		{
		case 1000:
			{
				ChatBuffer* cast = static_cast<ChatBuffer*>(_data->m_message);
				m_chatLog.push(ChatLogNode(cast->buffer));

				if( 1000 < m_chatLog.size() )
					m_chatLog.pop();
			}
			break;
		case 1001:
			{
				
			}
			break;
		}
	}
}
