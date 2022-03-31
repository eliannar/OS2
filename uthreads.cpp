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
#include <unordered_set>

using namespace std;

#define FAILURE -1
#define SUCCESS 0

//==== lib errors
#define LIB_ERROR_MESSAGE "thread library error: "
#define INVALID_TID_ERR "Invalid tid"
#define MAX_THREAD_NUM_ERR "The number of threads has exceeded the maximum"
#define INVALID_QUANTUM_USECS_ERR "Invalid quantum_usecs"

//==== sys errors
#define SYS_ERROR_MESSAGE "system error: "
#define FAILED_ALLOC_ERR "error in alloc"
#define SIGEMPTYSET_ERR "error in sigemptyset"


#define ERROR_EXIT_CODE 1
#define SUCCESS_EXIT_CODE 0





typedef unsigned long int address_t;

enum State{Ready, Blocked, Running};


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
int quantum_usecs;
int quantum_secs;
int cur_tid;


/// helper functions

//blackbox translation
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
	Thread* newThread;
	try
	{
		newThread = new Thread();
	}
	catch (bad_alloc &)
	{
		cerr << SYS_ERROR_MESSAGE << FAILED_ALLOC_ERR << endl;
		exit_and_free(ERROR_EXIT_CODE);
	}
	newThread->id = get_next_id();
	newThread->state = State::Running;
	sigsetjmp(newThread->env, 1);
	all_threads[0] = newThread;

	// TODO figure out time stuff

	return SUCCESS;

}




int uthread_spawn(thread_entry_point entry_point){
    int id = get_next_id();
    if(id == FAILURE){
		cerr << LIB_ERROR_MESSAGE << MAX_THREAD_NUM_ERR << endl;
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
		exit_and_free(ERROR_EXIT_CODE);
	}
	newThread->id = id;
	newThread->quanta = 0;
	newThread->state = State::Ready;
	address_t sp, pc;
	sp = (address_t) (newThread->stack) + STACK_SIZE - sizeof(address_t);
	pc = (address_t) entry_point;
	sigsetjmp(newThread->env, 1);
	// TODO figure out this section
	//(newThread->env->__jmpbuf)[JB_SP] = translate_address(sp);
	//(newThread->env->__jmpbuf)[JB_PC] = translate_address(pc);
	//if (sigemptyset(&newThread->env->__saved_mask) == FAILURE)
	//{
	//	cerr << SYS_ERROR_MESSAGE << SIGEMPTYSET_ERR << endl;
	//}

    //todo signal (unmask alarm signal)

}

int uthread_terminate(int tid){
    if (is_invalid_tid(tid)){
		cerr << LIB_ERROR_MESSAGE << INVALID_TID_ERR << endl;
        return FAILURE;
    }
    if(tid == 0){
        return exit_and_free(SUCCESS_EXIT_CODE);
    }
    //if(tid in Ready){ //todo
    //delete from ready
    //}

    delete all_threads[tid];
    all_threads[tid] = nullptr;
    available_ids[tid] = true;
    return SUCCESS;
}

