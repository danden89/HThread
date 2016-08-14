#ifndef PWARPPER_H
#define	PWARPPER_H


#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <unordered_set>
#include <iostream>
#include <string>
#include <dlfcn.h>
#include <unordered_map>
#include <string>

namespace hthread {
    class pwarpper {
    public:
        pwarpper();
        pwarpper(const pwarpper& orig);
        virtual ~pwarpper();

    public:
        void init_all();
        void init_func();
        void * get_routine(std::string sym);
        void test();

    public:
        static int (*raw_pthread_create)(pthread_t *thread, const pthread_attr_t *attr,
                void *(*start_routine) (void *), void *arg);
        
        static int (*raw_pthread_cancel)(pthread_t thread);
        
        static int (*raw_pthread_join)(pthread_t thread, void **retval);
        
        static void (*raw_pthread_exit)(void *retval)  __attribute__ ((noreturn));

        static void (*raw_pthread_cleanup_push)(void (*routine)(void *), void *arg);
        
        static pthread_t (*raw_pthread_self)(void);
        
        static pthread_t (*raw_pthread_barrier_wait)(pthread_barrier_t *barrier);
        
        static pwarpper* getInstance();
        
        static int (*raw_pthread_cond_wait)(pthread_cond_t *cond, pthread_mutex_t *mutex);
        
        static int (*raw_pthread_mutex_lock)(pthread_mutex_t *mutex);
        
        static int (*raw_pthread_mutex_unlock)(pthread_mutex_t *mutex);
        
        static int (*raw_pthread_mutex_trylock)(pthread_mutex_t *mutex);
        
//        static int (*raw_pthread_cond_timedwait)(pthread_cond_t *restrict cond,
//                pthread_mutex_t *restrict mutex,
//                const struct timespec *restrict abstime);
        
        static int (*raw_pthread_cond_timedwait)(pthread_cond_t * cond,
                pthread_mutex_t * mutex,
                const struct timespec *abstime);
        
        static int (*raw_pthread_cond_signal)(pthread_cond_t *cond);
        static int (*raw_pthread_cond_broadcast)(pthread_cond_t *cond);
        
        static int (*raw_sem_wait)(sem_t *sem);
        static int (*raw_sem_trywait)(sem_t *sem);
        static int (*raw_sem_post)(sem_t *);
        
        static void *(*raw_pthread_getspecific)(pthread_key_t key);
        static int (*raw_pthread_setspecific)(pthread_key_t key, const void *value); 

        static int (*raw_pthread_yield)(void);
        static int (*raw_sched_yield)(void);
        
    private:
        static std::string  pthread_so_name;
        void *handle; //for handling pthread library
    };
}
#endif	/* PWARPPER_H */

