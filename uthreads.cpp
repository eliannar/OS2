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
 * Struct for
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



// checks if a given tid is in valid range and exists
bool is_invalid_tid(int tid)
{
	return (tid < 0 || tid >= MAX_THREAD_NUM || all_threads[tid] == nullptr);
}


// function finds first true id in available_ids, sets it to true and returns it
int get_next_id(){
	for(int i = 0; i < MAX_THREAD_NUM; i++){
		if(available_ids[i]){
			available_ids[i] = false;
			return i;
		}
	}
	return FAILURE;
}

// frees all used resources
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

// exit and free all
int exit_and_free(int exit_code){
	free_all();
	exit(exit_code);
}

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

void timer_handler(int sig){
    //cout<< "timer" << endl;
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


int uthread_get_tid(){
	return cur_tid;
}

int uthread_get_total_quantums(){
	return global_quanta;
}

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
