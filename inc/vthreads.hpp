#ifndef VTHREADS_HPP
#define	VTHREADS_HPP

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <unordered_map>
#include <map>

//#include "tthreads.hpp"
#include "tbb/concurrent_unordered_map.h"
#include "semaphore.h"
//#include "vthread.hpp"
#include "blockingconcurrentqueue.h"
#include "htypes.hpp"
#include "readerwriterqueue.h"
#include "hlockfreequeue.hpp"


namespace hthread {

    class vthread;
    class tthread;
    class tthreads;
    using namespace std;

    class vthreads {
    public:
        vthreads();
        virtual ~vthreads();
        void add_wait(vthread* pvth);
        void add_vthread(pthread_t *thread,\
            const pthread_attr_t *attr,\
            void *(*start_routine) (void *), void *arg);
        vthread* get_wait();
        vthread* get_and_work(tthread *ptth);
        bool erase_wait(pthread_t pth);
        bool erase_working(pthread_t pth);
        unsigned long wait_size();
        unsigned long working_size();
        void lock();
        void unlock();
        void cond_wait();
        void cond_signal();
        void join_vthread(pthread_t vid, void **retval = NULL);
        void end_vthread(pthread_t vid, void *retval = NULL);
        //void end_vthread(pthread_t vid);
        pthread_t v2t(pthread_t vid);

        static vthreads* getInstance();
    private:
        
        pthread_mutex_t vs_mutex;
        pthread_cond_t vs_cond;
        unordered_map<pthread_t, vthread*>& vs_waiting;
    
    public:
// #define BLOCKING_QUEUE
#ifdef BLOCKING_QUEUE
        moodycamel::BlockingConcurrentQueue<vthread*>& vs_queue;
#endif
#ifdef LOCKFREE_QUEUE
        moodycamel::ConcurrentQueue<vthread*>& vs_queue;
#endif
#ifdef HLOCKFREEQUEUE
        hlockfreequeue<vthread*>& vs_queue;
#endif

        //moodycamel::BlockingReaderWriterQueue<vthread*>& vs_queue;
        //tbb::concurrent_unordered_map<pthread_t, vthread*>& vs_waiting;
    
    public:
        unordered_map<pthread_t, vthread*>& vs_working;
        //tbb::concurrent_unordered_map<pthread_t, vthread*>& vs_working;
        pthread_t vid; //magic number, cannot be initialized by 0
        hspin_t vid_spin_lock;
        
        
        //tbb::concurrent_unordered_map<pthread_t, pthread_cond_t*>& joins;
        unordered_map<pthread_t, pthread_cond_t*>& joins;
    };
    
    extern vthreads *vs;
}
#endif
