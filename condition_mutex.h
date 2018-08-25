#ifndef _CONDITION_MUTEX_H_
#define _CONDITION_MUTEX_H_

#include <stdint.h>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>

namespace thread_utils
{
    class ConditionMutex final
    {
    private:
        mutable std::mutex      mMutex;
        std::condition_variable mConditionVariable;
        bool                    mSignal;
        uint32_t                mWaitingThreadCount;
        std::atomic<bool>       mState;
    public:
        ConditionMutex();
        ~ConditionMutex();
        /**
         * Lock the underlying mutex.
         */
        void lock();
        /**
         * Unlock the underlying mutex.
         */
        void unlock();
        /**
         * Try to lock the underlying mutex
         * @return On success true is returned, otherwise false.
         */
        bool try_lock();
        /**
         * Block the current thread until the underlying condition variable is woken up.
         */
        void wait();
        /**
         * Block the current thread until the condition variable is woken up or after the specified timeout duration
         * @param timeout_ms - Block the current thread for at least this time in milliseconds.
         * @return False is returned if it is not waken up before the given timeout expired. If the condition variable is woken up true is returned.
         */
        bool wait_for(int64_t timeout_ms);
        /**
         * Wake one blocking thread
         */
        void notify_one();
        /**
         * Wake all blocking threads
         */
        void notify_all();

        #if defined(DEBUG)
        /**
         * Tells if the mutex is in locked or unlocked state.
         */
        bool state() const;
        #endif
    };
}

typedef thread_utils::ConditionMutex condition_mutex_t;

#endif
