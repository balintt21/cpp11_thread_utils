#include "condition_mutex.h"

thread_utils::ConditionMutex::ConditionMutex() 
    : mMutex()
    , mConditionVariable()
    , mSignal(false)
    , mWaitingThreadCount(0)
{}

thread_utils::ConditionMutex::~ConditionMutex()
{}

void thread_utils::ConditionMutex::lock()
{
    mMutex.lock();
}

void thread_utils::ConditionMutex::unlock()
{
    mMutex.unlock();
}

bool thread_utils::ConditionMutex::try_lock()
{ 
    return mMutex.try_lock();
}

void thread_utils::ConditionMutex::wait()
{
    std::unique_lock<std::mutex> locker(mMutex, std::adopt_lock);
    ++mWaitingThreadCount;
    mConditionVariable.wait(locker, [&] { return mSignal; });
    --mWaitingThreadCount;
    if( mWaitingThreadCount == 0 )
    { mSignal = false; }
    locker.release();
}

bool thread_utils::ConditionMutex::wait_for(int64_t timeout_ms)
{ 
    std::unique_lock<std::mutex> locker(mMutex, std::adopt_lock);
    ++mWaitingThreadCount;
    std::chrono::milliseconds ms{timeout_ms};
    bool waken = mConditionVariable.wait_for(locker, ms, [&] { return mSignal; });
    --mWaitingThreadCount;
    if( mWaitingThreadCount == 0 )
    { mSignal = false; }
    locker.release();
    return waken;
}

void thread_utils::ConditionMutex::notify_one()
{
    { //locking only while setting mSignal to avoid waking the waiting thread only to block again
        std::unique_lock<std::mutex> locker(mMutex);
        mSignal = true;
    }
    mConditionVariable.notify_one();
}

void thread_utils::ConditionMutex::notify_all()
{
    { //locking only while setting mSignal to avoid waking the waiting thread only to block again
        std::unique_lock<std::mutex> locker(mMutex);
        mSignal = true;
    }
    mConditionVariable.notify_all();
}
