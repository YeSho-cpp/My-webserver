//
// Created by YeSho on 2024/4/16.
//

#ifndef WEBSERVER_LST_TIMER_H
#define WEBSERVER_LST_TIMER_H

#include <netinet/in.h>
#include <ctime>
#include <sys/epoll.h>
#include <sys/fcntl.h>
#include <cstring>
#include <csignal>
#include <cassert>

class util_timer;

class tw_timer;

struct client_data{
  sockaddr_in address;
  int sockfd;
  util_timer *timer;
};

class util_timer{
public:
  util_timer() : prev(nullptr),next(nullptr){}

public:
  time_t expire{};
  client_data *user_data{};
  void (*cb_func)(client_data *){};
  util_timer* prev;
  util_timer* next;
};

class sort_timer_lst{
public:
  sort_timer_lst();
  ~sort_timer_lst();
  void add_timer(util_timer *timer);
  void del_timer(util_timer *timer);
  void adjust_timer(util_timer *timer);
  void tick();
private:
  void add_timer(util_timer *timer,util_timer* lst_head);
  util_timer *head;
  util_timer *tail;
};



class Utils{
public:
  Utils() = default;
  ~Utils() = default;
  void init(int time_slot);
  int setnonblocking(int fd);
  void addfd(int epollfd,int fd,bool one_shot,int TRIGMode);
  void addsig(int sig,void(handler)(int),bool restart= true);
  static void sig_handler(int sig);
  void show_errno(int connfd,const char *info);
  void time_handler();

public:
  static int *u_pipefd;
  static int u_epollfd;
  int m_TIMESLOT{};
  sort_timer_lst m_time_lst;
};

void cb_func(client_data *uer_data);







#endif//WEBSERVER_LST_TIMER_H
