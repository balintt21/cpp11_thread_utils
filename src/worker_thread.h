#ifndef _WORKER_THREAD_H_
#define _WORKER_THREAD_H_

#include <atomic>

#include "thread.h"

namespace thread_utils
{
    class WorkerThread
    {
    private:
        std::atomic_bool    mIsRunning;
        Thread              mThread;
    public:
        WorkerThread(const std::string& name) : mIsRunning(false), mThread(name) {}

        inline bool start(const std::function<bool (std::atomic_bool& is_running)>& loop_function)
        {
            mIsRunning.store(true);
            return mThread.run([loop_function, this]()
            {
                while(mIsRunning.load())
                {
                    if( !loop_function(mIsRunning) ) 
                    {
                        mIsRunning.store(false); 
                        break; 
                    }
                }
            });
        }

        inline void stop(bool wait = false)
        {
            mIsRunning.store(false);
            if( wait ) 
            { mThread.join(); }
        }

        inline bool isRunning() const { return mIsRunning.load(); }

        inline Thread& thread() { return mThread; }
    };
}

#endif
