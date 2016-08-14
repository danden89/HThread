#include <iostream>
#include "pwarpper.h"
#include "vthreads.hpp"
#include "tthreads.hpp"
#include "hglobal.h"
#include "hdebug.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <signal.h>

using namespace std;
using namespace hthread;

void init_task_num() {
    int task_num_fd;
    N_THREADS = sysconf( _SC_NPROCESSORS_ONLN );
//    cout << TASK_NUM_SHM_STR << endl;
    //shm_unlink(TASK_NUM_SHM_STR);
    task_num_fd = shm_open(TASK_NUM_SHM_STR, O_CREAT| O_RDWR, 0777);
    //task_num_fd = shm_open(str, O_CREAT| O_RDONLY, 0666);
    if (task_num_fd == -1)
    {
        shm_unlink(TASK_NUM_SHM_STR);
        printf("ERROR open shared memory.\n");
        exit(-1);
    }

    ftruncate(task_num_fd, sizeof(int));
    ptask_number = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, \
                        MAP_SHARED, task_num_fd, 0);
    printf("/*SUCCESS map shared memory.%lx*/\n", ptask_number);
    __sync_fetch_and_add(ptask_number, 1);
    printf("/*N_THREADS: %d\n*/\n", N_THREADS);
//    printf("SUCCESS ADD.\n");
    
//    signal(TASK_NUM_UPDATE_SIGNAL, task_num_update_handler);    
    
//    killpg(hthread_group_id, TASK_NUM_UPDATE_SIGNAL);
    N_THREADS_ALLOWED = N_THREADS / *ptask_number;
    if (N_THREADS % *ptask_number) N_THREADS_ALLOWED++;
    printf("/*task number: %d\n*/\n", *ptask_number);

}

extern "C" __attribute__((constructor)) void hthread_init(void) {
    debug( "hthread so loaded\n");
//    init_task_num();
    N_THREADS = sysconf( _SC_NPROCESSORS_ONLN );
    pr = pwarpper::getInstance();
    vs = vthreads::getInstance();
    ts = tthreads::getInstance(N_THREADS);
    
    
//    setpgid(hthread_group_id, hthread_group_id+1);
//    cout << "group id: " << getpgid(0) << endl;
//    init_task_num();
    
    debug("hthread so loaded\n");
    debug("ts size : %d\n", ts->size());
}

extern "C" __attribute__((destructor))  void hthread_fini(void) {
    __sync_fetch_and_add(ptask_number, -1);
//*ptask_number = 0;
//    shm_unlink(TASK_NUM_SHM_STR);
    
//    killpg(hthread_group_id, TASK_NUM_UPDATE_SIGNAL);
    printf("/*task number: %d\n*/", *ptask_number);
    cout <<  "/*hthread so unloaded\n*/\n" << endl;

//    delete vs;
//    delete ts;
//    delete pr;
}
