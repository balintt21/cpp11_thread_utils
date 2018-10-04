# cpp11_thread_utils
Utility classes for standard threading, mutex library.

## Requirements
* at least C++11
* _pthread_ or (MinGW-w64) _winpthreads_
## Namespace
* **thread_utils**
## Classes
* **Semaphore** - Template class. Header only semaphore implementation using std::condition_variable.
* **PosixSemaphore** - Header only, uses POSIX semaphore. (lazy impl.: omitting but not hiding retvals and errors) 
* **ConditionMutex** - A mutex and condition_variable in one piece. Implements _'Lockable'_ concept.
* **Thread** - A wrapper class around std::thread with extended functionality like:
  * _cancel_
  * _kill_
  * _detach_
  * _set priority_ (nice value)
  * _set affinity_ (cpu0, cpu1, cpu2,...)
  * _reuse object (restart)_

## Types
* **binary_semaphore_t** derived from class **Semaphore<2>**
* **semaphore_t** derived from class **Semaphore<std::numeric_limits<uint32_t>::max()>**

## Examples

### Example 1
_Starts a thread that would run forever. Waits for the thread to start with the help of a binary_semaphore_t and lets it run for 5 seconds before cancelling it._
```c++
binary_semaphore_t thread_started_event;
thread_utils::Thread thread("thread_0"); //assigning a name to the thread 'thread_0'
thread.run([&thread_started_event]()
{
    thread_started_event.notify();//Increments the semaphore's value by one (alias for post())
    uint32_t counter = 1;
    while(true)
    {
        thread_utils::testCancel();//Creates a cancelation point (1) If canceled this function does not return
        thread_utils::sleepFor(1000);
        printf("thread_0 is running for %u\n", counter++);
    }
});
thread_started_event.wait();//Waits for the thread to start, blocks if the semaphore's value is zero
thread_utils::sleepFor(5000);//Blocks the execution of the current thread for at least the specified milliseconds
thread.cancel();//Sends a cancellation signal
thread.join();//Waits for the thread to finish it's execution
```
_(1) See [cancellation points](http://pubs.opengroup.org/onlinepubs/000095399/functions/xsh_chap02_09.html#tag_02_09_05_02)_

### Example 2
_Starts a thread without cancellation points that would count forever. Kills the thread after 5 seconds and prints the result._
```c++
binary_semaphore_t thread_started_event;
thread_utils::Thread thread("thread_1");
std::atomic_uint64_t counter(0);
thread.run([&counter, &thread_started_event]()
{
    thread_started_event.notify();//Increments the semaphore's value by one (alias for post())
    while(true) { ++counter; }
});
thread_started_event.wait();
thread_utils::sleepFor(5000);
thread.kill();//Sends a SIGUSR2 signal to the thread that will invoke pthread_exit()
thread.join();
printf("thread_1 counted to %u\n", counter.load());
```

### Example 3
_Starts a thread that would read from a file forever. Kills | Cancels| Requests the thread after 10 seconds. In any case resources will be released._
```c++
std::atomic_bool quit_request(false);
thread_utils::Thread thread("thread_2");
FILE* input_file = fopen("./input.dat", "rb");

auto thread_function = [&input_file, &quit_request]()
{
    char buffer[1024]; //even if killed the stack of the thread will be freed
    while(!quit_request.load())
    {
        while( fread(buffer, 1024, 1, input_file) == 1 )
        {
            buffer[1023] = 0;
            printf("%s\n", buffer);
            if(quit_request.load()) 
            { break; }
        }
        fseek(input_file, 0, SEEK_SET);
    }
};

//This function is invoked no matter to the cause of thread's termination
auto on_exit_callback = [&input_file]()
{
    //Release resources with the help of this callback if the thread is cancelled, killed, or generally
    if(input_file) { fclose(input_file); }
}

thread.run(thread_function, on_exit_callback);
thread_utils::sleepFor(10000);
quit_request.store(true);//thread.kill() or thread.cancel();
thread.join();
```
