#ifndef _BLOCKING_QUEUE_H_
#define _BLOCKING_QUEUE_H_

#include "semaphore.h"
#include <queue>
#include <mutex>

#if __cplusplus == 201703L
#include <optional>
#endif

namespace thread_utils
{
    template<typename T>
    class BlockingQueue
    {
    private:
        std::mutex      mMutex;
        std::queue<T>   mQueue;
        semaphore_t     mQueueSemaphore;
    public:
        BlockingQueue() {}
        void push(const T& element)
        {
            std::lock_guard<std::mutex> guard(mMutex);
            mQueue.push(element);
            mQueueSemaphore.post();
        }
        /* BasicLockable implementation*/
        void lock() { mMutex.lock(); }
        
        void unlock() { mMutex.unlock(); }
        /* Non-thread safe member functions*/
        T& front() { return mQueue.front(); }

        T& back() { return mQueue.back(); }

        bool empty() { return mQueue.empty(); }

        void pop() { return mQueue.pop(); }

        /* Thread-safe member functions*/
        /**
         * Blocks the current thread while the queue is empty or after the specified timeout duration
         * @param timeout_ms - Timeout in milliseconds (-1 means forever)
         * @return False is returned if the queue is empty and timed out, otherwise true is returned
         */
        bool wait(int64_t timeout_ms = 1000) 
        {
            if( timeout_ms >= 0 )
            { return mQueueSemaphore.wait_for(timeout_ms); } 
            else 
            {
                mQueueSemaphore.wait();
                return true;
            }
        }

        #if __cplusplus == 201703L
        /**
         * Pops and returns the last element. Blocks if there is no element in the queue
         */
        std::optional<T> take(int64_t timeout_ms = 1000)
        {
            if( mQueueSemaphore.wait_for(timeout_ms) )
            {
                std::lock_guard<std::mutex> guard(mMutex);
                T element = mQueue.front();
                mQueue.pop();
                return std::make_optional<T>(element);
            } else {
                return std::nullopt;
            }
        }
        #else
        /**
         * Pops and returns the last element. Blocks if there is no element in the queue
         */
        T take(int64_t timeout_ms = 1000)
        {
            if( mQueueSemaphore.wait_for(timeout_ms) )
            {
                std::lock_guard<std::mutex> guard(mMutex);
                T element = mQueue.front();
                mQueue.pop();
                return element;
            } else {
                return T();
            }
        }
        #endif
    };
}

#endif
