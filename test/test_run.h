#include "thread.h"
#include "semaphore.h"

#include <stdint.h>
#include <atomic>

namespace thread_utils
{
    namespace tests
    {
        /**
         * Tests:
         * 1. Are Thread::join(), cancel(), kill(), detach() functions working?
         * 2. If an 'on_exit' function is passed to Thread::run then will it be invoked no matter the way of
         * thread was terminated?
         * 
         */
        bool test_run()
        {
            std::atomic_uint32_t score(8);
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

                        th.run([&thread_start_event]()
                        {
                            thread_start_event.signal();
                            uint32_t lifetime_sec = 10;
                            while(lifetime_sec)
                            {
                                sleepFor(1000);
                                --lifetime_sec;
                            }
                        });
                        thread_start_event.wait();
                        th.detach();

                        if( th.run( thread_function, on_exit_callback ) )
                        { 
                            --score;
                            thread_start_event.wait();
                            quit_flag.store(true);
                            th.join();
                        }
                    }
                }
            }
            return (score == 0);
        }
    }

}
