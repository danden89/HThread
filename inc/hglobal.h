#ifndef HGLOBAL_H
#define	HGLOBAL_H

#include <pthread.h>
#include "concurrent_set.hpp"

namespace hthread {
class vthread;
class pwarpper;
class tthreads;
class vthreads;

// the maximum num of vthreads allowable 
//#define MAX_VTHREADS 8192 
#define MAX_VTHREADS 660
// possible false sharing here, could be improved in the future
extern vthread vid_to_pvth[MAX_VTHREADS];
extern pwarpper *pr;
extern tthreads *ts;
extern vthreads *vs;
extern pthread_key_t thread_schd_key; 
extern pthread_key_t cur_hcoroutine; 

//#define N_THREADS (sysconf( _SC_NPROCESSORS_ONLN )/2)
//#define N_THREADS (10)
//extern const int N_THREADS;
extern int N_THREADS;
extern int volatile N_THREADS_ALLOWED;
extern int* ptask_number;

extern pthread_key_t cur_hcoroutine; // store the current executed hcoroutine;

extern concurrent_set<vthread*> signals_set;

extern const pid_t hthread_group_id;

#define TASK_NUM_UPDATE_SIGNAL (SIGRTMIN+5)
#define TASK_NUM_SHM_STR "hthread_ie929s9f89290"

}
#endif
