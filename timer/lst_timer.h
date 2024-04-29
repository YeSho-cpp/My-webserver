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

/*定时器类*/
class tw_timer {
public:
  tw_timer(int rot, int ts)
      : next(nullptr), prev(nullptr), rotation(rot), time_slot(ts) {}

public:
  int rotation;                   /*记录定时器在时间轮转多少圈后生效*/
  int time_slot;                  /*记录定时器属于时间轮上哪个槽（对应的链表，下同）*/
  void (*cb_func)(client_data *){}; /*定时器回调函数*/
  client_data *user_data{};         /*客户数据*/
  tw_timer *next;                 /*指向下一个定时器*/
  tw_timer *prev;                 /*指向前一个定时器*/
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

class time_wheel {
public:
  time_wheel() : cur_slot(0) {
    for (auto & slot : slots) {
      slot = nullptr; /*初始化每个槽的头结点*/
    }
  }
  ~time_wheel();
  /*根据定时值timeout创建一个定时器，并把它插入合适的槽中*/
  tw_timer *add_timer(int timeout);
  /*删除目标定时器timer*/
  void del_timer(tw_timer *timer);
  /*SI时间到后，调用该函数，时间轮向前滚动一个槽的间隔*/
  void tick();

private:
  /*时间轮上槽的数目*/
  static const int N = 60;
  /*每1 s时间轮转动一次，即槽间隔为1 s*/
  static const int SI = 1;
  /*时间轮的槽，其中每个元素指向一个定时器链表，链表无序*/
  tw_timer *slots[N]{};
  int cur_slot; /*时间轮的当前槽*/
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
