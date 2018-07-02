#include "TrySimpleLock.h"
#include <cassert>
#include <new>
#include <stdexcept>

namespace doublelink{

TrySimpleLock::TrySimpleLock()
{
    int err = ::pthread_mutex_init( &mutex_, NULL );
    assert( !err );
    if ( err )
    {
        throw std::bad_alloc();
    }
}

TrySimpleLock::~TrySimpleLock()
{
#ifndef NDEBUG
    int err =
#endif// #ifndef NDEBUG
    ::pthread_mutex_destroy( &mutex_ );
#ifndef NDEBUG
    assert( !err );
#endif// #ifndef NDEBUG
}

bool TrySimpleLock::TryLock()
{

    int err = ::pthread_mutex_trylock( &mutex_ );

    if ( err )
    {
        return false;
    }
    return true;
}

void TrySimpleLock::Lock()
{
  int err = ::pthread_mutex_lock( &mutex_ );
  assert( !err );
  if ( err )
  {
    throw std::runtime_error( "Failed to lock." );
  }
}

void TrySimpleLock::Unlock()
{

#ifndef NDEBUG
    int err =
#endif// #ifndef NDEBUG  
    ::pthread_mutex_unlock( &mutex_ );
#ifndef NDEBUG
    assert( !err );
#endif// #ifndef NDEBUG  

}

}

