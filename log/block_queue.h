//
// Created by YeSho on 2024/4/14.
//

#ifndef WEBSERVER_BLOCK_QUEUE_H
#define WEBSERVER_BLOCK_QUEUE_H

#include "../lock/lock.h"
#include <cstdlib>
#include <sys/time.h>
template<class T>
class block_queue{
public:
  // 构造 析构 clear  full empty size() maxsize() back front push pop pop()

  explicit block_queue(int maxsize=1000){
    if(maxsize<=0){
      exit(-1);
    }
    m_size=0;
    m_maxsize=maxsize;
    m_array=new T[maxsize];
    m_front=-1;
    m_back=-1;
  }

  ~block_queue(){
    m_lock.lock();
    delete[] m_array;
    m_lock.unlock();
  }

  void clear(){
    m_lock.lock();
    m_size=0;
    m_front=-1;
    m_back=-1;
    m_lock.unlock();
  }

  bool full(){
    m_lock.lock();

    if(m_size==m_maxsize) {
      m_lock.unlock();
      return true;
    }
    m_lock.unlock();
    return false;
  }

  bool empty(){
    m_lock.lock();

    if(m_size==0){
      m_lock.unlock();
      return true;
    }
    m_lock.unlock();
    return false;
  }

  bool front(T &item){
    m_lock.lock();
    if(m_size==0){
      m_lock.unlock();
      return false;
    }
    m_array[m_front]=item;
    m_lock.unlock();
    return true;
  }
  bool back(T &item){
    m_lock.lock();
    if(m_size==0){
      m_lock.unlock();
      return false;
    }
    m_array[m_back]=item;
    m_lock.unlock();
    return true;
  }

  int size(){
    m_lock.lock();
    int tmp=0;
    tmp=m_size;
    m_lock.unlock();
    return tmp;
  }

  int maxsize(){
    m_lock.lock();
    int tmp=0;
    tmp=m_maxsize;
    m_lock.unlock();
    return tmp;
  }

  bool push(const T&item){
    m_lock.lock();
    if(m_size==m_maxsize){ //队列满了
      m_cond.broadcast();
      m_lock.unlock();
      return false;
    }

    m_back=(m_back+1)%m_maxsize;
    m_array[m_back]=item;

    m_size++;
    m_cond.broadcast();
    m_lock.unlock();
    return true;
  }

  bool pop(T &item){
    m_lock.lock();
    while (m_size<=0){
      if(!m_cond.wait(m_lock.get())){
        m_lock.unlock();
        return false;
      }
    }
    m_front=(m_front+1)%m_maxsize;
    item=m_array[m_front];

    m_size--;
    m_lock.unlock();
    return true;
  }

  bool pop(T &item,int timeout){ // 毫秒
    m_lock.lock();
    struct timespec t{0,0};
    struct timeval now{0,0};

    gettimeofday(&now, nullptr);

    t.tv_sec=now.tv_sec+timeout/1000;
    t.tv_nsec=(timeout%1000)*1000;
    if(m_size<=0){
      if(!m_cond.timewait(m_lock.get(),t)){
        m_lock.unlock();
        return false;
      }
    }

    if(m_size<=0){
      m_lock.unlock();
      return false;
    }

    m_front=(m_front+1)%m_maxsize;
    item=m_array[m_front];
    m_size--;
    m_lock.unlock();
    return true;
  }

private:
  cond m_cond;
  locker m_lock;

  int m_maxsize{};
  int m_size{};
  T* m_array;
  int m_front{};
  int m_back{};
};

#endif//WEBSERVER_BLOCK_QUEUE_H
