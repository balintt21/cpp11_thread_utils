#ifndef _THREAD_UTILS_THREAD_H_
#define _THREAD_UTILS_THREAD_H_

#include <string>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include <memory>
#include <vector>

namespace thread_utils
{
    /*
     * Sleeps the current thread for the given amount of time in milliseconds
     * @param milliseconds
     */
    void sleepFor(int64_t milliseconds);
    /*
     * Sleeps the current thread until the given monotonic time in milliseconds
     * @param timestamp_ms
     */
    void sleepUntil(int64_t timestamp_ms);
    /**
     * Calling testCancel() creates a cancellation point within the calling thread.
     * If the calling thread is canceled as a consequence of a call to this function, then the function does not return.
     * Instead an 'on exit' event callback is invoked. See Thread::run() 
     */
    void testCancel();

    class Thread final
    {
    public:
        Thread(const std::string& name);
        ~Thread();
        /**
         * A new thread of execution starts executing the given @p function.
         * After the given @p function finished execution, canceled or killed then 
         * @p on_exit function is invoked.
         * 
         * A Thread object is reusable. Therefore this function can be called again after the previous
         * thread function has finished execution.
         * 
         * @param function The thread function of execution
         * @param on_exit Termination event callback. Useful for releasing resources if the thread is canceled
         * or killed.
         * @return False is returned if a thread associated to this object is already running, otherwise true is returned.
         */
        bool run(const std::function<void ()>& function, const std::function<void ()>& on_exit = nullptr);
        /**
         * Returns the name of the thread
         */
        const std::string& name() const noexcept;
        /**
         * Checks whether the thread is joinable, i.e. potentially running in parallel context
         * @return True on success, otherwise false is returned.
         */
        bool joinable() const noexcept;
        /**
         * Waits for a thread to finish it's execution 
         */
        void join();
        /**
         * Permits the thread to execute independently from the thread handle
         */
        void detach();
        /**
         * Sends a cancellation request to the thread
         * ( Thread execution will be interrupted at a cancellation point. See testCancel() and man pthread_cancel() )
         * @return False is returned if failed to send cancellation request, otherwise true.
         */
        bool cancel();
        /**
         * Terminates the associated thread by sending a signal (SIGUSR2 is used for that task !)
         * @return False is returned if failed to initiate termination request, otherwise true.
         */
        bool kill();
        /**
         * Sets the soft priority(nice value)
         * @param nice_value An integer representing priority level in the range of -20 to 19 where -20 is the highest priority.
         * @return True is returned if priority setting can be applied, otherwise false.
         */
        bool setPriority(int32_t nice_value);
        /**
         * Sets the list of cpus on which the thread is eligible to run
         * @param cpu_number - list of cpu numbers(ids)
         * @return True is returned if affinity setting can be applied, otherwise false.
         */
        bool setAffinity(const std::vector<int32_t>& cpu_numbers);
    private:
        struct Context
        {
            mutable std::mutex                              mutex;
            std::atomic<pid_t>                              pid;
            std::thread::native_handle_type                 nativeHandle;
            bool                                            killed;
            std::unique_ptr<std::thread>                    thread;
            std::atomic_bool                                state;
            std::function<void ()>                          function;
            std::function<void ()>                          onCancelled;
            int32_t                                         niceValue;
            std::string                                     name;
            std::vector<int32_t>                            cpu_set;
            Context(const std::string& _name);
        };

        struct CleanupContext
        {
            std::shared_ptr<thread_utils::Thread::Context> context;
            CleanupContext(const std::shared_ptr<Context>& ctx);
        };
        static void generalCleanupHandler(void * arg);

        mutable std::mutex       mContextMutex;
        std::shared_ptr<Context> mContext;
        const std::string        mName;//redundant information on purpose

        static void threadFunction(const std::shared_ptr<Context>& context);
        inline void resetContext(const std::shared_ptr<Context>& ctx)
        { std::atomic_store<Context>(&mContext, ctx); }
        inline std::shared_ptr<Thread::Context> getContext() const
        { return std::atomic_load<Context>(&mContext); }

        
    };
}

#endif
