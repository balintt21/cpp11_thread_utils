#include "condition_mutex.h"
#include <cassert>

#if !defined(DEBUG)
#   define NDEBUG
#endif

thread_utils::ConditionMutex::ConditionMutex() 
    : mMutex()
    , mConditionVariable()
    , mSignal(false)
    , mWaitingThreadCount(0)
    , mState(false)
{}

thread_utils::ConditionMutex::~ConditionMutex()
{
    assert( !mState.load() );
}

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
    if( !mState.load() ){
        return mMutex.try_lock();
    }
    return false; 
}

void thread_utils::ConditionMutex::wait()
{
    std::unique_lock<std::mutex> locker(mMutex, std::adopt_lock);
    ++mWaitingThreadCount;
    mState = false;
    mConditionVariable.wait(locker, [&] { return mSignal; });
    mState = true;
    --mWaitingThreadCount;
    if( mWaitingThreadCount == 0 )
    { mSignal = false; }
    locker.release();
}

bool thread_utils::ConditionMutex::wait_for(int64_t timeout_ms)
{ 
    using namespace std::chrono_literals;
    std::unique_lock<std::mutex> locker(mMutex, std::adopt_lock);
    ++mWaitingThreadCount;
    mState = false;
    bool waken = mConditionVariable.wait_for(locker, timeout_ms * 1ms, [&] { return mSignal; });
    mState = true;
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

#if defined(DEBUG)
bool thread_utils::ConditionMutex::state() const
{
    return mState.load();
}
#endif
