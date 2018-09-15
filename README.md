# cpp11_thread_utils
Utility classes for standard threading, mutex library.

## Requirements
* at least C++11
* _pthread_ or (MinGW-w64) _winpthreads_

## Classes
* **Semaphore** - Template class. Header only semaphore implementation using std::condition_variable.
* **PosixSemaphore** - Header only, uses POSIX semaphore. (lazy impl.: omitting but not hiding retvals and errors) 
* **ConditionMutex** - A mutex and condition_variable in one piece. Implements _'Lockable'_ concept.
* **Thread** - A wrapper class around std::thread with extended functionality like:
  * _cancel_
  * _kill_
  * _detach_
  * _set priority_ (nice value)
  * _reuse object (restart)_

## Types
* **binary_semaphore_t** derived from class **Semaphore<2>**
* **semaphore_t** derived from class **Semaphore<std::numeric_limits<uint32_t>::max()>**

## Examples

### Example 1
_Starts a thread that would run forever. Waits for the thread to start with the help of a binary_semaphore_t<br/>
and lets it run for 5 seconds before cancelling it._
```c++
thread_utils::binary_semaphore_t thread_started_event;
thread_utils::Thread thread("thread_0");
thread.run([&]()
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
