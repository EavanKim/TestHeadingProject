#pragma once
class CSocket_Listen : public CSocket
{
public:
	CSocket_Listen( uint64_t _ndfnum, uint16_t _port );
	virtual ~CSocket_Listen();


	virtual bool C_Connect() override;
	virtual int C_Send( Header* _data ) override;
	virtual int C_Recv() override;
	virtual bool C_Disconnect() override;

	void Select( std::vector<CMessage>& _receiveDatas );
	void Do( SOCKET _sock, std::vector<CMessage>& _receiveDatas );
	void BroadCasting( CMessage* _data );

protected:
	virtual void CreateInitializeData() override;

private:
	void CreateNewSession();
	bool ListenBind();
	uint64_t m_ndfnum = 0;
	uint16_t m_listenPort = 0;
	sockaddr_in m_listenIn = {};
	sockaddr_in m_acceptIn = {};
	fd_set m_readfds = {};
	std::unordered_map<SOCKET, CSession*> m_sessionMap;
	SOCKET m_broadCast = INVALID_SOCKET;
};

