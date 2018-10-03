#include "thread.h"
#include <unistd.h>
#include <signal.h>
#include <syscall.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <chrono>
#include <atomic>

static bool global_term_sig_handler_registered = false;

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

Thread::CleanupContext::CleanupContext(const std::shared_ptr<thread_utils::Thread::Context>& ctx) : context(ctx) {}

void Thread::generalCleanupHandler(void * arg)
{
    CleanupContext* cleanupContext = reinterpret_cast<CleanupContext*>(arg);
    if( cleanupContext->context )
    {
        if( cleanupContext->context->onCancelled )
        { cleanupContext->context->onCancelled(); }
        cleanupContext->context->state.store(false);
    }
    delete cleanupContext;
}

void sleepFor(int64_t milliseconds)
{
    std::chrono::milliseconds ms{milliseconds};
    std::this_thread::sleep_for(ms);
}

void sleepUntil(int64_t timestamp_ms)
{
    int64_t now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()).time_since_epoch().count();
    sleepFor(timestamp_ms - now);
}

void testCancel()
{
    pthread_testcancel();
}

Thread::Thread(const std::string& name) 
    : mContextMutex()
    , mContext(new Thread::Context(name))
    , mName(name)
{
    if( !global_term_sig_handler_registered )
    {
        global_term_sig_handler_registered = (setSignalHandler(SIGUSR2) == 0);
        #warning SIGUSR2 is used for killing threads! Disable this line if it is acceptable or use another signal at kill() and above this line
    }
}

Thread::~Thread()
{
    auto context = getContext();
    if( context )
    {
        if( context->thread && context->thread->joinable() )
        { context->thread->detach(); }
        resetContext(nullptr);
    }
}

bool Thread::run(const std::function<void ()>& function, const std::function<void ()>& on_cancel)
{
    std::lock_guard<std::mutex> concurent_detach_or_run_guard(mContextMutex);

    auto context = getContext();
    if( !context || (context && !context->state.load()) ) //not detached and is not running
    {
        context.reset();
        resetContext(std::make_shared<Context>(mName));
        //created new context
        mContext->function = function;
        mContext->onCancelled = on_cancel;
        mContext->thread.reset(new std::thread(std::bind(&thread_utils::Thread::threadFunction, mContext)));
        mContext->state.store(true);
        return true;
    }
    return false;
}

std::string Thread::name() const noexcept
{
    return mName;
}

bool Thread::joinable() const noexcept
{
    auto context = getContext();
    return ( context && context->state.load() && context->thread && context->thread->joinable() );
}

void Thread::join()
{
    auto context = getContext();
    if( context && context->state.load() && context->thread && context->thread->joinable() ) 
    {
        context->thread->join();
    }
}

void Thread::detach()
{
    std::lock_guard<std::mutex> concurent_detach_or_run_guard(mContextMutex);

    auto context = getContext();
    if( context ) //not detached
    { 
        if( context->thread && context->thread->joinable() )
        { context->thread->detach(); }
        resetContext(nullptr);
    }
}

bool Thread::cancel()
{
    auto context = getContext();
    if( context && context->state.load() )//not detached and running
    { 
        if(context->pid > 0)
        {
            return (pthread_cancel(static_cast<pthread_t>(context->nativeHandle)) == 0);
        } else {
            context->killed = true;
            return true;
        }
    }
    return false;
}

bool Thread::kill()
{
    auto context = getContext();
    if( context && context->state.load() )//not detached and running
    { 
        std::lock_guard<std::mutex> guard(context->mutex);
        if( context->pid > 0 )
        {
            return pthread_kill(static_cast<pthread_t>(context->nativeHandle), SIGUSR2) == 0; 
        } else {
            context->killed = true;
            return true;
        }
    }
    return false;
}

bool Thread::setPriority(int32_t nice_value)
{
    auto context = getContext();
    if( context && context->state.load() )
    {
        std::lock_guard<std::mutex> guard(context->mutex);
        context->niceValue = nice_value;
        if( context->pid > 0 )
        {
            //set priority if thread already started
            return (setpriority(PRIO_PROCESS, context->pid, nice_value) == 0);
        } else {
            //priority will be set
            return true;
        }
    }
    return false;
}

void Thread::threadFunction(std::shared_ptr<Thread::Context> context)
{
    if( context )
    {
        {
            std::lock_guard<std::mutex> guard(context->mutex);
            if( context->killed ) //killed
            {
                if( context->onCancelled ) { context->onCancelled(); }
                context->state.store(false);
                return;
            }
            context->nativeHandle = context->thread->native_handle();
            context->pid = static_cast<pid_t>(syscall(SYS_gettid));
            setpriority(PRIO_PROCESS, 0, context->niceValue);
        }

        if( !context->name.empty() )
        { pthread_setname_np(static_cast<pthread_t>(context->thread->native_handle()), context->name.c_str()); }

        int old_cancel_state = 0;
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &old_cancel_state);

        pthread_cleanup_push(&generalCleanupHandler, new CleanupContext(context));

        if( context->function )
        {
            context->function();
        }

        pthread_cleanup_pop(1);
    }
}

Thread::Context::Context(const std::string& _name)
    : mutex()
    , pid(0)
    , nativeHandle(0)
    , killed(false)
    , thread()
    , state(false)
    , function()
    , onCancelled()
    , niceValue(0)
    , name(_name)
{}

}//thread_utils end
