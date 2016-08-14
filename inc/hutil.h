#ifndef HUTIL_H
#define	HUTIL_H
#include<string>
#include <sys/syscall.h>
#include <sys/sysinfo.h>
#include <linux/futex.h>
#include <sys/wait.h>
#include <sys/sysinfo.h>
#include "hglobal.h"
#include "hdebug.h"
#include "unistd.h"
using namespace std;

template<typename T>
string to_str(T t1)
{
    size_t len = sizeof (t1);
    return string((char*) &t1, 0, len);
}

int get_core_id(pthread_t pid);
size_t get_core_id2();
void assign_affinity(pthread_t pid);
void init_schd_lock();

inline int futex(int *uaddr, int futex_op, int val,
                 const struct timespec *timeout, int *uaddr2, int val3)
__attribute__((always_inline));

inline int futex(int *uaddr, int futex_op, int val,
                 const struct timespec *timeout, int *uaddr2, int val3)
{
    return syscall(SYS_futex, uaddr, futex_op, val,
                   timeout, uaddr, val3);
}

inline int h_add_and_fetch(int * pop1, int op2) __attribute__((always_inline));

int h_add_and_fetch(int * pop1, int op2)
{
    __asm__ __volatile__("movl %%eax, %%ecx;"
                         "lock; xaddl %%eax, %2;"
                         "addl %%ecx, %%eax"
                         : "=a" (op2)
                         : "a" (op2), "m" (*pop1)
                         : "ecx", "memory");
    return op2;
}

inline int h_fetch_and_add(int * pop1, int op2) __attribute__((always_inline));

int h_fetch_and_add(int * pop1, int op2)
{
    __asm__ __volatile__("lock; xaddl %%eax, %2;"
                         : "=a" (op2)
                         : "a" (op2), "m" (*pop1)
                         : "memory");
    return op2;
}

inline void hsem_wait(int* p) __attribute__((always_inline));

void hsem_wait(int* p)
{
    register int new_val;
    register int ret;
    new_val = h_add_and_fetch(p, -1);
    if (new_val < 0)
    {
        while (true)
        {
            ret = futex(p, FUTEX_WAIT, new_val, NULL, NULL, 0);

            if (hthread_likely(ret == 0)) break;
            if (hthread_likely(ret == -1 && errno == EAGAIN))
            {
                new_val = *p;
            }
        }
    }
}

inline void hsem_post(int* p) __attribute__((always_inline));

void hsem_post(int* p)
{

    register int new_val;
    register int ret;
    new_val = h_fetch_and_add(p, 1);
    futex(p, FUTEX_WAKE, 1, NULL, NULL, 0);
    return;
    if (new_val <= 7)
        do
            ret = futex(p, FUTEX_WAKE, 1, NULL, NULL, 0);
        while (hthread_unlikely(ret != 1));
    //        perror("failure when set cpu affinity in thread\n");
    //        exit(EXIT_FAILURE);

}

#endif

