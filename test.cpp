#include <stdio.h>
#include <atomic>

#include "thread.h"
#include "thread_sleep.h"

int main(int argc, char** argv)
{
    thread_utils::Thread th("test_th0");
    thread_utils::Thread th1("test_th1");
    th.setPriority(5);

    bool success = th.run([]()
    {
        uint32_t cnt = 1;
        while(true)
        {
            thread_utils::sleep_for(1000);
            printf("th0: running for %u s\n", cnt++);
            thread_utils::test_cancel();
        }
    });

    if(success)
    { printf("Thread %s(%lu) is started.\n", th.getName().c_str(), th.getId()); }

    success = th.run([](){});
    if( !success )
    { puts("Tried to start it again but it is already running."); }

    success = th1.run([]()
    {
        uint32_t cnt = 6;
        while(cnt > 0)
        {
            thread_utils::sleep_for(1000);
            printf("th1: remaining lifetime %u s\n", cnt--);
        }
        puts("th1: ending life");
    });

    if(success)
    { printf("Thread %s(%lu) is started.\n", th1.getName().c_str(), th1.getId()); }

    thread_utils::sleep_for(2000);
    puts("Detaching th1");
    th1.detach();
    puts("th1 detached");

    printf("Waiting for thread, is running? %s\n", th.joinable() ? "yes" : "no" );
    thread_utils::sleep_for(15000);
    th.cancel();
    if( th.joinable() )
    {
        th.join();
    }
    puts("Finished.");

    return 0;
}