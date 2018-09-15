# cpp11_thread_utils
Utility classes for std::thread library.

# Requirements
* _pthread_ 
* OR (MinGW-w64) _winpthreads_

# Example
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
