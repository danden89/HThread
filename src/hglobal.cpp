#include "hglobal.h"
#include "vthread.hpp"



namespace hthread
{
class vthread;
class pwarpper;
class tthreads;
class vthreads;

vthread vid_to_pvth[MAX_VTHREADS];
pwarpper *pr;
tthreads *ts;
vthreads *vs;
//const int N_THREADS;
int N_THREADS;
int volatile N_THREADS_ALLOWED;
int* ptask_number;

pthread_key_t thread_schd_key = 0;
pthread_key_t cur_hcoroutine = 0;

concurrent_set<vthread*> signals_set;

const pid_t hthread_group_id = 3726482682;

}
