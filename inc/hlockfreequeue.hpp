#include <boost/lockfree/queue.hpp>
#include <pwarpper.h>
#include <time.h>
#include <stdlib.h>

template<typename T>
class hlockfreequeue {

    private:
        boost::lockfree::queue<T, boost::lockfree::fixed_sized<true> > &m_q;
       // boost::lockfree::queue<T > &m_q;

    public:

        hlockfreequeue<T> (size_t q_size): 
            m_q (* new boost::lockfree::queue<T, boost::lockfree::fixed_sized<true> > (q_size)) {
                printf("is_lock_free: %d\n", m_q.is_lock_free());
            }

        ~hlockfreequeue<T> () { 
//            delete &m_q;
        }

        bool try_dequeue(T& item) {
            return m_q.pop(item);
        }

        bool try_enqueue(T& item) {
            return m_q.push(item);
        }

        void enqueue(const T item) {
            int t = 1;
            while (!m_q.push(item)) {
            //    (*pwarpper::raw_sched_yield)();
            //    usleep(0xFF & rand());
                 usleep(t);
                t = ( t << 1 ) |  1 & 0x000000FF; 
            }
        }

        void dequeue(T& item) {
            int t = 1;
            while (!try_dequeue(item)) {
             //   (*pwarpper::raw_sched_yield)();
                usleep(t);
                t = ( t << 1 ) |  1 & 0x000000FF;
            }
        }
};
