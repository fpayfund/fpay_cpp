#ifndef DOUBLELINK_TRYSIMPLELOCK_H_
#define DOUBLELINK_TRYSIMPLELOCK_H_
#include <pthread.h>
namespace doublelink{

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
#endif// DOUBLELINK_TRYSIMPLELOCK_H_

