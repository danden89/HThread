#ifndef HCOROUTINE_H
#define	HCOROUTINE_H

#include <boost/context/all.hpp>
#include <boost/random.hpp>
#include <pthread.h>
#include "vthreads.hpp"
#include "hcontext.h"
/*
#include <boost/coroutine/all.hpp>
using boost::coroutines::coroutine;



typedef coroutine<hcoroutine_ret_t()> \
    hcoroutine_t, *phcoroutine_t;
typedef coroutine<hcoroutine_ret_t>::push_type \
    hcoroutine_push_t, *phcoroutine_push_t;
typedef coroutine<hcoroutine_ret_t>::pull_type \
    hcoroutine_push_t, *phcoroutine_push_t;
*/

using namespace boost::context;
using namespace hthread;

enum hcoroutine_ret_t : int {
    STATUS_FINISHED,
    STATUS_SUSPENDED
};

enum thread_type_t : int {
    TYPE_RAW_THREAD,
    TYPE_VTHREAD
};

extern pthread_key_t cur_hcoroutine;

template<size_t STACK_SIZE, bool FPU>
class hcoroutine_traits{

public:
   //#define jump_fcontext(x1, x2, x3, x4) safe_jump_fcontext(x1, x2, x3, x4, (&jump_lock))

    hcoroutine_traits<STACK_SIZE, FPU>(void *(*_start_routine) (void *), void *_arg) : \
        start_routine(_start_routine), arg(_arg)
    {
        jump_lock = 0;
        ctx = make_fcontext(sp+STACK_SIZE, STACK_SIZE, \
		hcoroutine_traits<STACK_SIZE, FPU>::manage);
    }

    inline intptr_t resume(){
        //must add return here, otherwise gcc will optimize the return value to 0
        (*pwarpper::raw_pthread_setspecific)(cur_hcoroutine, (void *)this);
        return safe_jump_fcontext(&owner_ctx, ctx, reinterpret_cast<intptr_t>(this), FPU, &jump_lock);
        //return reinterpret_cast<intptr_t>( this->arg + 32 );
    }

    inline static void manage(intptr_t p_hcoroutine){
        hcoroutine_traits<STACK_SIZE, FPU> *me = \
            reinterpret_cast<hcoroutine_traits<STACK_SIZE, FPU> *>(p_hcoroutine);
        (*me->start_routine)(me->arg);
        me->yield_myself(STATUS_FINISHED);
        printf("should not run to here. The function already ended.");
    }
    
    inline static void yield(){
        hcoroutine_traits<STACK_SIZE, FPU> *me = \
		reinterpret_cast<hcoroutine_traits<STACK_SIZE, FPU> *>\
                ( (*pwarpper::raw_pthread_getspecific)(cur_hcoroutine) );
        me->yield_myself(STATUS_SUSPENDED);
    }

    inline static void try_yield(){
        hcoroutine_traits<STACK_SIZE, FPU> *me = \
                reinterpret_cast<hcoroutine_traits<STACK_SIZE, FPU> *>\
                ( (*pwarpper::raw_pthread_getspecific)(cur_hcoroutine) );
        me->try_yield_myself(STATUS_SUSPENDED);
    }
    
    inline void yield_myself(hcoroutine_ret_t ret) {
        //printf("hcoroutine_ret_t: %d\n", ret);
        // ret = (hcoroutine_ret_t)3;
        safe_jump_fcontext(&ctx, owner_ctx, (intptr_t)ret, FPU, &jump_lock);
    }

    inline void try_yield_myself(hcoroutine_ret_t ret) {
        try_jump_fcontext(&ctx, owner_ctx, (intptr_t)ret, FPU, &jump_lock);
    }

    inline bool try_lock_jump() {
   //     return ! __sync_lock_test_and_set(&jump_lock, 1);
    }

    inline void lock_jump() {
     //   while (__sync_lock_test_and_set(&jump_lock, 1));
    }

    inline void unlock_jump() {
       // __sync_lock_release(&jump_lock);
    }

    void *(*start_routine) (void *) = nullptr;
    void *arg = nullptr;
    
    char sp[STACK_SIZE];
    
    fcontext_t ctx;
    fcontext_t owner_ctx; // the fcontext_t of the caller
    vthread *vth;
    unsigned long long jump_lock __attribute((aligned (8))); 
};

typedef hcoroutine_traits<65536, false> hcoroutine;



#endif	/* HTYPES_H */

