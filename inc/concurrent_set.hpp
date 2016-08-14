#ifndef CONCURRENT_SET_HPP
#define	CONCURRENT_SET_HPP

#include <unordered_set>
#include <vector>
#include <pthread.h>
#include <deque>

namespace hthread{

template<class T, int SIZE = 32, int BUCKET_SIZE = 16>
class concurrent_set {
    
    std::unordered_set<T>* buckets[SIZE];
    pthread_spinlock_t buckets_locks[SIZE];
    
    
    
    inline int hash(T v) {
        char *p = (char *)&v;
        int n = sizeof(T);
        char sum = 0;
        for ( int i = 0; i < n; i++, p++ ) {
            sum += *p;
        }
        //sum = ( sum << 3 ) - sum; // sum *= 7 ignore overflow 
        sum *= 179425823;
        sum >>= 3;
        //std::cout << "sum  & (SIZE-1)" <<  (sum  & (SIZE-1)) << std::endl;
        return sum  & (SIZE-1); //assume SIZE is always power of 2
    }

    public:
        
    concurrent_set<T, SIZE, BUCKET_SIZE>(){
        pthread_spinlock_t tmp;
        pthread_spin_init(&tmp, PTHREAD_PROCESS_PRIVATE); 
        for ( int i = 0; i < SIZE; i++ ) {
            buckets[i] = new std::unordered_set<T>(BUCKET_SIZE);
            buckets_locks[i] = tmp;
        }
    }
    
    ~concurrent_set<T, SIZE, BUCKET_SIZE>(){
        for ( int i = 0; i < SIZE; i++ ) {
            delete buckets[i];
        }
    }
    
    bool insert(T v) {
        int bucket_idx = hash(v);
        bool ret;
        pthread_spin_lock(buckets_locks+bucket_idx);
        if ( buckets[bucket_idx]->find(v) != buckets[bucket_idx]->end() ){
            ret = false; // found, return false
        } else {
            buckets[bucket_idx]->insert(v);
            ret = true; // not found, insert and return true;
        }
        pthread_spin_unlock(buckets_locks+bucket_idx);
        return ret;
    }
    
    bool erase(T v) {
        int bucket_idx = hash(v);
        bool ret;
        pthread_spin_lock(buckets_locks+bucket_idx);
        if ( buckets[bucket_idx]->find(v) == buckets[bucket_idx]->end() ){
            ret = false; // do not found, return false
        } else {
            buckets[bucket_idx]->erase(v);
            ret = true; // found and erased, return true
        }
        pthread_spin_unlock(buckets_locks+bucket_idx);
        return ret;
    }
    
    //unsafe
    bool empty() {
        for ( int i = 0; i < SIZE; i++ ) {
            if ( !buckets[i]->empty() ) return false;
        }
        return true;
    }
    
    //unsafe
    std::deque<T> get_all(){
        std::deque<T> ret;
        for (int i = 0; i < SIZE; i++)
        {
            if (!buckets[i]->empty())
            {
                pthread_spin_lock(buckets_locks + i);
                for (auto it = buckets[i]->begin(); \
                        it != buckets[i]->end(); it++)
                {
                    ret.push_back(*it);
                }
                pthread_spin_unlock(buckets_locks + i);
            }
        }
        return ret;
    }
};

};
#endif	

