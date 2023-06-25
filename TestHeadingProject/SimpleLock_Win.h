#pragma once

class SimpleLock_Win
{
public:
	SimpleLock_Win();
	~SimpleLock_Win();

	void lock( LONG64 _spinWait = 10 );
	void unlock();

	bool trylock();
private:
	volatile LONG64 m_lock;
};

