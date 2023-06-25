#include "psudoPCH.h"

SimpleLock_Win::SimpleLock_Win()
{
    m_lock = 0;
}

SimpleLock_Win::~SimpleLock_Win()
{
}

void SimpleLock_Win::lock( LONG64 _spinWait )
{
    // Stack Scope에 개별적으로 생기므로,
    // Thread별 Stack에서 잠금대기 진입 시 생겨납니다.
    // spinlock 대기가 길어질수록, 동작이 끝난다음 후폭풍이 옵니다.
    // 적절히 끊어주세요
    volatile LONG64 counter = 0;
    while( 0 != InterlockedCompareExchange64( &m_lock, 1, 0 ) )
    {
        if( _spinWait == InterlockedCompareExchange64(&counter, _spinWait, _spinWait ) )
        {
            Sleep(1);
            InterlockedExchange64( &counter, 0 );
        }
        InterlockedIncrement64( &counter );
    }
}

void SimpleLock_Win::unlock()
{
    InterlockedExchange64( &m_lock, 0 );
}

bool SimpleLock_Win::trylock()
{
    if( 0 == InterlockedCompareExchange64( &m_lock, 1, 0 ) )
        return true;

    return false;
}
