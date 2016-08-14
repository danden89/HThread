#ifndef HDEBUG_H
#define	HDEBUG_H

#define hthread_likely(x)       __builtin_expect((x),1)
#define hthread_unlikely(x)     __builtin_expect((x),0)

#define DEBUG_PREFIX "%s, %d: "
//#define DEBUG

#ifdef HDEBUG  

//#define debug_util(fmt, args...)  printf (DEBUG_PREFIX"fmt"\n" ,__FILE__, __LINE__, ##args)   
//
//#define debug(fmt, args...)  debug_util(fmt, ##args) 
#define debug(fmt, args...) \
    do{ \
        printf(DEBUG_PREFIX, __FILE__, __LINE__); \
        printf(fmt, ##args);  \
    }while (0);
              
#else
#define debug(fmt, args...) 
#endif

template<typename T>
void print_bytes(T t) {
    int s = sizeof(T);
    printf("size = %d\n", s);
    char *p = (char*)&t;
    while ( s-- ) {
        printf("%2x ", (unsigned char)*p);
        p++;
    }
    puts("");
}

#endif	/* HDEBUG_H */

