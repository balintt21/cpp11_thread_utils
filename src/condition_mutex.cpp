#include "condition_mutex.h"

thread_utils::ConditionMutex::ConditionMutex() 
    : mMutex()
    , mConditionVariable()
    , mSignal(false)
    , mState(false)
    , mWaitingThreadCount(0)
{}

thread_utils::ConditionMutex::~ConditionMutex()
{}

void thread_utils::ConditionMutex::lock()
{
    mMutex.lock();
    mState.store(true);
}

void thread_utils::ConditionMutex::unlock()
{
    mState.store(false);
    mMutex.unlock();
}

bool thread_utils::ConditionMutex::try_lock()
{ 
    if( mMutex.try_lock() )
    {
        mState.store(true);
        return true;
    } else {
        return false;
    }
}

void thread_utils::ConditionMutex::wait()
{
    std::unique_lock<std::mutex> locker(mMutex, std::adopt_lock);
    ++mWaitingThreadCount;
    mState.store(false);
    mConditionVariable.wait(locker, [&] { return mSignal; });
    mState.store(true);
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
    mState.store(false);
    bool waken = mConditionVariable.wait_for(locker, ms, [&] { return mSignal; });
    mState.store(true);
    --mWaitingThreadCount;
    if( mWaitingThreadCount.load() == 0 )
    { mSignal = false; }
    locker.release();
    return waken;
}

void thread_utils::ConditionMutex::notify_one()
{
    if( !mState.load() )
    { //locking only while setting mSignal to avoid waking the waiting thread only to block again
        std::unique_lock<std::mutex> locker(mMutex);
        mSignal = (mWaitingThreadCount.load() > 0);
    } else {
        mSignal = (mWaitingThreadCount.load() > 0);
    }
    mConditionVariable.notify_one();
}

void thread_utils::ConditionMutex::notify_all()
{
    if( !mState.load() )
    { //locking only while setting mSignal to avoid waking the waiting thread only to block again
        std::unique_lock<std::mutex> locker(mMutex);
        mSignal = (mWaitingThreadCount.load() > 0);
    } else {
        mSignal = (mWaitingThreadCount.load() > 0);
    }
    mConditionVariable.notify_all();
}
