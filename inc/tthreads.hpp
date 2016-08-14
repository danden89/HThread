#ifndef TTHREADS_HPP
#define	TTHREADS_HPP

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <map>
#include <unordered_map>
#include <atomic>
#include <semaphore.h>
#include "pwarpper.h"
#include "vthreads.hpp"
#include "vthread.hpp"
//#include "vthreads.hpp"
#include "hglobal.h"




using namespace std;
 
namespace hthread
{
    #ifdef USE_FUTEX
    struct hschd_lock {
        int num;
        int padding[15];
        
        hschd_lock() : num(1){
        }
    };
    #endif

    #ifdef USE_MUTEX
    struct hschd_lock {
        pthread_mutex_t num;
        char padding[14];
        
        hschd_lock(){
            num = PTHREAD_MUTEX_INITIALIZER;
        }
    };
    #endif
    
    class tthread {
    public:
        pthread_t pid;
        int core_id; // the n-th true thread in the system
        pthread_attr_t attr;
        vthread *task;        
        bool active;
        bool after_p; 

        tthread(pthread_t _thread, pthread_attr_t _attr, int _core_id = -1)
        : pid(_thread), attr(_attr), task(NULL), core_id(_core_id) {
        }

        bool operator<(const tthread &t2) const {
            return (unsigned long) pid < (unsigned long) t2.pid;
        }

        bool operator>(const tthread& t2) const {
            return (unsigned long) pid > (unsigned long) t2.pid;
        }

        static bool less_than(const tthread t1, const tthread &t2) {
            return (unsigned long) t1.pid < (unsigned long) t2.pid;
        }
    };

    class tthreads {
    public:
        #if defined(USE_FUTEX) || defined(USE_MUTEX)
        static hschd_lock schd_locks[8]  __attribute__((aligned(64)));
        #endif
        #ifdef USE_HSEM
        static int target_num  __attribute__((aligned(64)));
        #endif
        
    public:
        tthreads(int n_threads);
        virtual ~tthreads();
        void add(tthread* ptth); // add it to the map
        void add_pthread(int n_threads);
        struct tthread* get();
        struct tthread* find(pthread_t pth);
        void erase(pthread_t pth);
        void cancel(pthread_t pth);
        unsigned long size();
        void lock();
        void unlock();
        void cond_wait();
        void cond_signal();
        pthread_t t2v(pthread_t tid);
        
        //int barrier_wait(pthread_barrier_t *barrier);
        pthread_t get_tid();
        
        static tthreads* getInstance(int n_threads);
        
        static atomic<int> active_threads_count;
        
        inline static void p() __attribute__((always_inline));
        inline static void v() __attribute__((always_inline));

    private:
        static sem_t num_threads;
        pthread_mutex_t ts_mutex;
        pthread_cond_t ts_cond;
        //unordered_map<pthread_t, tthread*>& real_threads;
        int core_id;
    public:
        static int core_num;
        vector<tthread*> &real_threads;
    };
    
    extern tthreads *ts;
    
    void tthreads::p() {
//        int active_threads = atomic_load(&tthreads::active_threads_count);
    if ( N_THREADS >= core_num)
        return;
#ifdef USE_SEMAPHORE
    (*pwarpper::raw_sem_wait)(&num_threads);
#endif
#ifdef USE_FUTEX
    pthread_t myid = (* pwarpper::raw_pthread_self)();
    int core_id = get_core_id(myid);
    while (1)
    {

        if (__sync_bool_compare_and_swap(&(schd_locks[core_id].num), 1, 0))
            break;
        int ret = futex(&(schd_locks[core_id].num), \
                        FUTEX_WAIT|FUTEX_PRIVATE_FLAG, 0, NULL, NULL, 0);
        if (hthread_unlikely(ret == -1 && errno != EAGAIN)) {
            perror("failure when set cpu affinity in thread \n");
            exit(EXIT_FAILURE);
        }
    }
#endif
#ifdef USE_MUTEX
    //pthread_t myid = (* pwarpper::raw_pthread_self)();
    //int core_id = get_core_id(myid);
    int core_id = get_core_id2();
    (*pwarpper::raw_pthread_mutex_lock) (&(schd_locks[core_id].num));
#endif
    
#ifdef USE_HSEM
    hsem_wait(&target_num);
#endif
}

void tthreads::v() {
#ifdef USE_SEMAPHORE
    (*pwarpper::raw_sem_post)(&num_threads);
#endif
#ifdef USE_FUTEX
    //pthread_t myid = (* pwarpper::raw_pthread_self)();
    //int core_id = get_core_id(myid);
    int core_id = get_core_id2();
    if (__sync_bool_compare_and_swap(&(schd_locks[core_id].num), 0, 1))
    {

        int ret = futex(&(schd_locks[core_id].num), FUTEX_WAKE|FUTEX_PRIVATE_FLAG, 1, NULL, NULL, 0);
        if (hthread_unlikely(ret == -1)) {
            perror("failure when set cpu affinity in thread \n");
            exit(EXIT_FAILURE);
        }
    }
#endif
#ifdef USE_MUTEX
    //pthread_t myid = (* pwarpper::raw_pthread_self)();
    //int core_id = get_core_id(myid);
    int core_id = get_core_id2();
    (*pwarpper::raw_pthread_mutex_unlock) (&(schd_locks[core_id].num));
#endif
    
#ifdef USE_HSEM
    hsem_post(&target_num);
#endif
}
    
//    struct para{
//        tthreads * ts;
//        tthread *tt;
//        
//        para(tthreads * _ts, tthread *_tt):\
//            ts(_ts), tt(_tt){
//        }
//    };
}


#endif
