#include "uthreads.h"


#include <stdlib.h>
using namespace std;

#define FAILURE -1;
#define SUCCESS 0;

enum State{Ready, Blocked, Running};

typedef struct Thread{
    int id;
    int quanta;
    sigjmp_buf env;
    State state;
    char stack[STACK_SIZE]

    Thread(int id){
        this->id = id;
        this->quanta = 0;
        this->state = State::Ready;
        address_t sp, pc;
        sp = (address_t) (this->stack) + STACK_SIZE - sizeof(address_t);
        pc = (address_t) f;
        sigsetjmp(this->env, 1);
        (this->env->__jmpbuf)[JB_SP] = translate_address(sp);
        (this->env->__jmpbuf)[JB_PC] = translate_address(pc);
        if (sigemptyset(&this->env->__saved_mask) == FAILURE)
        {
            //todo handle error
        }
        //todo what happens to stack?

    }
};

//global stuff to figure out
bool[MAX_THREAD_NUM] available_ids = new bool[MAX_THREAD_NUM];
int quantum;
Thread[MAX_THREAD_NUM] all_threads = new Thread[MAX_THREAD_NUM];
int cur_tid;


int uthread_init(int quantum_usecs){
    for(int i = 0; i < MAX_THREAD_NUM; i++){
        available_ids[i] = true;
        all_threads[i] = nullptr;
    }
}

int get_next_id(){
    for(int i = 0; i < MAX_THREAD_NUM; i++){
        if(available_ids[i]){
            available_ids[i] = false;
            return i;
        }
    }
    return FAILURE;
}


int uthread_spawn(thread_entry_point entry_point){
    int id = get_next_id();
    if(id == FAILURE){
        //todo handle error write to stderr
        return FAILURE;
    }
    Thread thread = new Thread(id);
    //todo signal (unmask alarm signal)

}

int uthread_terminate(int tid){
    if (tid < 0 || tid >= MAX_THREAD_NUM || all_threads[tid] == nullptr){
        //todo handle error write to stderr
        return FAILURE;
    }
    if(tid == 0){
        return free_all();
    }
    if(tid in Ready){ //todo
    //delete from ready
    }
    delete all_threads[tid];
    all_threads[tid] = nullptr;
    available_ids[tid] = true;
    return SUCCESS;
}

int free_all(){
    for(int i=0; i<MAX_THREAD_NUM; i++){
        if(all_threads[i] != nullptr){
            delete all_threads[i];
        }
    }
    delete all_threads;
    delete available_ids;
    //todo add other globals
    return SUCCESS;
}