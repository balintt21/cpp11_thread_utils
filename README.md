# cpp11_thread_utils
Utility classes for standard threading, mutex library.

## Requirements
* at least C++11
* _pthread_ or (MinGW-w64) _winpthreads_

## Classes
* **Thread** - A wrapper class around std::thread with extended functionality like:
  * _cancel_
  * _kill_
  * _detach_
  * _set priority_
* **Semaphore** - Header only semaphore implementation using std::condition_variable
* **PosixSemaphore** - Header only, uses POSIX semaphore. (lazy impl.: omitting but not hiding retvals and errors) 
* **ConditionMutex** - A mutex and condition_variable in one piece. Implements _'Lockable'_ concept.

## Example
```c++

thread_utils::Thread thread("thread_0");

auto thread_function = [&quit_flag]()
{
    uint32_t counter = 1;
    while(quit_flag.load())
    {
        thread_utils::sleepFor(1000);
        printf("thread_0 is running for %u\n", counter++);
    }
}

thread.run(thread_function);
```
