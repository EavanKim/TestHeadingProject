#pragma once
class CChatUser
{
public:
	void Add( WSAEVENT _handle, std::string _userName );

	bool Get( IN std::string _userName, OUT WSAEVENT& _result );
	bool Get( IN WSAEVENT _handle, OUT std::string _result );

	void remove( std::string _userName );
	void remove( WSAEVENT _handle );

	void clear();

private:
	concurrency::concurrent_unordered_map< WSAEVENT, std::string >	m_handleTable;
	concurrency::concurrent_unordered_map< std::string, WSAEVENT >	m_userTable;
};

