#ifndef VTHREAD_HPP
#define	VTHREAD_HPP

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <unordered_map>
#include <queue>
#include "tbb/concurrent_unordered_map.h"
#include "semaphore.h"
#include "hcontext.h"
#include <linux/futex.h>
#include <sys/time.h>
#include <errno.h>

#include "hglobal.h"
#include <pthread.h>
#include "pwarpper.h"
#include "vthreads.hpp"
#include "htypes.hpp"

/*
inline void * read_rsp() {
    void* ret;
    __asm__ __volatile__ ( "movq %%rsp, %0": :"a"(ret) );
    return ret;
}
*/

using namespace std;
using namespace hthread;

//extern tthreads* ts;
//class vthreads;

namespace hthread
{

extern pthread_key_t cur_hcoroutine;



enum hcoroutine_ret_t : size_t
{
    STATUS_FINISHED,
    STATUS_SUSPENDED,
    STATUS_BLOCKED,
    STATUS_LOCKFAIL,
//    MAX = 0xFFFFFFFFFFFFFFFF

};

enum thread_type_t : unsigned int
{
    TYPE_RAW_THREAD,
    TYPE_VTHREAD
};

class vthread
{
#define STACK_SIZE 655360
#define FPU true
    public:
    typedef vthread this_vthread_type;

    pthread_t vid;
    pthread_attr_t attr;
    void *(*start_routine) (void *);
    void *arg;
    struct tthread *owner;
    void **retval; 
    void *routine_ret; 
    bool to_be_cancelled;
    
    hcoroutine_ret_t cur_status;
    
    unsigned int hold_lockers = 0; // tracker the number of locks this thread hold
    
    hthread_mutex_t join_mutex; // used to replace join_spinlock
    
    int join_waiters_num; // sum of joined vthreads and true threads    
    //hspin_t join_spinlock;
    pthread_spinlock_t join_spinlock;
    // a queue for join operations from vthreads
    vthread_node *join_wait_queue; 
    // for join operations from tthreads
    hthread_sema_t join_sem;
    
    vthread_node thisnode;
    
    unordered_map<pthread_key_t, const void *> vtls;
    
    bool signal_on;
    int uninteruptable;

    vthread (pthread_t _thread, const pthread_attr_t *_attr,
            void *(*_start_routine) (void *), void *_arg) 
    {
        init(_thread, _attr, _start_routine, _arg);
    }
    
    vthread()  {
        pthread_attr_t *_attr = nullptr;
        init(0, _attr, NULL, NULL);
    }

    //replace the constructor to be used in mem pool

    // inline
    void init(pthread_t _thread, const pthread_attr_t *_attr,
                void *(*_start_routine) (void *), void *_arg)
    {
        vid = _thread;
        if ( _attr )
            attr = *_attr;
        start_routine = _start_routine;
        arg = _arg;

        owner = NULL;
        retval = NULL;
        routine_ret = NULL;
        jump_lock = 0;
        cur_status = STATUS_SUSPENDED;
        to_be_cancelled = 0;
        
        hold_lockers = 0;
        // the queue should be always emtpy
        //pthread_spin_init(&join_spin_lock, PTHREAD_PROCESS_PRIVATE); 
        /*while(!join_wait.empty()) {
            printf("some thing bad happened in when initing a vthread\n");
            join_wait.pop();
        }*/
        
        
        join_waiters_num = 0;
        join_wait_queue = NULL;
        //join_spinlock = 1;
        //pthread_spin_init(&join_spinlock, PTHREAD_PROCESS_PRIVATE);
        join_sem.fut = 0;
        /*
        join_mutex = PTHREAD_MUTEX_INITIALIZER;
        join_cond = PTHREAD_COND_INITIALIZER;
        */

        runnable = true;
                
        ctx = make_fcontext(sp + STACK_SIZE, STACK_SIZE, \
		this_vthread_type::manage);
        
        thisnode.vth = this;
        thisnode.next = nullptr;
        
        signal_on = false;
        uninteruptable = 0;

        sp = new char[STACK_SIZE];
        
        vtls.clear();
    }

    ~vthread()
    {
        
    }

    inline intptr_t resume()
    {
        (*pwarpper::raw_pthread_setspecific)(cur_hcoroutine, (void *) this);
        //must add return here, otherwise gcc will optimize the return value to 0
        return safe_jump_fcontext(&owner_ctx, ctx, reinterpret_cast<intptr_t> (this), FPU, &jump_lock);
        //return reinterpret_cast<intptr_t>( this->arg + 32 );
    }
    
    inline static bool is_vthread() {
        this_vthread_type *me = \
                reinterpret_cast<this_vthread_type *> \
                ((*pwarpper::raw_pthread_getspecific)(cur_hcoroutine));
        return me;
    }
    
    inline bool is_on_cur_vthread() {
        return true;
/*
        if ( read_rsp() >= this && \
                read_rsp() < this + sizeof(this_vthread_type) )
            return true;
        return false;
*/
    }
    
    inline static void join(this_vthread_type *pjoin_target, void **_retval)  {
        
        this_vthread_type *me = get_cur_vthread(); 
        //printf ("me: %lx\n", me );
        //printf ("pjoin_target->vid: %d\n", pjoin_target->vid );

        if ( me ) {
            //__sync_fetch_and_add ( &me->hold_lockers, 1) ;
            //pthread_spin_lock(&pjoin_target->join_spinlock);
            while(__sync_lock_test_and_set(&pjoin_target->join_mutex.c, 1)) {;
                me->yield_myself(STATUS_LOCKFAIL);
            }
            //printf ("me: %lx get lock\n", me );
            pjoin_target->retval = _retval;
            if ( pjoin_target->cur_status != STATUS_FINISHED ) {
                pjoin_target->join_waiters_num++;
                me->cur_status = STATUS_BLOCKED;
                vthread_node::enqueue(&pjoin_target->join_wait_queue, &me->thisnode);

                //pthread_spin_unlock(&pjoin_target->join_spinlock);
                //__sync_lock_release (&pjoin_target->join_mutex.c);
                //__sync_fetch_and_sub ( &me->hold_lockers, 1) ;
                printf("%lx enqueued in %lx\n", me, pjoin_target);
                me->yield_myself(reinterpret_cast<size_t>(&pjoin_target->join_mutex)); // the worker will unlock it
            } else {
                //printf("%ls target is finished\n", me);
                //pthread_spin_unlock(&pjoin_target->join_spinlock);
                if (_retval)  *_retval = pjoin_target->routine_ret;
                __sync_lock_release (&pjoin_target->join_mutex.c);
            }
        }
        else {
            while(__sync_lock_test_and_set(&pjoin_target->join_mutex.c, 1)) {;
                for ( int i = 0; i < 1000; i++) {}
            }
            pjoin_target->retval = _retval;
            if ( pjoin_target->cur_status != STATUS_FINISHED ) {
                pjoin_target->join_waiters_num++;
                //pthread_spin_unlock(&pjoin_target->join_spinlock);
                __sync_lock_release (&pjoin_target->join_mutex.c);
                //printf("waiter: waiters: %d\n", pjoin_target->join_waiters_num);
                pjoin_target->join_sem.wait();
            } else {
                //pthread_spin_unlock(&pjoin_target->join_spinlock);
                if (_retval) *_retval = pjoin_target->routine_ret;
                __sync_lock_release (&pjoin_target->join_mutex.c);
            }
        }
    }
    
    inline void wake_up_joined() {
        delete[] sp;
        vthread_node *head;
        int waiters = 0;

        /*
        printf("vid_to_pvth = %lx \n", vid_to_pvth);
        printf("sizeof(vthread) = %d\n", sizeof(vthread));
        printf("%lx start to wakeup\n", this);
*/
        //__sync_fetch_and_add ( &hold_lockers, 1);
        while(__sync_lock_test_and_set(&join_mutex.c, 1)) {
            yield_myself(STATUS_LOCKFAIL);
        }

        if ( retval ) *retval = routine_ret;
        cur_status = STATUS_FINISHED;

        if ( 0 != join_waiters_num ) {
            waiters = join_waiters_num;
            join_waiters_num = 0;
            head = join_wait_queue;
            join_wait_queue = nullptr;
        }
        //pthread_spin_unlock(&join_spinlock);
        __sync_lock_release (&join_mutex.c);
        //__sync_fetch_and_sub ( &hold_lockers, 1) ;

      //  printf("waiters: %d\n", waiters);
        
        if ( !waiters ) return;

        vthread_node *p;
        do {
            p = vthread_node::dequeue(&head);
            if ( !p ) break;
            p->vth->cur_status = STATUS_SUSPENDED;
            vs->vs_queue.enqueue(p->vth);
            waiters--;
        }
        while ( p );
        //cout << waiters << waiters << endl;
        while ( waiters-- ) {
            this->join_sem.post();
        }
    }

    inline static void manage(intptr_t p_hcoroutine)
    {
        this_vthread_type *me = \
            reinterpret_cast<this_vthread_type *> (p_hcoroutine);
        me->routine_ret = (*me->start_routine)(me->arg);  
        me->yield_myself(STATUS_FINISHED);
        printf("should not run to here. The function already ended.");
    }

    inline static this_vthread_type * get_cur_vthread()
    {
        return reinterpret_cast<this_vthread_type *> \
            ((*pwarpper::raw_pthread_getspecific)(cur_hcoroutine));
    }
    
    inline static void yield()
    {
        this_vthread_type *me = \
		reinterpret_cast<this_vthread_type *> \
 ((*pwarpper::raw_pthread_getspecific)(cur_hcoroutine));
        me->yield_myself();
    }

    inline static void try_yield()
    {
        this_vthread_type *me = \
                reinterpret_cast<this_vthread_type *> \
 ((*pwarpper::raw_pthread_getspecific)(cur_hcoroutine));
        if ( me->is_on_cur_vthread() )
            me->try_yield_myself();
    }

    inline void  yield_myself(hcoroutine_ret_t ret = STATUS_SUSPENDED)
    {
        //printf("hcoroutine_ret_t: %d\n", ret);
        // ret = (hcoroutine_ret_t)3;
        safe_jump_fcontext(&ctx, owner_ctx, (intptr_t) ret, FPU, &jump_lock);
    }

    inline void * try_yield_myself(hcoroutine_ret_t ret = STATUS_SUSPENDED)
    {
        return try_jump_fcontext(&ctx, owner_ctx, (intptr_t) ret, FPU, &jump_lock);
    }

    inline bool try_lock_jump()
    {
        //     return ! __sync_lock_test_and_set(&jump_lock, 1);
    }

    inline void lock_jump()
    {
        //   while (__sync_lock_test_and_set(&jump_lock, 1));
    }

    inline void unlock_jump()
    {
        // __sync_lock_release(&jump_lock);
    }

    //char sp[STACK_SIZE];
    char *sp;

    bool runnable;
    fcontext_t ctx;
    fcontext_t owner_ctx; // the fcontext_t of the caller
    unsigned long long jump_lock __attribute((aligned(8)));

    bool operator<(const this_vthread_type &v2) const
    {
        return (unsigned long) vid < (unsigned long) v2.vid;
    }

    bool operator>(const this_vthread_type& v2) const
    {
        return (unsigned long) vid > (unsigned long) v2.vid;
    }

    static bool less_than(const this_vthread_type v1, const this_vthread_type &v2)
    {
        return (unsigned long) v1.vid < (unsigned long) v2.vid;
    }
};
}
#endif	

