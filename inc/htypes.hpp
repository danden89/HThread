#ifndef HTYPES_HPP
#define HTYPES_HPP

#include "hdebug.h"
#include <sys/syscall.h>
#include <linux/futex.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/mman.h>
#include <boost/lockfree/queue.hpp>

namespace hthread
{

    //typedef pthread_spinlock_t hspin_t;
    //inline void hspin_lock(hspin_t *lock) {
    //    pthread_spin_lock(lock);
    //}
    //
    //inline void hspin_unlock(hspin_t *lock) {
    //    pthread_spin_unlock(lock);
    //}

    typedef unsigned int hspin_t;
    const int SPIN_LOCK_READS_BETWEEN_CMPXCHG = 1000;
    inline void hspin_lock(hspin_t *lock)
    {
        __asm__ __volatile__
            (
             "xor %%r8d, %%r8d \r\n"
             "inc %%r8d \r\n"

             "lock; xchg %%r8d, %0 \r\n"
             "test %%r8d, %%r8d \r\n"
             "je spinend%= \r\n"

             "spinloop%=: \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             "pause \r\n"
             //             "wait_loop%=: \r\n"
             //             "dec %1 \r\n"
             //             "cmp $0, %1 \r\n"
             //             "//jne spinloop%= \r\n"

             "lock; xchg %%r8d, %0 \r\n"
             "test %%r8d, %%r8d \r\n"
             "jne spinloop%= \r\n"

             "spinend%=: \r\n"
             :
             : "m"(*lock)
             //  : "m"(*lock), "a"(SPIN_LOCK_READS_BETWEEN_CMPXCHG)
             : "r8"
                 );
    }

    inline bool hspin_trylock(hspin_t *lock)
    {
        register hspin_t old_val;
        register hspin_t operand ;
        operand ^= operand;

        __asm__ __volatile__
            (
             // "xorl %%r8d, %%r8d \r\n"
             "xorl %%eax, %%eax \r\n"
             "lock; xchgl %%eax, (%1) \r\n"
             : "=a"(old_val)
             : "r"(lock), "a"(old_val)
             : "memory", "cc"
            );
        return old_val;
        return 0 == old_val;
    }

    inline void hspin_unlock(hspin_t *lock)
    {
        __sync_synchronize();
        //__asm__ __volatile__ ("" ::: "memory");
        //(*lock)++;
        //(*lock)--;
        (*lock) = 0;
    }


    //typedef unsigned int hspin_t;
    //const int SPIN_LOCK_READS_BETWEEN_CMPXCHG = 1000;
    //inline void hspin_lock(hspin_t *lock)
    //{
    //    __asm__ __volatile__
    //            (
    //             "xor %%r8d, %%r8d \r\n"
    //             "inc %%r8d \r\n"
    //
    //             "lock; xchg %%r8d, %0 \r\n"
    //             "test %%r8d, %%r8d \r\n"
    //             "je spinend%= \r\n"
    //
    //             "spinloop%=: \r\n"
    //             //"wait_loop%=: \r\n"
    //             "dec %1 \r\n"
    //             "cmp $0, %1 \r\n"
    //             "jne spinloop%= \r\n"
    //    
    //             "lock; xchg %%r8d, %0 \r\n"
    //             "test %%r8d, %%r8d \r\n"
    //             "jne spinloop%= \r\n"
    //    
    //             "spinend%=: \r\n"
    //             :
    //             : "m"(*lock), "a"(SPIN_LOCK_READS_BETWEEN_CMPXCHG)
    //             : "r8"
    //             );
    //}
    //
    //inline void hspin_unlock(hspin_t *lock)
    //{
    //    //__sync_synchronize();
    //    __asm__ __volatile__ ("" ::: "memory");
    //    *lock = 0;
    //    
    //}

    template<class T>
        struct typed_node
        {
            volatile T* vth;
            volatile typed_node<T>* next;

            // the operation is actually a stack

            static void enqueue(typed_node<T>** phead, typed_node<T>* pnew_node)
            {
                typed_node<T>* prev_head = *phead;
                *phead = pnew_node;
                pnew_node->next = prev_head;
            }

            static void safe_enqueue(typed_node<T>** phead, typed_node<T>* pnew_node)
            {
                while (true) {
                    typed_node<T>* old_head = *phead;
                    pnew_node->next = old_head;
                    if (__sync_bool_compare_and_swap(phead, old_head, pnew_node)) 
                        break;
                }
            }

            static typed_node<T>* dequeue(typed_node<T>** phead)
            {
                if (!*phead) return nullptr;
                typed_node<T>* cur_head = *phead;
                *phead = cur_head->next;
                cur_head->next = nullptr;
                return cur_head;
            }

            static typed_node<T>* safe_dequeue(typed_node<T>** phead)
            {
                while (true) {
                    if (!*phead) return nullptr;
                    typed_node<T>* cur_head = *phead;
                    typed_node<T>* new_head = cur_head->next;
                    if (__sync_bool_compare_and_swap(phead, cur_head, new_head)) {
                        cur_head->next = nullptr;
                        return cur_head;
                    }
                }
            }

            typed_node<T>(T* _vth = nullptr) : vth(_vth), next(nullptr)
            {
            }

        };

    class vthread;
    typedef typed_node<vthread> vthread_node;

    typedef struct hthread_bin_sema_t
    {
        unsigned int fut; //number of availabe resources, default 1

        inline void wait()
        {
            int ret;
            while (true)
            {
                if (__sync_bool_compare_and_swap(&fut, 1, 0)) break;
                ret = syscall(SYS_futex, &fut, FUTEX_WAIT, 0, NULL, NULL, 0);
                if ( ret == -1 && errno != EAGAIN ) exit(EXIT_FAILURE);
            }
        }

        inline void post()
        {
            int ret;
            if (__sync_bool_compare_and_swap(&fut, 0, 1))
            {
                ret = syscall(SYS_futex, &fut, FUTEX_WAKE, 1, NULL, NULL, 0);
                if (ret == -1) exit(EXIT_FAILURE);
            }
        }

        hthread_bin_sema_t(unsigned int init_val = 1) : fut(init_val) {}

    } hthread_bin_sema_t;

    typedef hthread_bin_sema_t hthread_sema_t;

    typedef struct hthread_mutex_t
    {
        // number of users could be 0 or 1, 
        // including both virtual threads and real threads
        unsigned int volatile c; 
        hthread_bin_sema_t bin_sema; //used for kernel threads
        hspin_t spinlock; // the lock for this struct
        unsigned int tthread_num; // the number of real tthreads waiting for this
        vthread_node *waiting_vthreads;

        inline void acquire_acess()
        {
            hspin_lock(&spinlock);
        }

        inline void release_access()
        {
            hspin_unlock(&spinlock);
        }

        //this constructor would not be called in the pthread mode
        hthread_mutex_t() : c(0), spinlock(0), \
                            tthread_num(0),
                            bin_sema(hthread_bin_sema_t(0)), waiting_vthreads(nullptr)
        {
        }

    } hthread_mutex_t;

    typedef boost::lockfree::queue <vthread*, \
        boost::lockfree::fixed_sized<false> > hcond_lockfree_queue;

    typedef struct hthread_cond_t
    {
        // number of users could be 0 or 1, 
        // including both virtual threads and real threads

        volatile int waiter_num;
        int signals;
        int waker_blocked; // val 1: single wakeup 2: broadcast

        //vthread_node *waiting_vthreads;

        hcond_lockfree_queue *blocked_vths;
        pthread_mutex_t* mutex;
        //hthread_mutex_t* hmutex;
        hthread_bin_sema_t bin_sema; //used for kernel threads
        hspin_t spinlock; // the lock for this struct
        unsigned int tthread_num; // the number of real tthreads waiting for this

        inline void acquire_acess()
        {
            hspin_lock(&spinlock);
        }

        inline void release_access()
        {
            hspin_unlock(&spinlock);
        }

        //this constructor would not be called in the pthread mode
        hthread_cond_t() : spinlock(0), \
                           tthread_num(0),
                           bin_sema(hthread_bin_sema_t(0))
                           //, waiting_vthreads(nullptr)
        {
        }

    } hthread_cond_t;

    typedef struct hthread_sem_t
    {
        // number of users could be 0 or 1, 
        // including both virtual threads and real threads
        int c; 
        //unsigned int waiter_num; 
        //vthread_node *waiting_vthreads;

        hthread_sem_t() : c(0)
        {
        }

    } hthread_sem_t;


    typedef struct hthread_rwlock_t{
        volatile int r;
        //volatile int w;
        volatile vthread* w;

        hthread_rwlock_t() : r(0), w(0) {  }
    } hthread_rwlock_t;

    typedef struct hthread_barrier_t{
        volatile unsigned int waiters;
        volatile unsigned int cur_waiters;
        volatile unsigned int cur_round;
//        vthread_node *waiting_vthreads;
        boost::lockfree::queue <vthread*, \
            boost::lockfree::fixed_sized<true> > *blocked_vths[2];
    } hthread_barrier_t;
    /*
       typedef struct hthread_cond_t
       {
    // number of users, 
    // including both virtual threads and real threads
    unsigned int c; 
    hthread_cond_t bin_sema; //used for kernel threads
    hspin_t spinlock; // the lock for this struct
    vthread_node *waiting_vthreads;

    inline void acquire_acess()
    {
    hspin_lock(&spinlock);
    }

    inline void release_access()
    {
    hspin_unlock(&spinlock);
    }

    hthread_cond_t() : c(0), spinlock(0), \
    bin_sema(hthread_bin_sema_t(0)), waiting_vthreads(nullptr)
    {
    }

    } hthread_cond_t;

*/
}
#endif
