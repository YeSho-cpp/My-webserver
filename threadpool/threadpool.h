//
// Created by YeSho on 2024/4/16.
//

#ifndef WEBSERVER_THREADPOOL_H
#define WEBSERVER_THREADPOOL_H

#include <csignal>
#include "../CGlmysql/sql_connection_pool.h"
#include "../lock/lock.h"
#include <list>
using namespace std;

template<class T>
class thread_pool{
public:
  thread_pool(int actor_mode,connection_pool* connPool,int thread_num=8,int max_quest=1000);
  ~thread_pool();
  bool append(T *request,int state);
  bool append_p(T *request);
private:
  static void* worker(void *arg);
  void run();
private:
  int m_thread_num;
  int m_max_quest;
  pthread_t* m_threads;
  connection_pool* m_connPool{};
  int m_actor_mode;
  list<T *>m_work_queue;
  locker m_queue_locker;
  sem m_queue_stat;
};

template<class T>
void thread_pool<T>::run() {
  while (true){
    m_queue_stat.wait();
    m_queue_locker.lock();
    if(m_work_queue.empty()){
      m_queue_locker.unlock();
      continue;
    }

    T* request=m_work_queue.front();
    m_work_queue.pop_front();
    m_queue_locker.unlock();
    if(!request) continue;
    // 需要补充

  }
}

template<class T>
void *thread_pool<T>::worker(void *arg) {
  auto *pool=(thread_pool*)arg;
  pool->run();
  return pool;
}

template<class T>
bool thread_pool<T>::append(T *request, int state) {
  m_queue_locker.lock();
  if(!request||m_work_queue.size()>=m_max_quest) {
    m_queue_locker.unlock();
    return false;
  }
  request->m_state=state;
  m_work_queue.push_back(request);
  m_queue_locker.unlock();
  m_queue_stat.post();
  return true;
}

template<class T>
bool thread_pool<T>::append_p(T *request) {

  m_queue_locker.lock();
  if(!request||m_work_queue.size()>=m_max_quest) {
    m_queue_locker.unlock();
    return false;
  }
  m_work_queue.push_back(request);
  m_queue_locker.unlock();
  m_queue_stat.post();
  return true;
}

template<class T>
thread_pool<T>::~thread_pool() {
  delete[] m_threads;
}

template<class T>
thread_pool<T>::thread_pool(int actor_mode, connection_pool *connPool, int thread_num, int max_quest):m_thread_num(thread_num),m_max_quest(max_quest),m_threads(nullptr),m_connPool(connPool),m_actor_mode(actor_mode)
{
  if(thread_num<=0||max_quest<=0) {
    throw std::exception();
  }
  m_threads=new pthread_t[thread_num];

  for(int i=0;i<thread_num;++i){
    if(pthread_create(m_threads+i, nullptr,worker,this)!=0)
    {
      delete []m_threads;
      throw std::exception();
    }

    if(pthread_detach(m_threads[i]))
    {
      delete []m_threads;
      throw std::exception();
    }

  }
}

#endif//WEBSERVER_THREADPOOL_H
