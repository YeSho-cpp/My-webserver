//
// Created by YeSho on 2024/4/16.
//
#include "lst_timer.h"
#include "../http/http_conn.h"
#include <cerrno>


sort_timer_lst::sort_timer_lst() {
  head= nullptr;
  tail= nullptr;
}
sort_timer_lst::~sort_timer_lst() {
  util_timer *tmp=head;
  while(tmp){
    head=tmp->next;
    delete tmp;
    tmp=head;
  }
}
void sort_timer_lst::add_timer(util_timer *timer) {
  if (timer== nullptr) return;
  if(head== nullptr&&tail== nullptr){
    head=tail=timer;
    return;
  }
  // 插在头部
  if(head!= nullptr&&timer->expire<head->expire){
    timer->next=head;
    head->prev=timer;
    timer->prev= nullptr;
    head=timer;
    return;
  }

  // 插在尾部
  if(tail!= nullptr&&timer->expire>tail->expire){
    tail->next=timer;
    timer->prev=tail;
    timer->next= nullptr;
    tail=timer;
    return;
  }

  // 插在中间，只能搜索了

  add_timer(timer,head);

}
void sort_timer_lst::add_timer(util_timer *timer, util_timer *lst_head) {
  // 从头部开始查找第一个大于超时大于timer位置进行插入

  util_timer *pre=lst_head;
  util_timer *cur=lst_head->next;
  while(cur){
    if(cur->expire>timer->expire){
      timer->prev=pre;
      timer->next=cur;
      pre->next=timer;
      cur->prev=timer;
      break;
    }
    cur=cur->next;
    pre=pre->next;
  }
  if(cur== nullptr){
    pre->next=timer;
    timer->prev=pre;
    timer->next= nullptr;
    tail=timer;
  }
}
void sort_timer_lst::del_timer(util_timer *timer) {
  if (timer== nullptr) return;
  if((timer==head)&&(timer==tail)){
    delete timer;
    head= nullptr;
    tail= nullptr;
    return;
  }
  if(timer==head){
    auto tmp=head;
    head=head->next;
    head->prev= nullptr;
    delete timer;
    return;
  }

  if(timer==tail){
    auto tmp=tail;
    tail=tail->prev;
    tail->next= nullptr;
    delete timer;
    return;
  }
  timer->prev->next=timer->next;
  timer->next->prev=timer->prev;
  delete timer;
}
void sort_timer_lst::adjust_timer(util_timer *timer) {
  // 这个只负责延长时间
  if (timer==nullptr) return;
  auto tmp=timer->next;
  //  延长后比最小的还小
  if(tmp== nullptr||timer->expire<tmp->expire){
    return;
  }
  if(timer==head){
    head=head->next;
    head->prev= nullptr;
    timer->next= nullptr;
    add_timer(timer,head);
  }

  else{ // 先删除后插入
    timer->prev->next = timer->next;
    timer->next->prev = timer->prev;
    add_timer(timer, timer->next);
  }

}
void sort_timer_lst::tick() { // 就是从头节点开始查找直到一个未到期的定时器，之前的都要触发该定时器的回调函数
  if(head== nullptr) return;

  time_t cur= time(nullptr);
  util_timer *tmp=head;
  while(tmp){

    if(tmp->expire>cur){
      break;
    }

    tmp->cb_func(tmp->user_data);

    head=tmp->next;
    if(head){
      head->prev= nullptr;
    }
    delete tmp;
    tmp=head;
  }
}

int* Utils::u_pipefd=nullptr;
int Utils::u_epollfd=0;

void Utils::init(int time_slot) {
  m_TIMESLOT=time_slot;
}
int Utils::setnonblocking(int fd) {
  int old_op=fcntl(fd,F_GETFL);
  int new_op=old_op|O_NONBLOCK;
  fcntl(fd,F_SETFL,new_op);
  return old_op;
}
void Utils::addfd(int epollfd, int fd, bool one_shot, int TRIGMode) {
  epoll_event event{};
  event.data.fd=fd;
  if(one_shot)
    event.events|=EPOLLONESHOT;
  if(TRIGMode==1)
    event.events=EPOLLIN|EPOLLET|EPOLLHUP;
  else
    event.events=EPOLLIN|EPOLLHUP;
  epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);
  setnonblocking(fd);
}
void Utils::addsig(int sig, void (handler)(int), bool restart) {
  struct sigaction sa{};
  memset(&sa,'\0', sizeof(sa));
  sa.sa_handler=handler;
  if(restart)
    sa.sa_flags|=SA_RESTART;
  sigfillset(&sa.sa_mask);
  assert(sigaction(sig, &sa, nullptr) != -1);
}
void Utils::sig_handler(int sig) {
  int olderrno=errno;
  int msg=sig;
  send(u_pipefd[1],(char *)&msg,1,0);
  errno=olderrno;
}
void Utils::show_errno(int connfd, const char *info) {
  send(connfd,info, strlen(info), 0);
  close(connfd);
}
void Utils::time_handler() {
  m_time_lst.tick();
  alarm(m_TIMESLOT);
}

void cb_func(client_data *uer_data){

  epoll_ctl(Utils::u_epollfd,EPOLL_CTL_DEL,uer_data->sockfd, nullptr);
  assert(uer_data);
  close(uer_data->sockfd);
  // 后面补充
  http_conn::m_user_count--;
}
