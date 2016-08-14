#include "tthreads.hpp"
#include "pwarpper.h"
#include "mgr_thread.h"
#include "vthreads.hpp"
#include "hglobal.h"
#include <atomic>
#include <time.h>
#include <sys/sysinfo.h>
#include "hutil.h"

using namespace hthread;

#ifdef USE_HSEM
int tthreads::target_num;
#endif

atomic<int> tthreads::active_threads_count;
int tthreads::core_num;

extern void init_task_num();

tthreads::tthreads(int n_threads) : \
    //real_threads(*new unordered_map<pthread_t, tthread*>())
    real_threads(*new vector<tthread*>())
{ 
    init_task_num();
    sem_init(&num_threads, 0, N_THREADS);
    ts_mutex = PTHREAD_MUTEX_INITIALIZER;
    //ts_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
    ts_cond = PTHREAD_COND_INITIALIZER;
    //active_threads_count = ATOMIC_VAR_INIT(n_threads);
    core_num = get_nprocs();
    active_threads_count = ATOMIC_VAR_INIT(core_num);
    #ifdef USE_HSEM
    target_num = core_num;
     #endif
    debug("enter tthreads constructor\n");    
    pthread_key_create (&thread_schd_key, NULL);  
    pthread_key_create (&cur_hcoroutine, NULL);  
    add_pthread(n_threads);
    core_id = 0;

    pthread_t tt;
    (*pwarpper::raw_pthread_create)(&tt, NULL, \
                mgr_routine, NULL);
    //pschd_locks = new int[get_nprocs()];
}

void tthreads::add(tthread* ptth) {
    //real_threads.insert(std::make_pair(ptth->pid, ptth));
    real_threads.push_back(ptth);
    debug("ptth->thread: %d\n", ptth->pid);
}

void tthreads::lock(){
    (*pwarpper::raw_pthread_mutex_lock)(&ts_mutex);
}

void tthreads::unlock(){
    (*pwarpper::raw_pthread_mutex_unlock)(&ts_mutex);
}

void tthreads::add_pthread(int n_threads) {
    debug("enter add_pthread\n");
    pthread_t thread;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    debug("worker_routine: %llx\n", worker_routine);
    debug("*pwarpper::raw_pthread_create: %llx\n", *pwarpper::raw_pthread_create);
    for (int i = 0; i < n_threads; i++)
    {

        tthread* ptth = new tthread(thread, attr, core_id++);
        //lock();
        (*pwarpper::raw_pthread_create)(&(ptth->pid), &(ptth->attr), \
                worker_routine, (void *) ptth);
        debug("after create\n");        
        add(ptth);
        
        if ( core_id >= core_num )
            core_id = 0;
        //unlock();
    }
}

struct tthread* tthreads::get() {
//    auto iter = real_threads.begin();
//    struct tthread* ptth = iter->second;
//    return ptth;
    return nullptr;
}

struct tthread* tthreads::find(pthread_t tid) {
/*
 * if ( real_threads.find(tid) == real_threads.end() )
        printf("it not there\n");
    struct tthread *ptth = real_threads[tid];
    return ptth;
*/
}

void tthreads::erase(pthread_t pth) {
//    delete real_threads[pth];
//    real_threads.erase(pth);
    //atomic_fetch_add(&active_threads_count, 1);
}

void tthreads::cancel(pthread_t pth) {
    debug("cancelling in tthreads. \n");
    
    (*pwarpper::raw_pthread_cancel)(pth);    
}

unsigned long tthreads::size() {
    size_t s = real_threads.size();
    return s;
}

tthreads::~tthreads() {
    for (auto iter = real_threads.begin(); iter != real_threads.end(); iter++) {
        //(*pwarpper::raw_pthread_cancel)(iter->first);
    }
    //delete &ts;
    // comment uppers to avoid "core dump" 
}

void tthreads::cond_wait(){
    (* pwarpper::raw_pthread_cond_wait)(&ts_cond, &ts_mutex);
}

void tthreads::cond_signal(){
    pthread_cond_signal(&ts_cond);
}

pthread_t tthreads::t2v(pthread_t tid){
/*
 * auto iter = real_threads.find(tid);
    if(iter==real_threads.end())
        return (* pwarpper::raw_pthread_self)();
    return iter->second->task->vid;
*/
}

tthreads* tthreads::getInstance(int n_threads){
    debug("enter tthreads getInstance\n");
    static tthreads* ptth = new tthreads(n_threads);
    return ptth;
}

pthread_t tthreads::get_tid(){
    return (* pwarpper::raw_pthread_self)();
}

sem_t tthreads::num_threads;

#if defined(USE_FUTEX) || defined(USE_MUTEX)
static hschd_lock tthreads::schd_locks[8]  __attribute__((aligned(64)));
#endif


