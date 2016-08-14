#include "pwarpper.h"
#include "hdebug.h"
using namespace hthread;

// seem this does not work when initing the module
std::string pwarpper::pthread_so_name = "libpthread.so.0";

int (*(pwarpper::raw_pthread_create))(pthread_t *thread, const pthread_attr_t *attr,\
                void *(* start_routine) (void *), void *arg) = nullptr;
int (*pwarpper::raw_pthread_cancel)(pthread_t thread) = nullptr;
int (*pwarpper::raw_pthread_join)(pthread_t thread, void **retval) = nullptr;
void (*pwarpper::raw_pthread_exit)(void *retval) __attribute__((noreturn)) = nullptr;
void (*pwarpper::raw_pthread_cleanup_push)(void (*routine)(void *), void *arg) = nullptr;
pthread_t(*pwarpper::raw_pthread_self)(void) = nullptr;
pthread_t(*pwarpper::raw_pthread_barrier_wait)(pthread_barrier_t *barrier) = nullptr;
int (*pwarpper::raw_pthread_cond_wait)(pthread_cond_t *cond, pthread_mutex_t *mutex) = nullptr;
int (*pwarpper::raw_pthread_mutex_lock)(pthread_mutex_t *mutex) = nullptr;
int (*pwarpper::raw_pthread_mutex_unlock)(pthread_mutex_t *mutex) = nullptr;
int (*pwarpper::raw_pthread_mutex_trylock)(pthread_mutex_t *mutex) = nullptr;
int (*pwarpper::raw_sem_wait)(sem_t *sem) = nullptr;
int (*pwarpper::raw_sem_trywait)(sem_t *sem) = nullptr;
int (*pwarpper::raw_sem_post)(sem_t *sem) = nullptr;
int (*pwarpper::raw_pthread_cond_timedwait)(pthread_cond_t *cond,
        pthread_mutex_t *mutex,
        const struct timespec *abstime) = nullptr;
int (*pwarpper::raw_pthread_cond_signal)(pthread_cond_t *cond) = nullptr;
int (*pwarpper::raw_pthread_cond_broadcast)(pthread_cond_t *cond) = nullptr;

void *(*pwarpper::raw_pthread_getspecific)(pthread_key_t key) = nullptr;
int (*pwarpper::raw_pthread_setspecific)(pthread_key_t key, const void *value) = nullptr;

int (*pwarpper::raw_pthread_yield)(void) = nullptr;
int (*pwarpper::raw_sched_yield)(void) = nullptr;
        
pwarpper::pwarpper() {
    std::string tstr = "in pwarpper constructor\n";
    debug(tstr.c_str());
    init_all();
}

void pwarpper::init_all() {
    char *err;
    handle = dlopen("libpthread.so.0", RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "%s\n", dlerror());
        exit(EXIT_FAILURE);
    }
    init_func();
}

void * pwarpper::get_routine(std::string sym) {
    char *err;
    void *func = dlsym(handle, sym.c_str());
    err = dlerror();
    if (err) {
        fprintf(stderr, "%s\n", err);
        exit(EXIT_FAILURE);
    }
    return func;
}

void pwarpper::init_func() {
    *(void **) (&(pwarpper::raw_pthread_create)) = get_routine("pthread_create");
    *(void **) (&(pwarpper::raw_pthread_cancel)) = get_routine("pthread_cancel");
    *(void **) (&(pwarpper::raw_pthread_join)) = get_routine("pthread_join");
    *(void **) (&(pwarpper::raw_pthread_exit)) = get_routine("pthread_exit");
    *(void **) (&(pwarpper::raw_pthread_cleanup_push)) = get_routine("_pthread_cleanup_push");
    *(void **) (&(pwarpper::raw_pthread_self)) = get_routine("pthread_self");
    *(void **) (&(pwarpper::raw_pthread_barrier_wait)) = get_routine("pthread_barrier_wait");
    *(void **) (&(pwarpper::raw_pthread_cond_wait)) = get_routine("pthread_cond_wait");
    *(void **) (&(pwarpper::raw_pthread_mutex_lock)) = get_routine("pthread_mutex_lock");
    *(void **) (&(pwarpper::raw_pthread_mutex_unlock)) = get_routine("pthread_mutex_unlock");
    *(void **) (&(pwarpper::raw_pthread_mutex_trylock)) = get_routine("pthread_mutex_trylock");
    *(void **) (&(pwarpper::raw_pthread_cond_timedwait)) = get_routine("pthread_cond_timedwait");
    *(void **) (&(pwarpper::raw_pthread_cond_signal)) = get_routine("pthread_cond_signal");    
    *(void **) (&(pwarpper::raw_pthread_cond_broadcast)) = get_routine("pthread_cond_broadcast");
    *(void **) (&(pwarpper::raw_sem_wait)) = get_routine("sem_wait");
    *(void **) (&(pwarpper::raw_sem_trywait)) = get_routine("sem_trywait");
    *(void **) (&(pwarpper::raw_sem_post)) = get_routine("sem_post");
    *(void **) (&(pwarpper::raw_pthread_getspecific)) = get_routine("pthread_getspecific");
    *(void **) (&(pwarpper::raw_pthread_setspecific)) = get_routine("pthread_setspecific");
    *(void **) (&(pwarpper::raw_pthread_yield)) = get_routine("pthread_yield");
    *(void **) (&(pwarpper::raw_sched_yield)) = get_routine("sched_yield");
}

//must use c++11 use ensure the singleton
pwarpper* pwarpper::getInstance() {
    static pwarpper* pwarp = new pwarpper();
    return pwarp;
}

pwarpper::pwarpper(const pwarpper& orig) {
}

pwarpper::~pwarpper() {
}

extern "C" void pwarpper::test() {
    std::cout << "test " << std::endl;
}
