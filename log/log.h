//
// Created by YeSho on 2024/4/15.
//

#ifndef WEBSERVER_LOG_H
#define WEBSERVER_LOG_H
#include "../lock/lock.h"
#include "block_queue.h"
#include <cstring>
#include <string>
class Log{
public:
  //  采用懒汉的单例模式
  static Log* get_instance(){
    static Log instance;
    return &instance;
  }
  static void* flush_log_thread(void *args){
    return Log::get_instance()->async_write_log();
  }
  bool init(const char* file_name,int close_log,int log_buf_size,int split_size,int max_queue_size=0);

  void write_log(int level,const char *format,...);

  void flush();

  static tm get_time();

private:
  Log();
  virtual ~Log(){
    if(m_fp!= nullptr)
      fclose(m_fp);
  }
  void *async_write_log(){
    std::string log_str;
    while(m_block_queue->pop(log_str)){
      m_mutex.lock();
      fputs(log_str.c_str(),m_fp);
      m_mutex.unlock();
    }
  }


private:
  char log_name[128]{}; //日志文件名
  char dir_name[128]{}; //目录名称
  int m_close_log{};
  long long m_count; //日志行数
  int m_split_size{}; //日志最大行数
  int m_log_buf_size{}; // 日志缓冲区大小
  int m_today{};
  char * m_buf{};
  block_queue<std::string> *m_block_queue{}; // 阻塞队列(异步使用)
  bool m_is_async; //是否同步
  FILE *m_fp{}; // 文件描述符
  locker m_mutex; // 互斥锁
};


#define LOG_DEBUG(foramt,...) if(m_close_log==0) { Log::get_instance()->write_log(0,foramt, ##__VA_ARGS__); Log::get_instance()->flush();};
#define LOG_INFO(foramt,...) if(m_close_log==0) { Log::get_instance()->write_log(1,foramt, ##__VA_ARGS__); Log::get_instance()->flush();};
#define LOG_WARN(foramt,...) if(m_close_log==0) { Log::get_instance()->write_log(2,foramt, ##__VA_ARGS__); Log::get_instance()->flush();};
#define LOG_ERROR(foramt,...) if(m_close_log==0) { Log::get_instance()->write_log(3,foramt, ##__VA_ARGS__); Log::get_instance()->flush();};

#endif//WEBSERVER_LOG_H
