#include <stdio.h>
#include <atomic>
#include <signal.h>

#include "thread.h"
#include "thread_sleep.h"

static void SigTermHandler( int signum )
{
    printf("PROCESS SigTermHandler caugth(%d)\n", signum);
}

int main(int argc, char** argv)
{

    if( signal( SIGTERM, SigTermHandler ) == SIG_ERR ){ printf( "Connecting signal %d failed!\n", SIGTERM ); }
    else { printf( "Connecting signal %d succeeded!\n", SIGTERM ); }

    thread_utils::Thread th("test_th0");
    thread_utils::Thread th1("test_th1");
    thread_utils::Thread th2("test_th2");
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
    }, [](){ puts("Clean up handler called for th0!"); });

    if(success)
    { printf("Thread %s(%lu) is started.\n", th.name().c_str(), th.id()); }

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
    { printf("Thread %s(%lu) is started.\n", th1.name().c_str(), th1.id()); }

    thread_utils::sleep_for(3000);
    puts("Detaching th1");
    th1.detach();
    puts("th1 detached");

    
    success = th2.run([]()
    {
        uint32_t cnt = 1;
        printf("th2 pthread_self(%lu)\n", pthread_self());
        while(true)
        {
            thread_utils::sleep_for(1000);
            printf("th2: running for %u s\n", cnt++);
            thread_utils::test_cancel();
        }
    }, [](){ puts("Clean up handler called for th2 when killed!"); });
    if(success)
    { printf("Thread %s(%lu) is started.\n", th2.name().c_str(), th2.id()); }
    thread_utils::sleep_for(5000);
    printf("Killing th2 ?%d\n", th2.kill());
    

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