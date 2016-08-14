/* 
 * File:   strategy.h
 * Author: xu
 *
 * Created on March 15, 2016, 3:29 PM
 */

#ifndef STRATEGY_H
#define	STRATEGY_H
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <dlfcn.h>
#include "pwarpper.h"
#include <assert.h>
#include <hglobal.h>
#include "tthreads.hpp"
#include "pwarpper.h"
#include "mgr_thread.h"
#include "vthread.hpp"
#include "vthreads.hpp"
#include "hglobal.h"
#include <atomic>
#include <time.h>
#include <sys/sysinfo.h>
#include "hutil.h"
#include <pthread.h>

using namespace hthread;

class strategy {
    public:
        strategy();
        virtual ~strategy();

        inline static int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                void *(*start_routine) (void *), void *arg)
            __attribute__((always_inline));
        inline static int pthread_cancel(pthread_t thread)
            __attribute__((always_inline));
        inline static pthread_t pthread_self(void)
            __attribute__((always_inline));
        inline static int pthread_barrier_wait(pthread_barrier_t *barrier)
            __attribute__((always_inline));
        inline static int strategy::pthread_barrier_init(pthread_barrier_t * barrier,
                const pthread_barrierattr_t * attr, unsigned count)
            __attribute__((always_inline));
        inline static int pthread_barrier_destroy(pthread_barrier_t *barrier)
            __attribute__((always_inline));
        inline static int pthread_join(pthread_t thread, void **retval)
            __attribute__((always_inline));
        inline static void pthread_exit(void *retval) __attribute__((noreturn))
            __attribute__((always_inline));
        inline static void tthread_cleanup(void *arg)
            __attribute__((always_inline));
        inline static int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
            __attribute__((always_inline));
        inline static int pthread_cond_signal(pthread_cond_t *cond)
            __attribute__((always_inline));
        inline static int pthread_cond_broadcast(pthread_cond_t *cond)
            __attribute__((always_inline));
        inline static int pthread_mutex_lock(pthread_mutex_t *mutex)
            __attribute__((always_inline));
        inline static int pthread_mutex_lock(pthread_mutex_t *mutex, vthread* me)
            __attribute__((always_inline));
        inline static int pthread_mutex_trylock(pthread_mutex_t *mutex)
            __attribute__((always_inline));
        inline static int pthread_mutex_unlock(pthread_mutex_t *mutex)
            __attribute__((always_inline));
        inline static int pthread_cond_timedwait(pthread_cond_t *cond,
                pthread_mutex_t *mutex,
                const struct timespec *abstime)
            __attribute__((always_inline));
        inline static int sem_wait(sem_t *sem)
            __attribute__((always_inline));
        inline static int sem_trywait(sem_t *sem) __attribute__((always_inline));
        inline static int sem_post(sem_t *sem) __attribute__((always_inline));
        inline static void reach_target_num_threads() __attribute__((always_inline));
        inline static void yield_running() __attribute__((always_inline));
        inline static void back_running() __attribute__((always_inline));
        inline static void start_vthread() __attribute__((always_inline));
        inline static void finish_vthread(tthread *ptth, vthread *pvth) __attribute__((always_inline));

        inline static void *pthread_getspecific(pthread_key_t key);
        inline static int pthread_setspecific(pthread_key_t key, const void *value);
        inline static int sched_yield(void) __attribute__((always_inline));
    private:

};

int strategy::pthread_create(pthread_t *thread, const pthread_attr_t *attr,
        void *(*start_routine) (void *), void *arg) {
    debug("h thread is starting....\n");
    debug("func @ addr: %llx\n", start_routine);
    debug("arg @ addr: %llx\n", arg);
    //vs->lock();
    vs->add_vthread(thread, attr, start_routine, arg);
    debug("add 1 vthread\n");
    debug("vs->size: %d\n", vs->wait_size());
    debug("vs->working_size: %d\n", vs->working_size());
    //vs->cond_signal();
    //vs->unlock();
    return 0;
}

/*
   int strategy::pthread_cancel(pthread_t thread) {
   debug("h thread is cancelling....\n");    
   vs->lock();
   if ( vs->erase_wait(thread) == true )  {
   vs->unlock();
   return 0;
   }
   pthread_t actual_pid = vs->v2t(thread);
   vs->end_vthread(thread);
   vs->unlock();
   ts->lock();
   ts->erase(actual_pid);
   ts->unlock();
   ts->cancel(actual_pid);
// need to know the status of the thread to add following comments
//atomic_fetch_add(&tthreads::active_threads_count, -1);
//ts->v();
return 0;
}
*/

int strategy::pthread_cancel(pthread_t thread) {
    vid_to_pvth[thread].to_be_cancelled = true;
}

pthread_t strategy::pthread_self(void) {    
    pr = pwarpper::getInstance();
    vs = vthreads::getInstance();
    N_THREADS = sysconf( _SC_NPROCESSORS_ONLN );
    ts = tthreads::getInstance(N_THREADS);

    //debug("called here \n" );
    vthread *me = reinterpret_cast<vthread *> \
                  ((*pwarpper::raw_pthread_getspecific)(cur_hcoroutine));
    debug("me %d\n", me);
    if ( me ) return me->vid;
    pthread_t vid = (* pwarpper::raw_pthread_self)();
    debug("vid %d\n", vid);
    return vid;
}
/*
 * lock free verion, but it seems do not work on current queue
 *
 */
#ifdef LOCKFREE_BARRIER
int strategy::pthread_barrier_wait(pthread_barrier_t *barrier) {
    debug("pthread_barrier_wait called\n");
    //   printf("pthread_barrier_wait called\n");
    vthread *me = reinterpret_cast<vthread *> \
                  ((*pwarpper::raw_pthread_getspecific)(cur_hcoroutine));
    volatile hthread_barrier_t *hb = (hthread_barrier_t*)barrier;
    int cur_round = hb->cur_round;
    int cur_waiter = __sync_add_and_fetch(&hb->cur_waiters, 1);
    //cout << "hb->waiters : " << hb->waiters << endl; 
    //cout << "cur_waiter : " << cur_waiter << endl;
    if (cur_waiter != hb->waiters) {
        while (true) {
            if (cur_round == hb->cur_round) {
                //    printf("failed again\n");
                me->yield_myself();
            }
            else if (cur_round + 1 == hb->cur_round)
                return 0;
            else
                printf("should not run to here");
        }
    }
    else {
        if (!__sync_bool_compare_and_swap(&hb->cur_waiters, cur_waiter, 0))
            printf("should not run to here");
        printf("wake up other guys\n");;
        __sync_add_and_fetch(&hb->cur_round, 1);
        return PTHREAD_BARRIER_SERIAL_THREAD;
    }
    return 0;
}
#endif

#ifdef BLOCKED_BARRIER
int strategy::pthread_barrier_wait(pthread_barrier_t *barrier) {
    debug("pthread_barrier_wait called\n");
    //printf("pthread_barrier_wait called\n");
    vthread *me = reinterpret_cast<vthread *> \
                  ((*pwarpper::raw_pthread_getspecific)(cur_hcoroutine));
    volatile hthread_barrier_t *hb = (hthread_barrier_t*)barrier;
    volatile int cur_round = hb->cur_round;
    volatile int cur_waiter = __sync_add_and_fetch(&hb->cur_waiters, 1);
    //    printf( " %lx hb->waiters : %d\n", hb, hb->waiters);
    //    printf( " %lx cur_waiter : %d\n", hb,  cur_waiter);
    //    if (hb->waiters == 13) {
    //        cout << "big error!!!" << endl;
    //        exit(-1);
    //    }
    if (cur_waiter != hb->waiters) {
        //vthread_node::enqueue(&hb->waiting_vthreads, &me->thisnode);
        me->runnable = false;
        while(!hb->blocked_vths[1&cur_round]->push(me)) {
            (pwarpper::raw_sched_yield)();
        }
        //printf("%d-th waiter in barreri %lx in round %d\n", cur_waiter, hb, cur_round);
        me->yield_myself(STATUS_BLOCKED);
    }
    else {
        if (!__sync_bool_compare_and_swap(&hb->cur_waiters, cur_waiter, 0))
            printf("should not run to here");
        __sync_add_and_fetch(&hb->cur_round, 1);
        //printf("%lx wake up other guys in round %d\n", hb, cur_round);
        while ((--cur_waiter)>0) {

            //    vthread_node* pvth_node;
            //    while( !(pvth_node = vthread_node::safe_dequeue(&hb->waiting_vthreads) )) {
            //    me->yield_myself();
            //   }
            //   printf("cur_waiter %lx %d in round %d\n", hb, cur_waiter, cur_round);
            vthread *blocked_v;
            while(true) {
                if (hb->blocked_vths[1&cur_round]->pop(blocked_v)) { 
                    //            printf("cur_waiter %lx %d in round %d\n", hb, cur_waiter, cur_round);
                    //           puts("get one");
                    break;
                }
                //     printf("failed to read\n");
                me->yield_myself();
            }
            //   printf("before\n" );
            vs->vs_queue.enqueue(blocked_v); 
            //  printf("after\n");
        }
        //printf("%lx finish wake up other guys\n", hb);
        return PTHREAD_BARRIER_SERIAL_THREAD;
    }
    return 0;
}
#endif

int strategy::pthread_barrier_init(pthread_barrier_t * barrier,
        const pthread_barrierattr_t * attr, unsigned count) {
    //printf("pthread_barrier_init called: %d in barrier %lx\n", count, barrier);
    volatile hthread_barrier_t *hb = (hthread_barrier_t*)barrier;
    hb->waiters = count;
    hb->cur_waiters = 0;
    hb->cur_round = 0;
    //    hb->waiting_vthreads = nullptr;
    hb->blocked_vths[0] = new boost::lockfree::queue<vthread*, \
                          boost::lockfree::fixed_sized<true> >(count<<1);
    hb->blocked_vths[1] = new boost::lockfree::queue<vthread*, \
                          boost::lockfree::fixed_sized<true> >(count<<1);
    return 0;
}

int strategy::pthread_barrier_destroy(pthread_barrier_t *barrier) {
    printf("pthread_barrier_destroy called: %d\n", 0);
    volatile hthread_barrier_t *hb = (hthread_barrier_t*)barrier;
    //delete hb->blocked_vths[0];
    //delete hb->blocked_vths[1];
    return strategy::pthread_barrier_init(barrier, NULL, 0);
}

int strategy::pthread_join(pthread_t thread, void **retval) {
    debug("h thread is joining....\n");    
    //printf("target is %d\n", thread);
    vthread::join(vid_to_pvth + thread, retval);
    return 0;
}

void strategy::pthread_exit(void *retval){
    debug("h thread is exiting....\n");
    vthread *me = reinterpret_cast<vthread *> \
                  ((*pwarpper::raw_pthread_getspecific)(cur_hcoroutine));
    //pthread_spin_lock(&me->join_spinlock);
    while(__sync_lock_test_and_set(&me->join_mutex.c, 1)) {;
        //for ( int i = 0; i < 1000; i++) {}
        me->yield_myself(STATUS_LOCKFAIL);
    }
    me->routine_ret = retval;
    __sync_lock_release (&me->join_mutex.c);
    //pthread_spin_unlock(&me->join_spinlock);
    me->yield_myself(STATUS_FINISHED);
}

void strategy::tthread_cleanup(void* arg){

}

#ifndef LOCKFREE_COND 
//version 1
int strategy::pthread_cond_wait(pthread_cond_t* cond, pthread_mutex_t* mutex) {
    debug("pthread_cond_wait called\n");
    //printf("pthread_cond_wait called\n");

    hthread_cond_t* hc = (hthread_cond_t*)cond;
    while (hthread_unlikely(!hc->blocked_vths)) {
        hcond_lockfree_queue *tmp = new hcond_lockfree_queue(16);
        if (__sync_bool_compare_and_swap(&hc->blocked_vths, nullptr, tmp))
            break;
        else
            delete tmp;
    }
    hthread_mutex_t *hm = (hthread_mutex_t *) mutex;
    //hc->mutex = mutex;
    vthread *me = reinterpret_cast<vthread *> \
                  ((*pwarpper::raw_pthread_getspecific)(cur_hcoroutine));
    //printf("%lx wait cond %lx with mutex %lx\n", me, cond, mutex);
    //hc->waiter_num++;
    __sync_add_and_fetch(&hc->waiter_num, 1);
    //printf("hc->waiter_num:  %d\n", hc->waiter_num);
    me->cur_status = STATUS_BLOCKED;
    //vthread_node::enqueue(&hc->waiting_vthreads, &me->thisnode);
    me->runnable = false;
    while (!hc->blocked_vths->push(me));
    //printf("insert myself %lx into queue %lx\n with size(): ", me, hc->blocked_vths, 88888);
    me->yield_myself(reinterpret_cast<size_t>(hm));
    me->cur_status = STATUS_SUSPENDED;
    while (hthread_unlikely(__sync_lock_test_and_set(&hm->c, 1)))
    {
        me->yield_myself(STATUS_LOCKFAIL);
    }
    //hc->waiter_num--;
    //__sync_add_and_fetch(&hc->waiter_num, -1);
    return 0;
}

#else
//version 2
int strategy::pthread_cond_wait(pthread_cond_t* cond, pthread_mutex_t* mutex) {
    debug("pthread_cond_wait called\n");
    //printf("pthread_cond_wait called\n");
    hthread_cond_t* hc = (hthread_cond_t*)cond;
    hthread_mutex_t *hm = (hthread_mutex_t *) mutex;
    hc->mutex = mutex;
    vthread *me = reinterpret_cast<vthread *> \
                  ((*pwarpper::raw_pthread_getspecific)(cur_hcoroutine));
    //hc->hmutex = hm;
    //__sync_lock_release (&hm->c);
    //__sync_fetch_and_add(&hc->waiter_num, 1);
    while ( hc->waker_blocked == 2 ) {
        me->yield_myself(reinterpret_cast<size_t> (hm));
        while (hthread_unlikely(__sync_lock_test_and_set(&hm->c, 1)))
        {
            me->yield_myself(STATUS_LOCKFAIL);
        }
    }
    hc->waiter_num++;
    //printf("hc->waiter_num = %d\n", hc->waiter_num);
    do
    {
        // hm would be unlocked by the worker
        me->yield_myself(reinterpret_cast<size_t> (hm));
        // lock the mutex
        //printf("try lock\n");
        while (hthread_unlikely(__sync_lock_test_and_set(&hm->c, 1)))
        {
            me->yield_myself(STATUS_LOCKFAIL);
        }
    }
    while (hc->signals <= 0);
    hc->signals--;
    //printf("hc->signals = %d\n", hc->signals);
    hc->waiter_num--;
    //printf("hc->waiter_num = %d\n", hc->waiter_num);
    if (hc->waker_blocked == 1 || \
            hc->waker_blocked == 2 && hc->waiter_num == 0)
    {
        hc->waker_blocked = 0;
    }
    return 0;
}
#endif

#ifndef LOCKFREE_COND 
int strategy::pthread_cond_signal(pthread_cond_t* cond) {
    debug("pthread_cond_signal called\n");
    //printf("pthread_cond_signal called\n");
    hthread_cond_t* hc = (hthread_cond_t*)cond;
    //hthread_mutex_t *hm = (hthread_mutex_t *) hc->mutex;
    //if (!hc->waiter_num) return 0;
    //vthread_node *waiter = vthread_node::dequeue(&hc->waiting_vthreads);
    vthread *waiter;

    while (hthread_unlikely(!hc->blocked_vths)) {
        printf("blocked_vths : %lx\n", hc->blocked_vths);
        hcond_lockfree_queue *tmp = new hcond_lockfree_queue(16);
        if (__sync_bool_compare_and_swap(&hc->blocked_vths, nullptr, tmp))
            break;
        else
            delete tmp;
    }
//    printf("blocked_vths : %lx\n", hc->blocked_vths);
 //   if ( !hc->waker_blocked) {
 //       printf("empty call1 %lx\n", waiter);
 //       return 0;
//    }
    vthread *me = reinterpret_cast<vthread *> \
                  ((*pwarpper::raw_pthread_getspecific)(cur_hcoroutine));
#ifdef MUST_WAKE
    while (hc->waiter_num) {
        if (hc->blocked_vths->pop(waiter)) {
            __sync_add_and_fetch(&hc->waiter_num, -1);
            printf("hc->waiter_num:  %d\n", hc->waiter_num);
            break;
        } else {
            me->yield_myself();
        }
    }
#else
    if (hc->blocked_vths->pop(waiter)) {
        __sync_add_and_fetch(&hc->waiter_num, -1);
        //   printf("hc->waiter_num:  %d\n", hc->waiter_num);
    }
    else
        return 0;
#endif
    if ( !waiter ) {
        //    puts("empty call2, big error");
        //    exit(-1);
        return 0;
    }
    waiter->cur_status = STATUS_SUSPENDED;
    vs->vs_queue.enqueue(waiter);
    //printf("wakeup %lx in  cond %lx with mutex %lx\n", waiter, cond, hm);
    //me->cur_status = STATUS_SUSPENDED;
    //me->yield_myself(reinterpret_cast<size_t> (hm));
    // lock the mutex
    //printf("try lock\n");
    //while (hthread_unlikely(__sync_lock_test_and_set(&hm->c, 1)))
    //{
    //    me->yield_myself(STATUS_LOCKFAIL);
    //}
    //    vthread *me = nullptr;
    //    __sync_fetch_and_add(&hc->waiter_num, -1);
    //    printf("signal hc->c = %d\n", hc->c);
    //    __sync_fetch_and_add(&hc->c, +1);
    //    printf("signal hc->waiter_num = %d\n", hc->waiter_num);
    //    printf("signal hc->c = %d\n", hc->c);
    return 0;
}

#else
// version 2
int strategy::pthread_cond_signal(pthread_cond_t* cond) {
    debug("pthread_cond_signal called\n");
    //    printf("pthread_cond_signal called\n");
    hthread_cond_t* hc = (hthread_cond_t*)cond;
    hthread_mutex_t *hm = (hthread_mutex_t *) hc->mutex;
    vthread *me = reinterpret_cast<vthread *> \
                  ((*pwarpper::raw_pthread_getspecific)(cur_hcoroutine));
    //printf("hc->signals = %d\n", hc->signals);
    //printf("hc->waiter_num = %d\n", hc->waiter_num);
    if (hc->waiter_num > 0) {
        hc->signals++;
        hc->waker_blocked = 1;
        //me->yield_myself(reinterpret_cast<size_t> (hm));


        //while (hthread_unlikely(__sync_lock_test_and_set(&hm->c, 1)))
        //{
        //    me->yield_myself(STATUS_LOCKFAIL);
        //}
    }
    //printf("hc->signals = %d\n", hc->signals);
    //printf("hc->waiter_num = %d\n", hc->waiter_num);
    return 0;
}
#endif

#ifndef LOCKFREE_COND 
//version 1
int strategy::pthread_cond_broadcast(pthread_cond_t* cond) {
    debug("pthread_cond_broadcast called\n");
    //puts("pthread_cond_broadcast called\n");
    hthread_cond_t* hc = (hthread_cond_t*)cond;
    hthread_mutex_t *hm = (hthread_mutex_t *) hc->mutex;
    while (hthread_unlikely(!hc->blocked_vths)) {
        hcond_lockfree_queue *tmp = new hcond_lockfree_queue(16);
        if (__sync_bool_compare_and_swap(&hc->blocked_vths, nullptr, tmp))
            break;
        else
            delete tmp;
    }
    if ( !hc->blocked_vths  ) return 0;

    vthread *me = reinterpret_cast<vthread *> \
                  ((*pwarpper::raw_pthread_getspecific)(cur_hcoroutine));

    vthread *waiter;

    //#define MUST_WAKE
#ifdef MUST_WAKE
    while(hc->waiter_num) {
        //waiter = vthread_node::dequeue(&hc->waiting_vthreads);
        if (hc->blocked_vths->pop(waiter)) {
            __sync_add_and_fetch(&hc->waiter_num, -1);
            //      printf("hc->waiter_num:  %d\n", hc->waiter_num);
            waiter->
                vs->vs_queue.enqueue(waiter);
        } else {
            me->yield_myself();
        }
        //   printf("wakeup/broadcast %lx in  cond %lx with mutex %lx\n", waiter, cond, hm);
    }
#else 
    //waiter = vthread_node::dequeue(&hc->waiting_vthreads);
    while (hc->blocked_vths->pop(waiter)) {
        __sync_add_and_fetch(&hc->waiter_num, -1);
        // printf("hc->waiter_num:  %d\n", hc->waiter_num);
        vs->vs_queue.enqueue(waiter);
    } 
#endif

    me->cur_status = STATUS_SUSPENDED;
    //    me->yield_myself(reinterpret_cast<size_t> (hm));
    //    while (hthread_unlikely(__sync_lock_test_and_set(&hm->c, 1)))
    //    {
    //        me->yield_myself(STATUS_LOCKFAIL);
    //    }
    return 0;
}
#else
//version 2
int strategy::pthread_cond_broadcast(pthread_cond_t* cond) {
    debug("pthread_cond_broadcast called\n");
    //    puts("pthread_cond_broadcast called\n");
    hthread_cond_t* hc = (hthread_cond_t*)cond;

    hthread_mutex_t *hm = (hthread_mutex_t *) hc->mutex;
    vthread *me = reinterpret_cast<vthread *> \
                  ((*pwarpper::raw_pthread_getspecific)(cur_hcoroutine));

    if (hc->waiter_num > 0) {
        hc->signals = hc->waiter_num;
        hc->waker_blocked = 2;
        do {
            me->yield_myself(reinterpret_cast<size_t> (hm));

            while (hthread_unlikely(__sync_lock_test_and_set(&hm->c, 1)))
            {
                me->yield_myself(STATUS_LOCKFAIL);
            }
        } while(hc->waker_blocked);
    }
    //printf("hc->signals = %d\n", hc->signals);
    //printf("hc->waiter_num = %d\n", hc->waiter_num);
    return 0;
}
#endif

int strategy::pthread_mutex_lock(pthread_mutex_t *mutex) {
    return strategy::pthread_mutex_lock(mutex, nullptr);
}

int strategy::pthread_mutex_lock(pthread_mutex_t *mutex, vthread *me){

    debug("lock called\n");
    hthread_mutex_t *hm = (hthread_mutex_t *) mutex;
    //hm->acquire_acess(); return 0;

    //vthread *me = reinterpret_cast<vthread *> \
    ((*pwarpper::raw_pthread_getspecific)(cur_hcoroutine));
    //vthread *me = nullptr;

#ifdef SPIN_LOCK_STYLE

    //        while ( !__sync_bool_compare_and_swap(&hm->c, 0, 1) ) {
    //        while ( __sync_lock_test_and_set(&hm->c, 1) ) {
    while ( hthread_likely (__sync_lock_test_and_set(&hm->c, 1) ) ) {
        //        while ( pthread_spin_trylock((pthread_spinlock_t*)hm) ) {
        //        while ( hspin_trylock((hspin_t*)hm) ) {
        if ( !me ) {
            me = reinterpret_cast<vthread *> \
                 ((*pwarpper::raw_pthread_getspecific)(cur_hcoroutine));
        }
        //me->yield_myself(reinterpret_cast<size_t>(hm));
        me->yield_myself(STATUS_LOCKFAIL);
    }
    return 0;

#endif

    if ( me )
    {
        hm->acquire_acess();
        //printf ( "hm->c: %ud\n", hm->c );
        while ( hm->c == 1 )
        {
            //            printf ( "me -> vid %d: wait lock\n", me->vid);
            //            printf ( "hm->waiting_vthreads: %lx\n", hm->waiting_vthreads);
            vthread_node::enqueue(&hm->waiting_vthreads, &me->thisnode);

            //            printf ( "hm->waiting_vthreads: %lx\n", hm->waiting_vthreads);
            //hm->release_access();

            //printf ( "me: %lx\n", me );
            //            printf ( "me->vid: %d\n", me->vid );
            //            printf ( "&me->thisnode: %lx\n", &me->thisnode );

            me->yield_myself(reinterpret_cast<size_t>(hm)); //let the real threads to the unlock work
            hm->acquire_acess();
        }
        //printf ( "me -> vid %d: got lock\n", me->vid);
        hm->c = 1;
        hm->release_access();
    } else {
        printf ( "should not come to here \n" );
        hm->acquire_acess();
        while ( hm->c == 1 )
        {
            hm->tthread_num++;
            hm->release_access();
            hm->bin_sema.wait();
            hm->acquire_acess();
        }
        hm->c = 1;
        hm->release_access();
    }
    return 0;
    }

    int strategy::pthread_mutex_unlock(pthread_mutex_t *mutex) {
        debug("unlock called\n");
        hthread_mutex_t *hm = (hthread_mutex_t *) mutex;

        //hm->release_access(); return 0;

        //vthread *me = reinterpret_cast<vthread *> \
        ((*pwarpper::raw_pthread_getspecific)(cur_hcoroutine));

#ifdef SPIN_LOCK_STYLE
        //    if ( me ) {
        //        __sync_bool_compare_and_swap(&hm->c, 1, 0);
        __sync_lock_release (&hm->c);
        //          pthread_spin_unlock((pthread_spinlock_t*)hm);
        //        __sync_synchronize();
        //        hm->c = 0;
        return 0;
        //    }
#endif

        hm->acquire_acess();
        vthread_node *vthNode;
        int waited_tthtread_num;
        vthNode = vthread_node::dequeue(&hm->waiting_vthreads);
        debug ( "line 210: hm->waiting_vthreads: %lx\n", hm->waiting_vthreads);
        waited_tthtread_num = hm->tthread_num;
        if ( hm->tthread_num > 0 ) hm->tthread_num--;
        hm->c = 0;
        hm->release_access();

        //printf ( "vthNode: %lx\n", vthNode );
        if ( vthNode ) {
            vs->vs_queue.enqueue(vthNode->vth);
            debug ( "unlock vthNode->vth->vid: %d\n", vthNode->vth->vid );
        } else if ( waited_tthtread_num > 0 ) {
            hm->bin_sema.post();
        }
        return 0;

    }

    int strategy::pthread_mutex_trylock(pthread_mutex_t *mutex)
    {
        debug("trylock called\n");
        hthread_mutex_t *hm = (hthread_mutex_t *) mutex;
        //    vthread *me = reinterpret_cast<vthread *> \
        //            ((*pwarpper::raw_pthread_getspecific)(cur_hcoroutine));
#ifdef SPIN_LOCK_STYLE
        if ( 0 ==  __sync_lock_test_and_set(&hm->c, 1) ) {
            return 0;
        }
        return EBUSY;
#endif


        hm->acquire_acess();

        if (hm->c == 1)
        {
            hm->release_access();
            return EBUSY;
        }

        hm->c = 1;
        hm->release_access();
        return 0;
    }

    int strategy::pthread_cond_timedwait(pthread_cond_t *cond,
            pthread_mutex_t *mutex,
            const struct timespec *abstime){
        debug("cond_timedwait called\n");
        yield_running();
        int rtn = (*pwarpper::raw_pthread_cond_timedwait)(cond, mutex, abstime);
        back_running();
        return rtn;
    }

    int strategy::sem_wait(sem_t *sem){
        debug("sem_wait\n");
        //printf("sem_wait\n");
        hthread_sem_t* hs = (hthread_sem_t*)sem;

        vthread *me = nullptr;


        while ( sem_trywait(sem) != 0 ) {
            if ( !me ) {
                me = reinterpret_cast<vthread *> \
                     ((*pwarpper::raw_pthread_getspecific)(cur_hcoroutine));
            }
            me->yield_myself(STATUS_LOCKFAIL);
        }

        /*
           while (true)
           {
           while (hs->c <= 0)
           {
           if (!me)
           {
           me = reinterpret_cast<vthread *> \
           ((*pwarpper::raw_pthread_getspecific)(cur_hcoroutine));
           }
           me->yield_myself(STATUS_LOCKFAIL);
           }
           int c = __sync_fetch_and_add(&hs->c, -1);
           if (c <= 0) __sync_fetch_and_add(&hs->c, 1);
           else return 0;
           }*/
        return 0;
    }

    int strategy::sem_trywait(sem_t *sem){
        debug("sem_trywait\n");
        //printf("sem_trywait\n");
        hthread_sem_t* hs = (hthread_sem_t*)sem;
        if (hs->c <= 0)
        {
            errno = EAGAIN;
            return EBUSY;
        }
        int c = __sync_fetch_and_add(&hs->c, -1);
        if ( c <= 0 ) {
            __sync_fetch_and_add(&hs->c, 1);
            return EBUSY;
        }
        return 0;
    }

    int strategy::sem_post(sem_t *sem){
        debug("sem_post\n");
        hthread_sem_t* hs = (hthread_sem_t*)sem;
        __sync_fetch_and_add(&hs->c, 1);
        return 0;
    }

    void strategy::yield_running() {
#ifndef FOR_OPENMP
        int active_threads = atomic_fetch_add(&tthreads::active_threads_count, -1);
        reach_target_num_threads();
#endif
        ts->v();
    }

    void strategy::back_running() {
        tthreads::p();
#ifndef FOR_OPENMP
        int active_threads = atomic_fetch_add(&tthreads::active_threads_count, 1);    
#endif
    }

    void strategy::start_vthread() {
        tthreads::p();  
    }

    void strategy::finish_vthread(tthread *ptth, vthread *pvth) {
        pvth->wake_up_joined();
    }

    void strategy::reach_target_num_threads() {
        //return;
        int active_threads;
        do {
            active_threads = atomic_load(&tthreads::active_threads_count);
            if (active_threads >= N_THREADS) {
                return;
            }        
        } while ( hthread_unlikely( ! atomic_compare_exchange_weak(&tthreads::active_threads_count, \
                        &active_threads, N_THREADS) ) );
        ts->add_pthread( N_THREADS - active_threads );
        debug("reach_target_num_threads: %d -> %d\n", active_threads, N_THREADS);
    }

    void * strategy::pthread_getspecific(pthread_key_t key){
        vthread *me = reinterpret_cast<vthread *> \
                      ((*pwarpper::raw_pthread_getspecific)(cur_hcoroutine));
        if ( me )
            return me->vtls[key];
        else {
            return ((*pwarpper::raw_pthread_getspecific)(key));
        }
    }

    int strategy::pthread_setspecific(pthread_key_t key, const void *value){
        vthread *me = reinterpret_cast<vthread *> \
                      ((*pwarpper::raw_pthread_getspecific)(cur_hcoroutine));
        if ( me )
            me->vtls[key] = value;
        else 
            ((*pwarpper::raw_pthread_setspecific)(key, value));
        return 0;
    }

    int strategy::sched_yield(void) {
        vthread *me = reinterpret_cast<vthread *> \
                      ((*pwarpper::raw_pthread_getspecific)(cur_hcoroutine));
        if ( !me )
            printf("it is empty.\n");
        me->yield_myself();
    }
#endif	/* STRATEGY_H */

