//
// Created by YeSho on 2024/4/21.
//

#ifndef WEBSERVER_WEBSERVER_H
#define WEBSERVER_WEBSERVER_H

#include "../http/http_conn.h"
#include "../CGlmysql/sql_connection_pool.h"
#include "../threadpool/threadpool.h"
#include "../timer/lst_timer.h"

#define MAX_FD 65536
#define MAX_EVENT_NUMBER 10000
#define TIMESLOT 5

class Webserver{
public:
  Webserver();
  ~Webserver();
  void init(int port,string user,string passwd,string databaseName,int log_write,int opt_linger,int trigmode,int sql_num,int thread_num,int close_log,int actor_model);
  void log_write();
  void sql_pool();
  void threadPool();
  void trig_mode();
  void eventListen();
  void eventLoop();

  bool dealclientdata();
  bool dealwithsignal(bool &timeout,bool &stop_server);
  void deal_timer(util_timer *timer,int sockfd);
  void dealwithread(int sockfd);
  void dealwithwrite(int sockfd);
  void timer(int conn_fd, struct sockaddr_in client_address);
  void adjust_timer(util_timer *timer);

private:

  //基础
  int m_close_log; // 是否关闭日志
  int m_is_async; // 日志是否异步
  char *m_root;
  http_conn *users;
  int m_listen_fd;
  int m_TRIGMode;
  int m_LISTENTrigmode;
  int m_CONNTrigmode;
  int m_opt_linger;
  int m_epoll_fd;
  int m_pipefd[2];
  epoll_event events[MAX_EVENT_NUMBER];

  // 数据库相关
  connection_pool *m_sql_pool;
  int m_port;
  string m_user;
  string m_password;
  string m_dbName;
  int m_sql_num;

  // 线程池相关
  thread_pool<http_conn> *m_pool;
  int m_actormodel;
  int m_thread_num;

  // 定时相关
  client_data *user_timer;
  Utils utils;

};

#endif//WEBSERVER_WEBSERVER_H
