#ifndef MGR_THREAD_H
#define	MGR_THREAD_H
#include <pthread.h>
void* mgr_routine(void *p);
void* worker_routine(void *p);
void hworker_clearnup_routine2(void *p);
#endif	/* MGR_THREAD_H */

