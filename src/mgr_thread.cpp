#include "tthreads.hpp"
#include <stdio.h>
#include "strategy.h"
#include "mgr_thread.h"
#include "unistd.h"
#include <pthread.h>
#include "pwarpper.h"
#include "hglobal.h"
#include "vthread.hpp"
#include "tthreads.hpp"
#include <sys/sysinfo.h>
#include "hutil.h"

using namespace hthread;

inline void * read_rsp() {
    void* volatile ret;
    __asm__ __volatile__ ( "movq %%rsp, %0": :"a"(ret) );
    return ret;
}

void task_num_update_handler(int signum){
    //printf("udpate task number: %d\n", 0);
    int task_number;
    if ( !ptask_number ) task_number = 1;
    else task_number = *ptask_number;
    task_number = task_number == 0 ? 1 : task_number;
    //    printf("task number: %d\n", task_number);
    if ( task_number >= N_THREADS ) N_THREADS_ALLOWED = 1;
    else {        
        N_THREADS_ALLOWED = N_THREADS / task_number ;
        if ( N_THREADS % task_number  ) {
            N_THREADS_ALLOWED++;
        }
    }
    printf ( "N_THREADS_ALLOWED: %d \n", N_THREADS_ALLOWED);
}

void sig_handler(int signum)
{
    //    task_num_update_handler(TASK_NUM_UPDATE_SIGNAL);
    static bool flat = true;
    static bool flat2 = true;
    vthread* h = (vthread*)(*pwarpper::raw_pthread_getspecific)(cur_hcoroutine);
    if ( h == 0 ) return;
    //    unsigned long long rs = (unsigned long long)read_rsp();
    void *io;
    void *rs = &io;

    //    cout << "h = " << h  << endl;
    //    cout << "h stack = " << (void*)h->sp  << endl;
    //    cout << "rs = " << rs  << endl;
    //    printf("rs = %lx\n", rs);
    //    cout << "vthread size = " << sizeof(vthread) << endl;

    if ( (unsigned long long)rs > (unsigned long long)h && \
            (unsigned long long)rs - (unsigned long long)h < sizeof(vthread) &&
            h->cur_status != STATUS_BLOCKED &&
            !h->uninteruptable ) {
        //    cout << "scheduled out" << endl;
        if ( h->to_be_cancelled ) {
            h->try_yield_myself(STATUS_FINISHED);
        }
        else {
            if ( (void *)23 == h->try_yield_myself() ) cout << "failed to jump" << endl;
        }
    }
}

static void worker_cleanup(void *arg) {
    debug("cleanup\n");
    strategy::tthread_cleanup(arg);
}

// to implemented in the future for avoid dead lock when a thread is cancelled
void* mgr_routine(void *p) {
    debug("starting a mgr\n");
    //    return nullptr;

    int time_slice = 1000000;
    const char *str_ts = getenv("HTHREAD_TIME_SLICE");
    if ( str_ts )
        time_slice = atoi( str_ts ) * 1000;
    printf("/*time_slice: %d*/\n", time_slice);

    while ( true ) {


        task_num_update_handler(TASK_NUM_UPDATE_SIGNAL);
        usleep(time_slice);
        //        sleep(200);
        //printf("do a schdule, task_num = %d\n", *ptask_number);
        //killpg(getpid(), SIGUSR1);
        for ( auto it = ts->real_threads.begin(); \
                it != ts->real_threads.end(); it++ ) {
            //cout << "pid: " << (*it)->pid << endl;
            //printf("pid: 0x%lx\n", (*it)->pid);
            if ( (*it)->task )
                pthread_kill((*it)->pid, SIGUSR1);
        }
    }
}

void* worker_routine(void *p) {
    debug("starting a worker\n");
    pthread_cleanup_push(worker_cleanup, NULL);
    tthread *ptth = (tthread*)p;
    pthread_t myid = (* pwarpper::raw_pthread_self)();
    //printf("true pid: 0x%lx\n", myid);
    vthread *pvth = NULL;

    printf("/*core_id: %d*/ \n", ptth->core_id);
    signal(SIGUSR1, sig_handler);



    while (true) {

        if ( ptth->core_id >= N_THREADS_ALLOWED ) {
            //printf("%d: cannot run\n", (ptth->core_id) );
            usleep(200000);
            continue;
        }
        debug("starting a worker\n");
        pvth = vs->get_and_work(ptth);
        if (!pvth->runnable) {
            vs->vs_queue.enqueue(pvth);
            continue;
        }
        ptth->task = pvth;
        debug("get 1 vthread\n");
        debug("vs->size: %d\n", vs->wait_size());
        debug("vs->working_size: %d\n", vs->working_size());
        //strategy::start_vthread();
        //printf("get 1 vthread: %d\n", pvth->vid);
        hcoroutine_ret_t ret = (hcoroutine_ret_t)pvth->resume();
        ptth->task = nullptr;

       // printf("return from 1 vthread: %lx ret: %lx\n", pvth, ret);

        if ( ret == STATUS_FINISHED ) {
            //printf("finish 1 vthread: %d\n", pvth->vid);
            //printf("finish 1 vthread: %d\n", pvth->vid);
            //printf("true pid: 0x%ux\n", myid);
            strategy::finish_vthread(ptth, pvth);
        } 
#ifdef SPIN_LOCK_STYLE
        if ( ret == STATUS_BLOCKED ) {
            pvth->runnable = true;
            continue;
        }
        if ( ret == STATUS_SUSPENDED || ret == STATUS_LOCKFAIL ) {
//            usleep(3);
            vs->vs_queue.enqueue(pvth);
            continue;
        }
        else if ( ret > 5 ) {
            hthread_mutex_t* hm = (hthread_mutex_t*) ret;
            __sync_lock_release (&hm->c);
            pvth->runnable = true;
            if (pvth->cur_status == STATUS_SUSPENDED) {
                vs->vs_queue.enqueue(pvth);
            }
            //hm->release_access();
#ifdef LOCKFREE_COND
            //version 2
            vs->vs_queue.enqueue(pvth);
#endif
            continue;
        }
#endif
        /*
           else if ( ret > 5 ) {

           hthread_mutex_t* hm = (hthread_mutex_t*) ret;
           __sync_lock_release (&hm->c);
        //hm->release_access();
        }
        */
    }

    pthread_cleanup_pop(0);
}
