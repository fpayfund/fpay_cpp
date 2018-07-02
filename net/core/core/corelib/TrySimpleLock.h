#ifndef TRYSIMPLELOCK_H_
#define TRYSIMPLELOCK_H_
#include <pthread.h>
namespace core{

class TrySimpleLock
{
public:
    TrySimpleLock();
    ~TrySimpleLock();

    bool TryLock();
    void Lock();
    void Unlock();
  
private:
    pthread_mutex_t mutex_;

};

}
#endif// TRYSIMPLELOCK_H_

