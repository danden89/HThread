#include "strategy.h"
#include "semaphore.h"
#include <stdlib.h> 
//#include "libgomp.h"
//
extern "C" int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
        void *(*start_routine) (void *), void *arg){
    //printf("called here");
    return strategy::pthread_create(thread, attr, start_routine, arg);
}

extern "C" int pthread_cancel(pthread_t thread) {
    return strategy::pthread_cancel(thread);
}

extern "C" int pthread_join(pthread_t thread, void **retval) {
        return strategy::pthread_join(thread, retval);
}

extern "C" void pthread_exit(void *retval) {
        strategy::pthread_exit(retval);
}


extern "C" pthread_t pthread_self(void) {
    //printf("called here");
    return strategy::pthread_self();
}

extern "C" int pthread_barrier_wait(pthread_barrier_t *barrier){
    return strategy::pthread_barrier_wait(barrier);
}

extern "C" int pthread_barrier_init(pthread_barrier_t * barrier,
        const pthread_barrierattr_t * attr, unsigned count) {
    return strategy::pthread_barrier_init(barrier, attr, count);
}

extern "C" int pthread_barrier_destroy(pthread_barrier_t *barrier) {
    return strategy::pthread_barrier_destroy(barrier);
}

extern "C" int pthread_cond_init(pthread_cond_t *cond,
        const pthread_condattr_t *attr){
    hthread_cond_t *hc = (hthread_cond_t *)cond;
    hc->waiter_num = 0;
    hc->blocked_vths = nullptr;
    hc->mutex = nullptr;
    return 0;
}

extern "C" int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex){
    return strategy::pthread_cond_wait(cond, mutex);
}

extern "C" int pthread_cond_signal(pthread_cond_t *cond){
    return strategy::pthread_cond_signal(cond);
}

extern "C" int pthread_cond_broadcast(pthread_cond_t *cond){
    return strategy::pthread_cond_broadcast(cond);
}

extern "C" int pthread_cond_destroy(pthread_cond_t *cond) {
    //memcpy();
    return 0;
}

extern "C"  int pthread_mutex_lock(pthread_mutex_t *mutex){
    return strategy::pthread_mutex_lock(mutex);
}

extern "C"  int pthread_mutex_unlock(pthread_mutex_t *mutex){
    return strategy::pthread_mutex_unlock(mutex);
}

extern "C"  int pthread_mutex_trylock(pthread_mutex_t *mutex){
    return strategy::pthread_mutex_trylock(mutex);
}

extern "C"  int pthread_mutex_destroy(pthread_mutex_t *mutex){
    return 0;
}

extern "C"  int pthread_setcancelstate(int state, int *oldstate) {
    return 0;
}

extern "C" void *pthread_getspecific(pthread_key_t key) {
    //printf("unclear call pthread_getspecific\n");
    return strategy::pthread_getspecific(key);
    return 0;
}

extern "C" int pthread_setspecific(pthread_key_t key, const void *value) {
    //printf("unclear call pthread_setspecific\n");
    strategy::pthread_setspecific(key, value);
    return 0;
}
//extern "C" int pthread_cond_timedwait(pthread_cond_t *cond,
//        pthread_mutex_t *mutex, const struct timespec *abstime) {
//    return strategy::pthread_cond_timedwait(cond, mutex, abstime);
//}

extern "C"  int sem_wait(sem_t *sem){
    return strategy::sem_wait(sem);
}

extern "C"  int sem_trywait(sem_t *sem){
    return strategy::sem_trywait(sem);
}

extern "C"  int sem_post(sem_t *sem){
    return strategy::sem_post(sem);
}

extern "C" int sched_yield(void) {
    return strategy::sched_yield();
}

extern "C" int pthread_yield(void) {
    return strategy::sched_yield();
}

extern "C" int set_iteruptable(int uninteruptable) {
    cout << "i will always run" << endl;
    vthread *me = reinterpret_cast<vthread *> \
                  ((*pwarpper::raw_pthread_getspecific)(cur_hcoroutine));
    me->uninteruptable = uninteruptable;
    return 1;
}


extern "C" int pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock) {
    printf("pthread_rwlock_tryrdlock\n");
    hthread_rwlock_t *hrw = (hthread_rwlock_t*)rwlock;
    if (hrw->w) return EBUSY;
    __sync_fetch_and_add(&hrw->r, 1);
    if (!hrw->w) return 0;
    __sync_fetch_and_add(&hrw->r, -1);
    return EBUSY;
}

extern "C" int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock) {
    //printf("pthread_rwlock_rdlock\n");
    vthread *me = reinterpret_cast<vthread *> \
                  ((*pwarpper::raw_pthread_getspecific)(cur_hcoroutine));
    hthread_rwlock_t *hrw = (hthread_rwlock_t*)rwlock;
    while (true) {
        while (hrw->w) me->yield_myself();
        __sync_fetch_and_add(&hrw->r, 1);
        if (!hrw->w) return 0;
    //    std::cout << "confilicted" << std::endl;
        __sync_fetch_and_add(&hrw->r, -1);
    }
}

extern "C" int pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock) {
    printf("pthread_rwlock_trylock\n");
    vthread *me = reinterpret_cast<vthread *> \
                  ((*pwarpper::raw_pthread_getspecific)(cur_hcoroutine));
    hthread_rwlock_t *hrw = (hthread_rwlock_t*)rwlock;
    if (hrw->r || hrw->w || !__sync_bool_compare_and_swap(&hrw->w, nullptr, me)) return EBUSY;
    if (!hrw->r) return 0;
    __sync_lock_release(&hrw->w);
    return EBUSY;
}

extern "C" int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock) {
    //printf("pthread_rwlock_wrlock\n");
    vthread *me = reinterpret_cast<vthread *> \
                  ((*pwarpper::raw_pthread_getspecific)(cur_hcoroutine));
    hthread_rwlock_t *hrw = (hthread_rwlock_t*)rwlock;
    while (true) {
        while (hrw->r || hrw->w || !__sync_bool_compare_and_swap(&hrw->w, nullptr, me)) 
            me->yield_myself();
        if (!hrw->r) return 0;
     //   std::cout << "confilicted" << std::endl;
        __sync_lock_release(&hrw->w);
    }
}

extern "C" int pthread_rwlock_unlock(pthread_rwlock_t *rwlock) {
    //printf("pthread_rwlock_unlock\n");
    vthread *me = reinterpret_cast<vthread *> \
                  ((*pwarpper::raw_pthread_getspecific)(cur_hcoroutine));
    hthread_rwlock_t *hrw = (hthread_rwlock_t*)rwlock;
    if (hrw->w == me)
        __sync_lock_release(&hrw->w);
    else
        __sync_fetch_and_add(&hrw->r, -1);
}

extern "C" int pthread_rwlock_init(pthread_rwlock_t *rwlock,
        const pthread_rwlockattr_t *attr) {
    hthread_rwlock_t* hrw = (hthread_rwlock_t *)rwlock;
    hrw->r = 0;
    hrw->w = nullptr;
}

