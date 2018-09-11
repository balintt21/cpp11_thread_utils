#ifndef _THREAD_UTILS_THREAD_H_

#include <string>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include <memory>

namespace thread_utils
{
    /**
     * Creates a cancellation point within the calling thread
     * By invoking Thread::cancel() on a thread instance the thread execution will be interrupted at the
     * point of a test_cancel() call.
     */
    void test_cancel();

    class Thread final
    {
    public:
        Thread(const std::string& name);
        ~Thread();
        /**
         * A new thread of execution starts executing the given function.
         * 
         * @param function An std::function<void ()> object which will be invoked on a new thread when started.
         * @param on_cancel An std::function<void ()> object which will be called before termination triggered by a previous invokation of kill() api function.
         * @return False is returned if a thread associated to this object is already running, otherwise true is returned.
         */
        bool run(const std::function<void ()>& function, const std::function<void ()>& on_cancel = nullptr);
        /**
         * Returns a value of std::thread::id identifying the thread associated with *this.
         * @return size_t hash value of the thread id is returned if the thread is running otherwise the hash of the calling thread's id is returned.
         */
        size_t getId() const noexcept;
        /**
         * Returns the name of the thread
         */
        std::string getName() const noexcept;
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
         * The @p on_cancel passed to run() will be invoked on cancel
         * ( Thread execution will be interrupted at a cancellation point. See man pthread_cancel() )
         * @return False is returned if failed to send cancellation request, otherwise true.
         */
        bool cancel();
        /**
         * Terminates the associated thread by sending a signal
         * @return False is returned if failed to initiate termination request, otherwise true.
         */
        bool kill();
        /**
         * Sets the soft priority(nice value) of any new thread of execution. Which means that priority settings only applied if this function is
         * called before invoking run() api function
         * @param nice_value An integer representing priority level in the range of -20 to 19 where -20 is the highest priority.
         * @return True is returned if priority setting can be applied, otherwise false.
         */
        bool setPriority(int32_t nice_value);
    private:
        struct Context
        {
            mutable std::mutex           mutex;
            std::unique_ptr<std::thread> thread;
            std::atomic_bool             state;
            std::function<void ()>       function;
            std::function<void ()>       onCancelled;
            int32_t                      niceValue;
            std::string                  name;
            Context(const std::string& _name);
        };

        static void threadFunction(std::shared_ptr<Context> context);

        std::shared_ptr<Context> mContext;

    };
};

#endif