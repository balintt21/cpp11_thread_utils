#include "thread.h"
#include "semaphore.h"

#include <stdint.h>
#include <atomic>

#include <stdio.h>

namespace thread_utils
{
    namespace tests
    {
        bool test_run()
        {
            std::atomic_uint32_t score(6);
            Thread th("test_th0");

            std::atomic_bool quit_flag(false);
            binary_semaphore_t thread_start_event;

            auto thread_function = [&quit_flag, &score, &thread_start_event]()
            {
                thread_start_event.signal();
                while( !quit_flag.load() )
                {
                    thread_utils::sleepFor(1000);
                    thread_utils::testCancel();
                }
            };

            auto on_exit_callback = [&quit_flag, &score]()
            {
                --score;
                quit_flag.store(false);
            };

            puts("A");
            //normal termination
            if( th.run( thread_function, on_exit_callback ) )
            { 
                --score; 
                thread_start_event.wait();
                quit_flag.store(true);
                th.join();
                //termination triggered by cancel
                if( th.run( thread_function, on_exit_callback ) )
                { 
                    --score; 
                    thread_start_event.wait();
                    th.cancel();
                    th.join();
                    //termination triggered by kill
                    if( th.run( thread_function, on_exit_callback ) )
                    { 
                        --score;
                        thread_start_event.wait();
                        th.kill();
                        th.join();
                    }
                }
            }
            
            return (score == 0);
        }
    }

}
