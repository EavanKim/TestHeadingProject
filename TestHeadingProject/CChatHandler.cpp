#include "psudoPCH.h"

void CChatHandler::Do_Process( Heading::SessionData* _data )
{
	switch( _data->m_message->type )
	{
	case 1000:
	{
		Heading::ChatBuffer* cast = static_cast< Heading::ChatBuffer* >( _data->m_message );
		m_chatLog.push( ChatLogNode( cast->buffer ) );

		if( 1000 < m_chatLog.size( ) )
			m_chatLog.pop( );
	}
	break;
	case 1001:
	{

	}
	break;
	}
}
