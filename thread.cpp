#include "thread.h"
#include <unistd.h>
#include <signal.h>

namespace thread_utils
{

void test_cancel()
{
    pthread_testcancel();
}

Thread::Thread(const std::string& name) 
    : mContext(new Thread::Context(name))
{
}

Thread::~Thread()
{
    if( mContext )
    {
        if( mContext->thread && mContext->thread->joinable() )
        { mContext->thread->detach(); }
        mContext.reset();
    }
}

bool Thread::run(const std::function<void ()>& function, const std::function<void ()>& on_cancel)
{
    if( !mContext ) return false;

    if( !mContext->state.load() )
    {
        std::lock_guard<std::mutex> guard(mContext->mutex);
        mContext->function = function;
        mContext->onCancelled = on_cancel;
        mContext->state.store(true);
        mContext->thread.reset(new std::thread(std::bind(&thread_utils::Thread::threadFunction, mContext)));
    }
    return false;
}

size_t Thread::getId() const noexcept
{
    std::hash<std::thread::id> hasher;
    if( !mContext ) { return hasher(std::this_thread::get_id()); }
    std::lock_guard<std::mutex> guard(mContext->mutex);
    return ( mContext->thread ) ? hasher(mContext->thread->get_id()) : hasher(std::this_thread::get_id());
}

std::string Thread::getName() const noexcept
{
    if( !mContext ) { return ""; }
    std::lock_guard<std::mutex> guard(mContext->mutex);
    return mContext->name;
}

bool Thread::joinable() const noexcept
{
    if( !mContext ) { return false; }
    std::lock_guard<std::mutex> guard(mContext->mutex);
    return ( mContext->thread ) ? mContext->thread->joinable() : false;
}

void Thread::join()
{
    if( !mContext ) { return; }
    std::lock_guard<std::mutex> guard(mContext->mutex);
    if( mContext->thread && mContext->state.load() && mContext->thread->joinable() ) 
    { mContext->thread->join(); }
}

void Thread::detach()
{
    if( !mContext ) { return; }
    std::lock_guard<std::mutex> guard(mContext->mutex);
    if( mContext->thread ) 
    { 
        if(  mContext->thread->joinable() )
        { mContext->thread->detach(); }
        mContext.reset();
    }
}

bool Thread::cancel()
{
    if( !mContext ) { return false; }
    std::lock_guard<std::mutex> guard(mContext->mutex);
    if( mContext->state.load() && mContext->thread )
    { return pthread_cancel(static_cast<pthread_t>(mContext->thread->native_handle())) == 0; }
    return false;
}

bool Thread::kill()
{
    if( !mContext ) { return false; }
    std::lock_guard<std::mutex> guard(mContext->mutex);
    if( mContext->state.load() && mContext->thread )
    { return pthread_kill(static_cast<pthread_t>(mContext->thread->native_handle()), SIGKILL) == 0; }
    return false;
}

bool Thread::setPriority(int32_t nice_value)
{
    if( !mContext ) { return false; }
    std::lock_guard<std::mutex> guard(mContext->mutex);
    if( !mContext->state.load() )
    { 
        mContext->niceValue = nice_value; 
        return true;
    }
    return false;
}

static inline bool setNiceValue(int value)
{
	errno = 0;
    int set_value = 0;
	int actual_value = nice(0);
	if( (errno == 0) && (actual_value != value)){
		set_value = nice(-actual_value);
		if(errno == 0){
			set_value = nice(value);
		}
	}
	return (errno == 0) && (set_value == value);
}

void Thread::threadFunction(std::shared_ptr<Thread::Context> context)
{
    if( context )
    {
        setNiceValue(context->niceValue);
        if( !context->name.empty() )
        { pthread_setname_np(static_cast<pthread_t>(context->thread->native_handle()), context->name.c_str()); }

        if( context->function )
        {
            context->function();
        }
        context->state.store(false);
    }
}

Thread::Context::Context(const std::string& _name)
    : mutex()
    , thread()
    , state(false)
    , function()
    , onCancelled()
    , niceValue(0)
    , name(_name)
{}

}//thread_utils end