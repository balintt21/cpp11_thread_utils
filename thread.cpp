#include "thread.h"
#include <unistd.h>
#include <signal.h>
#include <syscall.h>
#include <sys/time.h>
#include <sys/resource.h>

static bool global_term_sig_handler_registered = false;

struct CleanupContext
{
    std::function<void ()> cleanupFunction;
    CleanupContext(const std::function<void ()>& fn) : cleanupFunction(fn) {}
};

static void generalCleanupHandler(void * arg)
{
    CleanupContext* context = reinterpret_cast<CleanupContext*>(arg);
    if( context->cleanupFunction )
    {
        context->cleanupFunction();
    }
    delete context;
}

static void generalSignalHandler(int signum, siginfo_t * siginfo, void * arg)
{
    int retval;
    pthread_exit(&retval);
}

static int setSignalHandler(const int signum)
{
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_sigaction = generalSignalHandler;
    act.sa_flags = SA_SIGINFO;
    if (sigaction(signum, &act, NULL))
        return errno;
    return 0;
}

namespace thread_utils
{

void test_cancel()
{
    pthread_testcancel();
}

Thread::Thread(const std::string& name) 
    : mContext(new Thread::Context(name))
{
    if( !global_term_sig_handler_registered )
    {
        global_term_sig_handler_registered = (setSignalHandler(SIGUSR2) == 0);
        #warning SIGUSR2 is used for killing threads! Disable this line if it is acceptable or use another signal at kill() and above this line
    }
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

size_t Thread::id() const noexcept
{
    std::hash<std::thread::id> hasher;
    if( !mContext ) { return hasher(std::this_thread::get_id()); }
    std::lock_guard<std::mutex> guard(mContext->mutex);
    return ( mContext->thread ) ? hasher(mContext->thread->get_id()) : hasher(std::this_thread::get_id());
}

std::string Thread::name() const noexcept
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
    { return pthread_kill(static_cast<pthread_t>(mContext->thread->native_handle()), SIGUSR2) == 0; }
    return false;
}

bool Thread::setPriority(int32_t nice_value)
{
    if( !mContext ) { return false; }
    std::lock_guard<std::mutex> guard(mContext->mutex);
    mContext->niceValue = nice_value;
    if( mContext->pid != 0 )
    { 
        return setpriority(PRIO_PROCESS, mContext->pid, nice_value) == 0;
    } 
    return true;
}

void Thread::threadFunction(std::shared_ptr<Thread::Context> context)
{
    if( context )
    {
        {
            std::lock_guard<std::mutex> guard(context->mutex);
            context->pid = static_cast<pid_t>(syscall(SYS_gettid));
            setpriority(PRIO_PROCESS, 0, context->niceValue);
        }

        if( !context->name.empty() )
        { pthread_setname_np(static_cast<pthread_t>(context->thread->native_handle()), context->name.c_str()); }

        if( context->onCancelled )
        {
            int old_cancel_state = 0;
            pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &old_cancel_state);
        }

        pthread_cleanup_push(&generalCleanupHandler, new CleanupContext(context->onCancelled));

        if( context->function )
        {
            context->function();
        }

        pthread_cleanup_pop(1);

        {
            std::lock_guard<std::mutex> guard(context->mutex);
            context->pid = 0;
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
    , pid(0)
    , name(_name)
{}

}//thread_utils end