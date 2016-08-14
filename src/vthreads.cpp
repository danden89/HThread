#include "vthreads.hpp"
#include "hglobal.h"
#include "hdebug.h"
#include <time.h>
#include "vthread.hpp"
#include "tthreads.hpp"
#include "htypes.hpp"

#define RESERVE_NUM 512
using namespace hthread;

vthreads::vthreads() : vs_waiting(*new unordered_map<pthread_t, vthread*>()), \
        vs_working(*new unordered_map<pthread_t, vthread*>()), \
        joins(*new unordered_map<pthread_t, pthread_cond_t*>()), 
#ifdef BLOCKING_QUEUE
        vs_queue(*new moodycamel::BlockingConcurrentQueue<vthread*>() ), 
#endif
#ifdef LOCKFREE_QUEUE
        vs_queue(*new moodycamel::ConcurrentQueue<vthread*>() ),
#endif
#ifdef HLOCKFREEQUEUE
        vs_queue(*new hlockfreequeue<vthread*>(1<<14) ), 
#endif
        //vs_queue(*new moodycamel::BlockingReaderWriterQueue<vthread*>()), 
        vid_spin_lock(0) 
{
    vs_mutex = PTHREAD_MUTEX_INITIALIZER;
    //vs_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
    vs_cond = PTHREAD_COND_INITIALIZER;
    vs_waiting.reserve(RESERVE_NUM);
    //vs_working.reserve(RESERVE_NUM);
    joins.reserve(RESERVE_NUM);
    vid = 15;
}

vthread* vthreads::get_wait() {
    vthread* ret = nullptr;
#ifdef BLOCKING_QUEUE
    vs_queue.wait_dequeue(ret);
#endif
#ifdef LOCKFREE_QUEUE
    do {
        vs_queue.try_dequeue(ret);
    }
    while (!ret);
#endif
#ifdef HLOCKFREEQUEUE
    //printf( "vs_queue: %lx\n", &vs_queue);
    vs_queue.dequeue(ret);
#endif
    //printf( "ret: %lx\n", ret);
    return ret;
};

void vthreads::add_vthread(pthread_t *thread, const pthread_attr_t* attr,
                           void *(*start_routine) (void *), void *arg)
{
    unsigned int cur_vid;
/*
    hspin_lock(&vid_spin_lock);
        ++vid;
        if ( vid >= MAX_VTHREADS ) {
            vid = 1024;
        }
        cur_vid = vid;
    hspin_unlock(&vid_spin_lock);
*/
    debug("cur_vid: %d\n", vid);
    cur_vid = __sync_fetch_and_add(&vid, 1);
    cur_vid %= MAX_VTHREADS;
    debug("cur_vid: %d\n", cur_vid);
    *thread = cur_vid;
    vthread *pvth = vid_to_pvth + cur_vid;
    //*pvth = vthread(cur_vid, *attr, start_routine, arg);
    pvth->init(cur_vid, attr, start_routine, arg);
    vs_queue.enqueue(pvth);
}

vthread* vthreads::get_and_work(tthread *ptth)
{
    vthread* pvth = get_wait();
    pvth->owner = ptth;
    return pvth;
}

pthread_t vthreads::v2t(pthread_t vid)
{
    vthread* pvth = vs_working[vid];
    return pvth->owner->pid;
}

bool vthreads::erase_wait(pthread_t pth)
{
    if (vs_waiting.find(pth) != vs_waiting.end() )
    {
        delete vs_waiting[pth];
        vs_waiting.erase(pth);
        return true;
    }
    return false;
}

bool vthreads::erase_working(pthread_t pth)
{
    if (vs_working.find(pth) != vs_working.end() )
    {
        delete vs_working[pth];
        vs_working.erase(pth);
        return true;
    }
    return false;
}

unsigned long vthreads::wait_size()
{
    size_t s = vs_waiting.size();
    return s;
}

unsigned long vthreads::working_size()
{
    size_t s = vs_working.size();
    return s;
}

vthreads::~vthreads()
{
    debug("~vthreads\n");
    //delete &vs_waiting;
    //delete &vs_working;
}

void vthreads::lock()
{
    (*pwarpper::raw_pthread_mutex_lock)(&vs_mutex);
}

void vthreads::unlock()
{
    (*pwarpper::raw_pthread_mutex_unlock)(&vs_mutex);
}

void vthreads::cond_wait()
{
    (* pwarpper::raw_pthread_cond_wait)(&vs_cond, &vs_mutex);
}

void vthreads::cond_signal()
{
    //(* pwarpper::raw_pthread_cond_signal)(&vs_cond);
    pthread_cond_signal(&vs_cond);
}

vthreads* vthreads::getInstance()
{
    static vthreads* pvth = new vthreads();
    return pvth;
}

void vthreads::join_vthread(pthread_t vid, void **retval)
{
    bool found = false;
    //if (likely(!(bool)retval)) return;
    if ( retval )
        *retval = nullptr;
    lock();
    if (vs_waiting.find(vid) != vs_waiting.end())
    {
        vthread* pvth = vs_waiting[vid];
        pvth->retval = retval;
        found = true;
    }
    else if (vs_working.find(vid) != vs_working.end())
    {
        //vthread& vth = vs_working[vid];
        vthread* pvth = vs_working[vid];
        pvth->retval = retval;
        found = true;
    }
    pthread_cond_t* pcond = nullptr;
    if (found)
    {
        pthread_cond_t* pcond = (pthread_cond_t*) malloc(sizeof (pthread_cond_t));
        if (hthread_unlikely(!pcond))
        {
            perror("malloc memory error in join_vthread\n");
            exit(1);
        }        
        *pcond = PTHREAD_COND_INITIALIZER;
        joins.insert(make_pair(vid, pcond));
        (* pwarpper::raw_pthread_cond_wait)(pcond, &vs_mutex);
    }
    unlock();
//    if (pcond)
//    {
//        int ret = pthread_cond_destroy(pcond);
//        if (!ret)
//        {
//            perror("destroy error\n"); exit(1);
//        }
//        free(pcond);
//    }
    
}

//void vthreads::end_vthread(pthread_t vid) {
//    pthread_cond_signal(&joins[vid]);
//    auto pvth = vs_working.find(vid);    
//    joins.erase(vid);
//    vs_working.erase(vid);
//}

void vthreads::end_vthread(pthread_t vid, void *retval)
{
    if ( joins.find(vid) != joins.end()  )
    {
        vthread* pvth = vs_working[vid];
        if ( pvth->retval )
            *(pvth->retval) = retval;
        //vth.retval = NULL;
        pthread_cond_t* pcond = joins[vid];
        pthread_cond_signal(pcond);
        joins.erase(vid);
    }
    erase_working(vid);
}

