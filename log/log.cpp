//
// Created by YeSho on 2024/4/15.
//
#include "log.h"

Log::Log() {
  m_count=0;
  m_is_async= false;
}

bool Log::init(const char *name, int close_log, int log_buf_size=8192, int split_size=5000000, int max_queue_size) {

  if(max_queue_size>0){ // 异步
    m_is_async= true;
    m_block_queue=new block_queue<std::string>(max_queue_size);
    pthread_t tid;
    pthread_create(&tid, nullptr,Log::flush_log_thread, nullptr);
  }

  m_close_log=close_log;
  m_split_size=split_size;
  m_buf=new char[log_buf_size];
  memset(m_buf,'\0', m_log_buf_size);

  tm my_tm=Log::get_time();

  m_today=my_tm.tm_mday;

  const char *p=strchr(name,'/');

  char full_log_name[256];
  if(p== nullptr){ // 没有目录
    strcpy(log_name,name);
    snprintf(full_log_name,255,"%d_%02d_%02d_%s",my_tm.tm_year+1900,my_tm.tm_mon+1,my_tm.tm_mday,log_name);
  }
  else{  //有目录
    strcpy(log_name,p+1);
    strncpy(dir_name,name,p- name+1);
    snprintf(full_log_name,255,"%s%d_%02d_%02d_%s",dir_name,my_tm.tm_year+1900,my_tm.tm_mon+1,my_tm.tm_mday,log_name);
  }

  m_fp= fopen(full_log_name,"a");
  if(m_fp== nullptr){
    return false;
  }

  return true;
}
void Log::write_log(int level,const char *format,...) {

  tm my_tm=Log::get_time();

  char s[16]={0};
  switch (level) {
    case 0:
      strcpy(s,"[debug]:");
      break;
    case 1:
      strcpy(s,"[info]:");
      break;
    case 2:
      strcpy(s,"[warn]:");
      break;
    case 3:
      strcpy(s,"[error]:");
      break;
    default:
      strcpy(s,"[info]:");
      break;
  }

  m_mutex.lock();
  m_count++;
  if(my_tm.tm_mday!=m_today||m_count%m_split_size==0){
    char new_log[256]={0};
    fflush(m_fp);
    fclose(m_fp);
    char tail[16]={0};
    snprintf(tail,16,"%d_%02d_%02d_",my_tm.tm_year+1900,my_tm.tm_mon+1,my_tm.tm_mday);
    if(my_tm.tm_mday!=m_today){ // 日期不同，要新建日志文件
      m_count=0;
      snprintf(new_log,255,"%s%s%s",dir_name,tail,log_name);
      m_today=my_tm.tm_mday;

    }
    else if(m_count%m_split_size==0){
      snprintf(new_log, 255, "%s%s%s.%lld", dir_name, tail, log_name, m_count / m_split_size);
    }

    m_fp=fopen(new_log,"a");

  }
  m_mutex.unlock();

  va_list list;
  va_start(list,format);
  std::string log_str;
  m_mutex.lock();
  int n= snprintf(m_buf,48,"%d-%01d-%01d %01d:%01d:%01d %s",my_tm.tm_year+1900,my_tm.tm_mon+1,my_tm.tm_mday,my_tm.tm_hour,my_tm.tm_min,my_tm.tm_sec,s);
  int m= vsnprintf(m_buf+n,m_log_buf_size-n-1,format,list);
  m_buf[m+n]='\n';
  m_buf[m+n+1]='\0';
  log_str=m_buf;
  m_mutex.unlock();
  if(m_is_async&&!m_block_queue->full()){ // 异步

    m_block_queue->push(log_str);
  }
  else{    // 同步
    m_mutex.lock();
    fputs(log_str.c_str(),m_fp);
    m_mutex.unlock();
  }
  va_end(list);

}
tm Log::get_time() {
  time_t t= time(nullptr);
  tm *sys_tm= localtime(&t);
  return  *sys_tm;
}
void Log::flush() {
  m_mutex.lock();
  fflush(m_fp);
  m_mutex.unlock();
}
