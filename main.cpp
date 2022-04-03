//
// Created by eliannar on 31/03/2022.
//
#include <iostream>
#include <unistd.h>
#include <functional>
#include "uthreads.h"
#define SECOND 1000000

void test_1_create_and_terminate();

using namespace std;

double sleep_time = 1;

void sleeper(double time)
{
    const clock_t begin_time = clock();
    // do something
    while(float( clock () - begin_time )/  CLOCKS_PER_SEC < time)
    {
    }
    return;
}

void single_thread_print_to_quantum()
{
    int id = uthread_get_tid();
    cout << "running thread " << id << endl;
    int i = 0;
    for (; i < 5; i++)
    {
        std::cout << "in thread " << id << " single function "<< i << endl;
        sleeper(sleep_time);
    }
    uthread_terminate(id);
}

void single_thread_quantum_check()
{
    int id = uthread_get_tid();
    cout << "ruuning thread " << id << endl;
    int i = 0;
    for (; i < 5; i++)
    {
        std::cout << "in thread " << id << " single function "<< i << endl;
        sleeper(sleep_time);
    }
    cout << "Thread " << id << " quanta " << uthread_get_quantums(id) << endl;
    cout << "Thread " << 0 << " quanta " << uthread_get_quantums(0) << endl;
    cout << "Total quanta " << uthread_get_total_quantums() << endl;
    uthread_terminate(id);
}

void self_blocking_method()
{
    int id = uthread_get_tid();
    cout << "running thread " << id << endl;
    int i = 0;
    for (; i < 5; i++)
    {
        std::cout << "in thread " << id << " single function "<< i << endl;
        sleeper(sleep_time);
        if (i == 2)
        {
            cout << "Blocking myself!" << endl;
            uthread_block(id);
        }
    }
    uthread_terminate(id);
}

void kill_main_on_termination()
{
    int id = uthread_get_tid();
    int i = 0;
    for (; i < 7; i++)
    {
        std::cout <<  "in thread " << id << " in single function "<< i << endl;
        sleeper(1);
    }
    uthread_terminate(0);
}

void test_1_create_and_terminate() {
    uthread_init(SECOND);
    cout << "running test 1" << endl;
    uthread_spawn(single_thread_print_to_quantum);
    while (1)
    {

    }
}

void test_2_create_two_threads()
{
    uthread_init(SECOND);
    cout << "running test 2" << endl;
    uthread_spawn(single_thread_print_to_quantum);

    uthread_spawn(single_thread_print_to_quantum);
    while (1)
    {

    }
}

void test_3_get_terminated_id()
{
    uthread_init(SECOND);
    cout << "running test 3" << endl;
    uthread_spawn(single_thread_print_to_quantum);
    uthread_spawn(single_thread_print_to_quantum);
    uthread_spawn(single_thread_print_to_quantum);
    sleeper(2);
    cout << "terminating thread 2\n";
    uthread_terminate(2);
    sleeper(2);
    cout << "creating a new thread\n";
    uthread_spawn(single_thread_print_to_quantum);
    while (1)
    {

    }
}

void test_4_creating_too_many_threads()
{
    uthread_init(SECOND);
    cout << "running test 4" << endl;
    for (int i=0; i<MAX_THREAD_NUM - 1; i++)
    {
        uthread_spawn(single_thread_print_to_quantum);
    }
    cout << "created 100 threads" << endl;
    int err_code = uthread_spawn(single_thread_print_to_quantum);
    if (err_code == -1)
    {
        cout << "error exceeded number of threads"<< endl;
    }
}

void test_5_terminating_main_thread()
{
    uthread_init(SECOND);
    cout << "running test 5" << endl;
    uthread_spawn(single_thread_print_to_quantum);
    uthread_spawn(kill_main_on_termination);
    while (true)
    {}
}

void test_6_block_thread()
{
    uthread_init(SECOND);
    cout << "running test 6" << endl;
    uthread_spawn(single_thread_print_to_quantum);
    uthread_spawn(single_thread_print_to_quantum);
    sleeper(2);
    cout << "Blocking thread 1" << endl;
    uthread_block(1);

    while (true)
    {}
}

void test_7_self_block_thread()
{
    sleep_time = 1;
    uthread_init(SECOND);
    cout << "running test 7" << endl;
    uthread_spawn(self_blocking_method);
    uthread_spawn(single_thread_print_to_quantum);
    while (true)
    {}
}

void test_8_block_and_resume()
{
    uthread_init(SECOND);
    cout << "running test 8" << endl;
    uthread_spawn(self_blocking_method);
    uthread_spawn(single_thread_print_to_quantum);
    sleeper(10);
    cout << "resuming thread 1" << endl;
    uthread_resume(1);
    while (true)
    {}
}

void test_9_basic_single_thread_quanta()
{
    uthread_init(SECOND);
    cout << "running test 9" << endl;
    uthread_spawn(single_thread_quantum_check);
    sleeper(15);
    while (true)
    {}
}

/*
 * more test - 09.05.2021 - start
 */

void single_thread_print_to_8()
{
    int id = uthread_get_tid();
    cout << "running thread " << id << endl;
    int i = 0;
    for (; i < 8; i++)
    {
        std::cout << "in thread " << id << " single function "<< i << endl;
        sleeper(sleep_time);
    }
    uthread_terminate(id);
}

void single_thread_print_to_30()
{
    int id = uthread_get_tid();
    cout << "running thread " << id << endl;
    int i = 0;
    for (; i < 30; i++)
    {
        std::cout << "in thread " << id << " single function "<< i << endl;
        sleeper(sleep_time);
    }
    uthread_terminate(id);
}

void single_thread_print_to_8_and_kill_process_2()
{
    int id = uthread_get_tid();
    cout << "running thread " << id << endl;
    int i = 0;
    for (; i < 4; i++)
    {
        std::cout << "in thread " << id << " single function "<< i << endl;
        sleeper(sleep_time);
    }
    cout <<"killing process 2" << endl;
    uthread_terminate(2);
    for (; i < 8; i++)
    {
        std::cout << "in thread " << id << " single function "<< i << endl;
        sleeper(sleep_time);
    }
    uthread_terminate(id);
}

void single_thread_print_to_4_then_suicide()
{
    int id = uthread_get_tid();
    cout << "running thread " << id << endl;
    int i = 0;
    for (; i < 4; i++)
    {
        std::cout << "in thread " << id << " single function "<< i << endl;
        sleeper(sleep_time);
    }
    cout <<"killing myself" << endl;
    uthread_terminate(id);
    for (; i < 8; i++)
    {
        std::cout << "in thread " << id << " single function "<< i << endl;
        sleeper(sleep_time);
    }
    uthread_terminate(id);
}

void thread_kill_main()
{
    int id = uthread_get_tid();
    cout << "running thread " << id << endl;
    int i = 0;
    for (; i < 4; i++)
    {
        std::cout << "in thread " << id << " single function "<< i << endl;
        sleeper(sleep_time);
    }
    cout <<"killing main" << endl;
    uthread_terminate(0);
}

void block_thread_1()
{
    uthread_block(1);
    uthread_terminate(uthread_get_tid());
}

void block_main_thread()
{
    uthread_block(0);
    uthread_terminate(uthread_get_tid());
}

void thread_blocking_itself_count_to_4()
{
    int id = uthread_get_tid();
    cout << "running thread " << id << endl;
    int i = 0;
    for (; i < 4; i++)
    {
        std::cout << "in thread " << id << " single function "<< i << endl;
        sleeper(sleep_time);
    }
    cout <<"blocking myself" << endl;
    uthread_block(id);
    for (; i < 8; i++)
    {
        std::cout << "in thread " << id << " single function "<< i << endl;
        sleeper(sleep_time);
    }
    uthread_terminate(id);
}

void count_to_4_block_1_thread_then_resume()
{
    int id = uthread_get_tid();
    cout << "running thread " << id << endl;
    int i = 0;
    for (; i < 4; i++)
    {
        std::cout << "in thread " << id << " single function "<< i << endl;
        sleeper(sleep_time);
    }
    cout <<"blocking 1 thread" << endl;
    uthread_block(1);
    for (; i < 8; i++)
    {
        std::cout << "in thread " << id << " single function "<< i << endl;
        sleeper(sleep_time);
    }
    cout << "resume thread 1: " << uthread_resume(1) << endl;
    uthread_terminate(id);
}



void block_thread_2_and_print_to_8_then_resume_thread_2()
{
    uthread_block(2);
    int id = uthread_get_tid();
    cout << "running thread " << id << endl;
    int i = 0;
    for (; i < 8; i++)
    {
        std::cout << "in thread " << id << " single function "<< i << endl;
        sleeper(sleep_time);
    }
    uthread_resume(2);
    uthread_terminate(id);
}



void test_2_1_1_init_call_with_negative_error()
{
    cout << uthread_init(-4) << endl;
}

void test_2_1_2_init_call_with_positive()
{
    cout << "running " << __FUNCTION__ << endl;
    cout << uthread_init(SECOND) << endl;
    uthread_terminate(0);
}

void test_2_2_1_spawn_regular_case()
{
    cout << "running " << __FUNCTION__ << endl;
    uthread_init(SECOND);
    cout << uthread_spawn(single_thread_print_to_8) << endl;
    cout << uthread_spawn(single_thread_print_to_8) << endl;
    cout << uthread_spawn(single_thread_print_to_8) << endl;
    while (true){}
}

void test_2_2_1_spawn_more_threads_than_max()
{
    cout << "running " << __FUNCTION__ << endl;
    uthread_init(SECOND);
    for (int i=0; i<MAX_THREAD_NUM - 1; i++)
    {
        uthread_spawn(single_thread_print_to_quantum);
    }
    cout << "created 100 threads" << endl;
    int err_code = uthread_spawn(single_thread_print_to_quantum);
    if (err_code == -1)
    {
        cout << "error exceeded number of threads"<< endl;
    }
    while (true){}
}

void test_2_3_1_terminate_one_thread_terminates_other()
{
    cout << "running " << __FUNCTION__ << endl;
    uthread_init(SECOND);
    uthread_spawn(single_thread_print_to_8_and_kill_process_2);
    uthread_spawn(single_thread_print_to_8);
    while(true){}
}

void test_2_3_2_self_termination()
{
    cout << "running " << __FUNCTION__ << endl;
    uthread_init(SECOND);
    uthread_spawn(single_thread_print_to_4_then_suicide);
    uthread_spawn(single_thread_print_to_8);
    while(true){}
}

void test_2_3_3_main_self_termination()
{
    cout << "running " << __FUNCTION__ << endl;
    uthread_init(SECOND);
    uthread_spawn(single_thread_print_to_4_then_suicide);
    uthread_spawn(single_thread_print_to_8);
    cout << "thread " << uthread_get_tid() << " suicide" << endl;
    uthread_terminate(uthread_get_tid());
    while(true){}
}

void test_2_3_4_thread_1_killing_main()
{
    cout << "running " << __FUNCTION__ << endl;
    uthread_init(SECOND);
    uthread_spawn(thread_kill_main);
    while(true){}
}

void test_2_3_5_get_smallest_terminated_id()
{
    cout << "running " << __FUNCTION__ << endl;
    uthread_init(SECOND);
    uthread_spawn(single_thread_print_to_8);
    uthread_spawn(single_thread_print_to_8);
    uthread_spawn(single_thread_print_to_8);
    uthread_spawn(single_thread_print_to_8);
    uthread_spawn(single_thread_print_to_8);
    uthread_terminate(4);
    uthread_terminate(2);
    uthread_terminate(3);
    cout << "terminated 2 3 4" << endl;
    uthread_spawn(single_thread_print_to_8);
    uthread_spawn(single_thread_print_to_8);
    uthread_spawn(single_thread_print_to_8);
    while(true){}
}

void test_2_3_6_terminate_valid_and_invalid_threads_return_values_check()
{
    cout << "running " << __FUNCTION__ << endl;
    uthread_init(SECOND);
    uthread_spawn(single_thread_print_to_8);
    uthread_spawn(single_thread_print_to_8);
    cout << "terminate invalid thread id, returns: " << uthread_terminate(4) << endl;
    cout << "terminate valid thread id, returns: " << uthread_terminate(2) << endl;
    while(true){}
}

void test_2_4_1_block_simple_block_by_main_and_other_thread()
{
    cout << "running " << __FUNCTION__ << endl;
    uthread_init(SECOND);
    uthread_spawn(single_thread_print_to_8);
    uthread_spawn(single_thread_print_to_8);
    uthread_spawn(single_thread_print_to_8);
    uthread_spawn(block_thread_1);
    cout << uthread_block(2);
    while (true){}
}

void test_2_4_2_block_not_exists_thread()
{
    cout << "running " << __FUNCTION__ << endl;
    uthread_init(SECOND);
    uthread_spawn(single_thread_print_to_8);
    cout << "blockin a thread that doesn't exist returns: " <<  uthread_block(2) << endl;
    while (true){}
}

void test_2_4_2_blocking_main_thread_error()
{
    cout << "running " << __FUNCTION__ << endl;
    uthread_init(SECOND);
    cout << "thread 1 is trying to block main thread" << endl;
    uthread_spawn(block_main_thread);
    cout << "main is trying to block itself, returns: " << uthread_block(0) << endl;
    while (true){}
}

void test_2_4_3_thread_blocking_itself()
{
    cout << "running " << __FUNCTION__ << endl;
    uthread_init(SECOND);
    uthread_spawn(thread_blocking_itself_count_to_4);
    while (true){}
}

void test_2_4_4_blocking_a_blocked_thread()
{
    cout << "running " << __FUNCTION__ << endl;
    uthread_init(SECOND);
    uthread_spawn(single_thread_print_to_8);
    uthread_spawn(single_thread_print_to_8);
    cout << "blocking first time: " << uthread_block(1) << endl;
    cout << "blocking second time: " << uthread_block(1) << endl;
    while (true){}
}

void test_2_5_1_resuming_a_blocked_thread()
{
    cout << "running " << __FUNCTION__ << endl;
    uthread_init(SECOND);
    uthread_spawn(single_thread_print_to_8);
    uthread_spawn(count_to_4_block_1_thread_then_resume);
    while (true){}
}

void test_2_5_2_resuming_ready_running_and_main()
{
    cout << "running " << __FUNCTION__ << endl;
    uthread_init(SECOND);
    uthread_spawn(single_thread_print_to_8);
    uthread_spawn(single_thread_print_to_8);
    cout << "main resumes ready_thread: " << uthread_resume(1) << endl;
    cout << "main resumes itself: " << uthread_resume(0) << endl;
    while (true){}
}





/*
 * more test - 09.05.2021 - end
 */



void block_1_thread_then_resume()
{
    int id = uthread_get_tid();
    cout << "running thread " << id << endl;
    cout <<"blocking 1 thread" << endl;
    uthread_block(1);
    int i = 0;
    for (; i < 8; i++)
    {
        std::cout << "in thread " << id << " single function "<< i << endl;
        sleeper(sleep_time);
    }
    cout << "resume thread 1: " << uthread_resume(1) << endl;
    uthread_terminate(id);
}




int main()
{
    test_1_create_and_terminate();
    //    test_2_create_two_threads();
    //    test_3_get_terminated_id();
    //    test_4_creating_too_many_threads();
    //    test_5_terminating_main_thread();
    //    test_6_block_thread();
    //    test_7_self_block_thread();
    //    test_8_block_and_resume();
    //    test_9_basic_single_thread_quanta();
    //    test_2_1_1_init_call_with_negative_error();
    //    test_2_1_2_init_call_with_positive();
    //    test_2_2_1_spawn_regular_case();
    //    test_2_2_1_spawn_more_threads_than_max();
    //    test_2_3_1_terminate_one_thread_terminates_other();
    //    test_2_3_2_self_termination();
    //    test_2_3_3_main_self_termination();
    //    test_2_3_4_thread_1_killing_main();
    //    test_2_3_5_get_smallest_terminated_id();
    //    test_2_3_6_terminate_valid_and_invalid_threads_return_values_check();
    //    test_2_4_1_block_simple_block_by_main_and_other_thread();
    //    test_2_4_2_block_not_exists_thread();
    //    test_2_4_2_blocking_main_thread_error();
    //    test_2_4_3_thread_blocking_itself();
    //    test_2_4_4_blocking_a_blocked_thread();
    //    test_2_5_1_resuming_a_blocked_thread();
    //    test_2_5_2_resuming_ready_running_and_main();
    //    test_2_6_1_mutex_lock_return();
    //    test_2_6_2_mutex_threads_waiting();
    //    test_2_6_3_main_thread_is_mutex_blocked();
    //    test_2_6_4_double_lock_error();
    //    test_2_6_5_unlock_unlocked_mutex_error();
    //    test_2_6_6_wrong_thread_trying_to_release_mutex();
    //    test_2_6_7_mutex_blocked_then_blocked_unlock_has_no_effect();
    //    test_2_6_8_mutex_blocked_then_blocked_by_threads_that_locks_mutex();
    //    test_2_6_9_resuming_mutex_blocked_thread_that_will_get_the_mutex();
    //    test_2_6_10_resuming_mutex_blocked_thread_that_wont_get_the_mutex();
    //    test_mutex_locked_by_blocked_thread();
    //    test_terminating_thread_that_locks_the_mutex_while_mutex_queue_not_empty();
    //test_terminating_thread_that_locks_the_mutex_while_mutex_queue_empty();
    return 0;
}
