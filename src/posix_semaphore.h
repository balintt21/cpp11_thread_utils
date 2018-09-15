#ifndef _POSIX_SEMAPHORE_H_
#define _POSIX_SEMAPHORE_H_

#include <stdint.h>
#include <semaphore.h>

namespace thread_utils
{
    class PosixSemaphore final
    {
    public:
        PosixSemaphore(uint32_t initial_value = 0)  { sem_init(&mSemaphore, 0/*share between threads*/, initial_value); }
        ~PosixSemaphore()                           { sem_destroy(&mSemaphore); }
        /**
         * Increments (unlocks) the semaphore.
         */
        inline void post()                          { sem_post(&mSemaphore); }
        /**
         * Alias for post()
         */
        inline void notify()                        { post(); }
        /**
         * Alias for post()
         */
        inline void signal()                        { post(); }
        /**
         * Returns the current value of the semaphore
         */
        inline int32_t value()                      { int val;sem_getvalue(&mSemaphore, &val);return val; }
        /**
         * Decrements (locks) the semaphore.
         * If the semaphore currently has the value zero, then the call BLOCKS until either it 
         * becomes possible to perform the decrement or a signal handler interrupts the call
         */
        inline void wait()                          { sem_wait(&mSemaphore); }
        /* NOTE: wait_for() is not implemented because sem_timedwait requires absolute time 
         * since the Epoch, 1970-01-01 00:00:00 +0000 (UTC). A sem_timedwait call may exit errno
         * ETIMEDOUT earlier or later than it should if the system clock is adjusted ahead. 
         */
    private:
        sem_t mSemaphore;
    };
}

#endif