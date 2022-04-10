#include "uthreads.h"


#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <queue>
#include <iostream>
#include <list>
#include <unordered_map>
#include <algorithm>

using namespace std;

#define FAILURE -1
#define SUCCESS 0

//==== lib errors
#define LIB_ERROR_MESSAGE "thread library error: "
#define INVALID_TID_ERR "Invalid tid"
#define MAX_THREAD_NUM_ERR "The number of threads has exceeded the maximum"
#define INVALID_QUANTUM_USECS_ERR "Invalid quantum_usecs"
#define READY_EMPTY_ERR "Ready queue had no threads to run"

//==== sys errors
#define SYS_ERROR_MESSAGE "system error: "
#define FAILED_ALLOC_ERR "Error in alloc"
#define SIGEMPTYSET_ERR "Error in sigemptyset"
#define TIMER_ERR "Error in timer initialization"
#define SIGACTION_ERR "Error in sigaction initialization"
#define MASK_ERR "Error in signal masking"
#define UNMASK_ERR "Error in signal unmasking"

#define ERROR_EXIT_CODE 1
#define SUCCESS_EXIT_CODE 0


enum State{Ready, Blocked, Running};

/**
 * Struct for Thread. Holds id, local quanta, environement, state and stack.
 */
typedef struct Thread{
    int id;
    int quanta;
    sigjmp_buf env;
    State state;
    char stack[STACK_SIZE];
} Thread;


/// setup global variables

// true if id is available, false otherwise
bool available_ids[MAX_THREAD_NUM];

// old version: Thread* all_threads = new Thread[MAX_THREAD_NUM];
// in this method, everything is initialized to nullptr
vector<Thread *> all_threads = vector<Thread *>(MAX_THREAD_NUM);
list<Thread *> ready = list<Thread *>();
unordered_map<int, list<int>> sleeping = unordered_map<int, list<int>>();
int quantum_usecs;
int quantum_secs;
int cur_tid;
int global_quanta;


/// helper functions


typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
	address_t ret;
	asm volatile("xor    %%fs:0x30,%0\n"
				 "rol    $0x11,%0\n"
	: "=g" (ret)
	: "0" (addr));
	return ret;
}



/**
 * @brief Checks validity of tid
 *
 * @return true if tid is invalid, false otherwise
*/
bool is_invalid_tid(int tid)
{
	return (tid < 0 || tid >= MAX_THREAD_NUM || all_threads[tid] == nullptr);
}


/**
 * @brief Finds smallest available tid
 *
 * @return tid on success, -1 on failure
*/
int get_next_id(){
	for(int i = 0; i < MAX_THREAD_NUM; i++){
		if(available_ids[i]){
			available_ids[i] = false;
			return i;
		}
	}
	return FAILURE;
}

/**
 * @brief Frees all memory saved in all_threads array
 *
 * @return 0 on success, -1 on failure
*/
int free_all(){
	for (int i = 0; i < MAX_THREAD_NUM; i++)
	{
		if (all_threads[i] != nullptr)
		{
			delete all_threads[i];
			all_threads[i] = nullptr;
		}
	}

	return SUCCESS;
}

/**
 * @brief runs free_all functions and exits.
 *
 * @return 0 on success, -1 on failure
*/
int exit_and_free(int exit_code){
	free_all();
	exit(exit_code);
}

/**
 * @brief Masks timer signal
 *
 * @return 0 on success, -1 on failure
*/
int mask_timer(){
	sigset_t set;
	if(sigemptyset(&set) == FAILURE){
		std::cerr << SYS_ERROR_MESSAGE << MASK_ERR << endl;
		return FAILURE;
	}
	if(sigaddset(&set, SIGVTALRM) == FAILURE){
		std::cerr << SYS_ERROR_MESSAGE << MASK_ERR << endl;
		return FAILURE;
	}
	//todo maybe need to add to old set
	if(sigprocmask(SIG_BLOCK, &set, nullptr) == FAILURE){
		std::cerr << SYS_ERROR_MESSAGE << MASK_ERR << endl;
		return FAILURE;
	}
	return SUCCESS;
}


/**
 * @brief Unmasks timer signal
 *
 * @return 0 on success, -1 on failure
*/
int unmask_timer(){
	sigset_t set;
	if(sigemptyset(&set) == FAILURE){
		std::cerr << SYS_ERROR_MESSAGE << UNMASK_ERR << endl;
		return FAILURE;
	}
	if(sigaddset(&set, SIGVTALRM) == FAILURE){
		std::cerr << SYS_ERROR_MESSAGE << UNMASK_ERR << endl;
		return FAILURE;
	}
	if(sigprocmask(SIG_UNBLOCK, &set, nullptr) == FAILURE){
		std::cerr << SYS_ERROR_MESSAGE << UNMASK_ERR << endl;
		return FAILURE;
	}
	return SUCCESS;
}


/**
 * @brief Runs next thread in Ready and updates states.
 *
 * @return void
*/
void run_next_thread() {
    //cout<< "runner" << endl;

    if (ready.empty()) {
        std::cerr << LIB_ERROR_MESSAGE << READY_EMPTY_ERR << endl;
        exit_and_free(ERROR_EXIT_CODE);
    }
    Thread* cur_thread_ptr = ready.front();
    ready.pop_front();
    cur_tid = cur_thread_ptr->id;
//    cout << cur_tid << endl;
    cur_thread_ptr->state = State::Running;
    // update quanta
    cur_thread_ptr->quanta++;
    global_quanta++;
    siglongjmp(cur_thread_ptr->env, 1);
}

/**
 * @brief saves environemnet of currently running thread, and moves it to Ready. Then runs next
 * thread in Ready.
 *
 * @return void
*/
void timer_handler(int sig){
	//needs to switch current thread back to READY and put next thread into run
	// null check is neccessary if thread terminated
	if(all_threads[cur_tid] != nullptr){
		Thread* cur_thread_ptr = all_threads[cur_tid];
		// save current thread context
		int ret = sigsetjmp(cur_thread_ptr->env, 1);
		if (ret != SUCCESS) {
		    return;
		}
		if(cur_thread_ptr->state == State::Running)
		{
			cur_thread_ptr->state = State::Ready;
			ready.push_back(cur_thread_ptr);
		}
	}
	// check if sleeping threads should be woken
	auto wakeup_pair = sleeping.find(global_quanta + 1);
	if (wakeup_pair != sleeping.end()) {
		for (auto i = wakeup_pair->second.begin(); i != wakeup_pair->second.end(); ++wakeup_pair) {
			int tid = *(i);
			Thread* wakeup_thread_ptr = all_threads[tid];
			wakeup_thread_ptr->state = State::Ready;
			ready.push_back(wakeup_thread_ptr);
		}
	}
	//run next thread that is in READY
	run_next_thread();
}

/**
 * @brief resets the timer and runs the next thread in Ready.
 *
 * @return void
*/
void reset_timer_and_run_next()
{
	mask_timer();
	// reset timer
	struct itimerval timer{};
	timer.it_value.tv_sec = quantum_secs;
	timer.it_value.tv_usec = quantum_usecs;
	timer.it_interval.tv_sec = quantum_secs;
	timer.it_interval.tv_usec = quantum_usecs;
	if (setitimer(ITIMER_VIRTUAL, &timer, nullptr))
	{
		std::cerr << SYS_ERROR_MESSAGE << TIMER_ERR << endl;
		exit(FAILURE);
	}
	run_next_thread();
}


/// real functions
/**
 * @brief initializes the thread library.
 *
 * You may assume that this function is called before any other thread library function, and that it is called
 * exactly once.
 * The input to the function is the length of a quantum in micro-seconds.
 * It is an error to call this function with non-positive quantum_usecs.
 *
 * @return On success, return 0. On failure, return -1.
*/
int uthread_init(int quantum_usecs_p){
	// non-positive quantum is invalid
	if (quantum_usecs_p <= 0) {
		cerr << LIB_ERROR_MESSAGE << INVALID_QUANTUM_USECS_ERR << endl;
		return FAILURE;
	}
	// initialize globals: std::fill_n is used to fills available_ids with true
	fill_n(available_ids, MAX_THREAD_NUM, true);

	quantum_usecs = quantum_usecs_p % 1000000;
	quantum_secs = quantum_usecs_p / 1000000;

	// create newThread - thread with id == 0
	Thread* HeadThread;
	try
	{
		HeadThread = new Thread();
	}
	catch (bad_alloc &)
	{
		cerr << SYS_ERROR_MESSAGE << FAILED_ALLOC_ERR << endl;
		exit_and_free(ERROR_EXIT_CODE);
	}
	HeadThread->id = get_next_id();
	sigsetjmp(HeadThread->env, 1);
	HeadThread->state = State::Running;
	all_threads[0] = HeadThread;

	// Install timer_handler as the signal handler for SIGVTALRM.
	struct sigaction sa = {0};
	struct itimerval timer{};
	sa.sa_handler = &timer_handler;
	if (sigaction(SIGVTALRM, &sa, nullptr) < 0)
	{
		std::cerr << SYS_ERROR_MESSAGE << SIGACTION_ERR << endl;
		exit(FAILURE);
	}
	//timer should initially expire after given guantum value, and continue to expire with same
	// value
	timer.it_value.tv_sec = quantum_secs;
	timer.it_value.tv_usec = quantum_usecs;
	timer.it_interval.tv_sec = quantum_secs;
	timer.it_interval.tv_usec = quantum_usecs;
	if (setitimer(ITIMER_VIRTUAL, &timer, nullptr))
	{
		std::cerr << SYS_ERROR_MESSAGE << TIMER_ERR << endl;
		exit(FAILURE);
	}
	global_quanta++;
	HeadThread->quanta = 1;
	return SUCCESS;

}


/**
 * @brief Creates a new thread, whose entry point is the function entry_point with the signature
 * void entry_point(void).
 *
 * The thread is added to the end of the READY threads list.
 * The uthread_spawn function should fail if it would cause the number of concurrent threads to exceed the
 * limit (MAX_THREAD_NUM).
 * Each thread should be allocated with a stack of size STACK_SIZE bytes.
 *
 * @return On success, return the ID of the created thread. On failure, return -1.
*/
int uthread_spawn(thread_entry_point entry_point){
	mask_timer();
    int id = get_next_id();
    if(id == FAILURE){
		cerr << LIB_ERROR_MESSAGE << MAX_THREAD_NUM_ERR << endl;
		unmask_timer();
        return FAILURE;
    }
	// create new thread and set it's id
	Thread* newThread;
	try
	{
		newThread = new Thread();
	}
	catch (bad_alloc &)
	{
		cerr << SYS_ERROR_MESSAGE << FAILED_ALLOC_ERR << endl;
		unmask_timer();
		exit_and_free(ERROR_EXIT_CODE);
	}
	newThread->id = id;
	newThread->quanta = 0;
	newThread->state = State::Ready;
	// initializes env to use the right stack, and to run from the function 'entry_point', when we'll use
	// siglongjmp to jump into the thread.
	address_t sp, pc;
	sp = (address_t) (newThread->stack) + STACK_SIZE - sizeof(address_t);
	pc = (address_t) entry_point;
	sigsetjmp(newThread->env, 1);
	(newThread->env->__jmpbuf)[JB_SP] = translate_address(sp);
	(newThread->env->__jmpbuf)[JB_PC] = translate_address(pc);
	if (sigemptyset(&newThread->env->__saved_mask) == FAILURE)
	{
		cerr << SYS_ERROR_MESSAGE << SIGEMPTYSET_ERR << endl;
		unmask_timer();
		exit_and_free(ERROR_EXIT_CODE);
	}
	ready.push_back(newThread);
	all_threads[id] = newThread;
    unmask_timer();
	return id;
}

/**
 * @brief Terminates the thread with ID tid and deletes it from all relevant control structures.
 *
 * All the resources allocated by the library for this thread should be released. If no thread with ID tid exists it
 * is considered an error. Terminating the main thread (tid == 0) will result in the termination of the entire
 * process using exit(0) (after releasing the assigned library memory).
 *
 * @return The function returns 0 if the thread was successfully terminated and -1 otherwise. If a thread terminates
 * itself or the main thread is terminated, the function does not return.
*/
int uthread_terminate(int tid){
	mask_timer();
    if (is_invalid_tid(tid)){
		cerr << LIB_ERROR_MESSAGE << INVALID_TID_ERR << endl;
		unmask_timer();
        return FAILURE;
    }
    if(tid == 0){
    	unmask_timer();
        return exit_and_free(SUCCESS_EXIT_CODE);
    }
    if(find(ready.begin(), ready.end(), all_threads[tid]) != ready.end()){
		ready.remove(all_threads[tid]);
    }
    bool is_running = (all_threads[tid]->state == State::Running);
	delete all_threads[tid];
    all_threads[tid] = nullptr;
    available_ids[tid] = true;
	unmask_timer();
	if (is_running)
	{
		reset_timer_and_run_next();
	}
    return SUCCESS;
}


/**
 * @brief Blocks the thread with ID tid. The thread may be resumed later using uthread_resume.
 *
 * If no thread with ID tid exists it is considered as an error. In addition, it is an error to try blocking the
 * main thread (tid == 0). If a thread blocks itself, a scheduling decision should be made. Blocking a thread in
 * BLOCKED state has no effect and is not considered an error.
 *
 * @return On success, return 0. On failure, return -1.
*/
int uthread_block(int tid){
	mask_timer();
	if(is_invalid_tid(tid) || tid == 0){
		cerr << LIB_ERROR_MESSAGE << INVALID_TID_ERR << endl;
		unmask_timer();
		return FAILURE;
	}
	if(all_threads[tid]->state != State::Blocked){
	    all_threads[tid]->state = State::Blocked;
		if(find(ready.begin(), ready.end(), all_threads[tid]) != ready.end()){ // if state==ready
			ready.remove(all_threads[tid]);
		}
		else{ // if state == running meaning we're blocking ourselves
		    int ret = sigsetjmp(all_threads[tid]->env, 1);
            if (ret != SUCCESS) {
                unmask_timer();
                return SUCCESS;
            }
		    unmask_timer();
			reset_timer_and_run_next();
		}
	}
	unmask_timer();
	return SUCCESS;
}



/**
 * @brief Resumes a blocked thread with ID tid and moves it to the READY state.
 *
 * Resuming a thread in a RUNNING or READY state has no effect and is not considered as an error. If no thread with
 * ID tid exists it is considered an error.
 *
 * @return On success, return 0. On failure, return -1.
*/
int uthread_resume(int tid){
	mask_timer();
	if(is_invalid_tid(tid)){
		cerr << LIB_ERROR_MESSAGE << INVALID_TID_ERR << endl;
		unmask_timer();
		return FAILURE;
	}
	Thread *resumeThread = all_threads[tid];
	// check if tid is a valid thread's id.
	if (resumeThread == nullptr) //todo why is this here after we checked validity?
	{
		std::cerr << LIB_ERROR_MESSAGE << INVALID_TID_ERR << endl;
		unmask_timer();
		return FAILURE;
	}
	if(all_threads[tid]->state == State::Blocked){
		all_threads[tid]->state = State::Ready;
		ready.push_back(all_threads[tid]);
	}
	unmask_timer();
	return SUCCESS;
}


/**
 * @brief Blocks the RUNNING thread for num_quantums quantums.
 *
 * Immediately after the RUNNING thread transitions to the BLOCKED state a scheduling decision should be made.
 * After the sleeping time is over, the thread should go back to the end of the READY threads list.
 * The number of quantums refers to the number of times a new quantum starts, regardless of the reason. Specifically,
 * the quantum of the thread which has made the call to uthread_sleep isn’t counted.
 * It is considered an error if the main thread (tid==0) calls this function.
 *
 * @return On success, return 0. On failure, return -1.
*/
int uthread_sleep(int num_quantums){
	mask_timer();
	int wake_up_quanta = global_quanta + num_quantums;
	all_threads[cur_tid]->state = State::Blocked;
	// check if wake_up_quanta in hashmap
	auto it = sleeping.find(wake_up_quanta);
	if (it == sleeping.end()) {
		// if not found, insert with tid
		pair<int, list<int>> thread_wake(wake_up_quanta,list<int>(cur_tid));
		sleeping.insert(thread_wake);
	}
	else {
		// if found, add tid to list
		it->second.push_back(cur_tid);
	}
	unmask_timer();
	reset_timer_and_run_next();
	return SUCCESS;
}

/**
 * @brief Returns the thread ID of the calling thread.
 *
 * @return The ID of the calling thread.
*/
int uthread_get_tid(){
	return cur_tid;
}

/**
 * @brief Returns the total number of quantums since the library was initialized, including the current quantum.
 *
 * Right after the call to uthread_init, the value should be 1.
 * Each time a new quantum starts, regardless of the reason, this number should be increased by 1.
 *
 * @return The total number of quantums.
*/
int uthread_get_total_quantums(){
	return global_quanta;
}

/**
 * @brief Returns the number of quantums the thread with ID tid was in RUNNING state.
 *
 * On the first time a thread runs, the function should return 1. Every additional quantum that the thread starts should
 * increase this value by 1 (so if the thread with ID tid is in RUNNING state when this function is called, include
 * also the current quantum). If no thread with ID tid exists it is considered an error.
 *
 * @return On success, return the number of quantums of the thread with ID tid. On failure, return -1.
*/
int uthread_get_quantums(int tid){
	if(is_invalid_tid(tid)){
		cerr << LIB_ERROR_MESSAGE << INVALID_TID_ERR << endl;
		return FAILURE;
	}
	Thread *thread = all_threads[tid];
	if (thread == nullptr)
	{
		std::cerr << LIB_ERROR_MESSAGE << INVALID_TID_ERR << endl;
		return FAILURE;
	}
	return all_threads[tid]->quanta;
}
