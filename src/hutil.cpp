#include "hutil.h"
#include "hdebug.h"
#include <sys/sysinfo.h>
#include "hglobal.h"

int get_core_id(pthread_t pid) {
    int core_num = get_nprocs();
    return (pid - 3282688) / 8392704 % core_num;
}

size_t get_core_id2() {
    //return 3;
    return (size_t)pthread_getspecific(hthread::thread_schd_key);
}

void assign_affinity(pthread_t pid){    
   // int core_id = get_core_id(pid);
    int core_id = get_core_id2();
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);
    printf("pid: %d\n", pid);
    printf("core_num: %d\n", get_nprocs());
    printf("core_id: %d\n", core_id);
    int ret = pthread_setaffinity_np(pid, sizeof(cpu_set_t), &cpuset);
    if ( ret != 0 ) {
        perror("failure when set cpu affinity in thread\n");
        exit(EXIT_FAILURE);
    }
}

//int futex(int *uaddr, int futex_op, int val,
//                 const struct timespec *timeout, int *uaddr2, int val3)
//{
//    return syscall(SYS_futex, uaddr, futex_op, val,
//                   timeout, uaddr, val3);
//}


//int h_add_and_fetch(int * pop1, int op2)
//{
//    register int ret;
//    __asm__ __volatile__("lock; xaddl %%eax, %2;"
//                         : "=a" (ret)
//                         : "a" (op2), "m" (*pop1)
//                         : "memory");
//    return op2 + ret;
//}

//void hsem_wait(int* p)
//{
//    register int new_val;
//    register int ret;
//    new_val = h_add_and_fetch(p, -1);
//    if ( new_val >= 0 )
//        return;
//    while ( true ) {
//        ret = futex(p, FUTEX_WAIT, new_val, NULL, NULL, 0);
//        if ( likely (ret == 0 ) )
//            return;        
//        else if ( likely (errno == EAGAIN ) ) {
//            new_val = *p;
//        }
//    }
//}


//void hsem_post(int* p) {
//    register int new_val;
//    register int ret;
//    new_val = h_add_and_fetch(p, 1);
//    if ( new_val > 1)
//        return;
//    ret = futex(p, FUTEX_WAKE, 1, NULL, NULL, 0);
//    if ( unlikely (ret == -1) )
//    {
//        perror("failure when set cpu affinity in thread\n");
//        exit(EXIT_FAILURE);
//    }
//}