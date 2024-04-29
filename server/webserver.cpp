//
// Created by YeSho on 2024/4/21.
//

// 这里的路径由项目路径和根目录组成

#include "webserver.h"
#include <arpa/inet.h>


Webserver::Webserver() {
  //http_conn类对象
  users=new http_conn[MAX_FD];

  //root文件夹路径
  char server_path[200];
  getcwd(server_path,200);
  char root[6]="/root";
  m_root=(char *) malloc(strlen(root)+ strlen(server_path)+1);
  strcpy(m_root,server_path);
  strcat(m_root,root);

  // 定时器
  user_timer=new client_data[MAX_FD];
}

void Webserver::init(int port, string user, string passwd, string databaseName, int log_write, int opt_linger, int trigmode, int sql_num, int thread_num, int close_log, int actor_model) {
  m_port=port;
  m_user=user;
  m_password=passwd;
  m_dbName=databaseName;
  m_is_async=log_write;
  m_opt_linger=opt_linger;
  m_TRIGMode=trigmode;
  m_sql_num=sql_num;
  m_thread_num=thread_num;
  m_close_log=close_log;
  m_actormodel=actor_model;
}


Webserver::~Webserver() {
  close(m_epoll_fd);
  close(m_pipefd[0]);
  close(m_pipefd[1]);
  close(m_listen_fd);
  delete[] users;
  delete[] user_timer;
  delete m_pool;
  free(m_root);
}
void Webserver::log_write() {
  if(m_close_log==0){
    if(m_is_async==0){
      Log::get_instance()->init("./ServerLog",m_close_log,2000,800000);
    }
    else if(m_is_async==1){
      Log::get_instance()->init("./ServerLog",m_close_log,2000,800000,800);
    }
  }
}
void Webserver::sql_pool() {
  m_sql_pool = connection_pool::GetInstance();
  m_sql_pool->init("localhost",3306,m_user,m_password,m_dbName,m_close_log,m_sql_num);
  users->initmysql_result(m_sql_pool);
}
void Webserver::threadPool() {
  m_pool=new thread_pool<http_conn>(m_actormodel,m_sql_pool,m_thread_num);
}
void Webserver::trig_mode() {

  if(m_TRIGMode==0){ // LT+LT
    m_LISTENTrigmode=0;
    m_CONNTrigmode=0;
  }

  if(m_TRIGMode==1){ // LT+ET
    m_LISTENTrigmode=0;
    m_CONNTrigmode=1;
  }

  if(m_TRIGMode==2){ // ET+LT
    m_LISTENTrigmode=1;
    m_CONNTrigmode=0;
  }

  if(m_TRIGMode==3){ // ET+ET
    m_LISTENTrigmode=1;
    m_CONNTrigmode=1;
  }
}
void Webserver::eventListen() {

  //网络编程基础步骤
  m_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  assert(m_listen_fd>=0);

  //优雅关闭连接
  if(m_opt_linger==0){ // 优雅关闭连接
    struct linger tmp{0,1};
    setsockopt(m_listen_fd,SOL_SOCKET,SO_LINGER,&tmp, sizeof(tmp));
  }
  else if(m_opt_linger==1){ // 不优雅
    struct linger tmp{1,1};
    setsockopt(m_listen_fd,SOL_SOCKET,SO_LINGER,&tmp, sizeof(tmp));
  }

  int ret=0;
  sockaddr_in address{};
  bzero(&address, sizeof(address));
  address.sin_port= htons(m_port);
  address.sin_family=AF_INET;
  address.sin_addr.s_addr= htonl(INADDR_ANY);

  int flag=1;
  setsockopt(m_listen_fd,SOL_SOCKET,SO_REUSEADDR,&flag, sizeof(flag));
  ret= bind(m_listen_fd, reinterpret_cast<const sockaddr *>(&address), sizeof(address));
  assert(ret>=0);
  ret= listen(m_listen_fd,5);
  assert(ret>=0);

  utils.init(TIMESLOT);

  //epoll创建内核事件表
  // epoll_event events[MAX_EVENT_NUMBER];
  m_epoll_fd=epoll_create(5);
  assert(m_epoll_fd!=-1);

  utils.addfd(m_epoll_fd,m_listen_fd, false,m_LISTENTrigmode);

  ret= socketpair(PF_UNIX,SOCK_STREAM,0,m_pipefd);
  assert(ret!=-1);
  utils.setnonblocking(m_pipefd[1]);
  utils.addfd(m_epoll_fd,m_pipefd[0], false,0);

  utils.addsig(SIGPIPE,SIG_IGN);
  utils.addsig(SIGALRM,Utils::sig_handler, false);
  utils.addsig(SIGTERM,Utils::sig_handler, false);

  alarm(TIMESLOT);

  //工具类,信号和描述符基础操作
  http_conn::m_epoll_fd=m_epoll_fd;
  Utils::u_epollfd=m_epoll_fd;
  Utils::u_pipefd=m_pipefd;

}
void Webserver::eventLoop() {
  bool stop_server=false;
  bool timeout=false;
  while(!stop_server){
    int num= epoll_wait(m_epoll_fd,events,MAX_EVENT_NUMBER,-1);
    if(num<0&&errno!=EINTR){
      LOG_ERROR("%s","epoll failure");
      break;
    }
    for(int i=0;i<num;i++){
         int sock_fd= events[i].data.fd;

         //处理新到的客户连接
         if(sock_fd==m_listen_fd){
            bool flag=dealclientdata();
            if(!flag) continue;
         }
         else if(events[i].events&(EPOLLRDHUP|EPOLLHUP|EPOLLERR)){
           //服务器端关闭连接，移除对应的定时器
           util_timer *timer= user_timer[sock_fd].timer;
           deal_timer(timer,sock_fd);
         }
         else if((sock_fd==m_pipefd[0])&&(events[i].events&EPOLLIN)){ //处理信号
           bool flag= dealwithsignal(timeout,stop_server);
           if(!flag)
             LOG_ERROR("%s", "dealclientdata failure");
         }
         else if(events[i].events&EPOLLIN){ //处理客户连接上接收到的数据
           dealwithread(sock_fd);
         }
         else if(events[i].events&EPOLLOUT){
           dealwithwrite(sock_fd);
         }
    }
    if(timeout){
      utils.time_handler();
      LOG_INFO("%s", "timer tick");
      timeout= false;
    }
  }
}
bool Webserver::dealclientdata() {

  sockaddr_in client_address{};
  socklen_t client_len= sizeof(client_address);
  if(m_LISTENTrigmode==0){ // LT
    int conn_fd=accept(m_listen_fd, reinterpret_cast<sockaddr *>(&client_address),&client_len);
    if(conn_fd<0){
      LOG_ERROR("%s:errno is:%d", "accept error", errno);
      return false;
    }
    else if(http_conn::m_user_count>=MAX_FD){
      utils.show_errno(conn_fd, "Internal server busy");
      LOG_ERROR("%s", "Internal server busy");
      return true;
    }
    timer(conn_fd,client_address);
  }
  else{ //ET
    while(true){
      int conn_fd=accept(m_listen_fd, reinterpret_cast<sockaddr *>(&client_address),&client_len);
      if(conn_fd<0){
        LOG_ERROR("%s:errno is:%d", "accept error", errno);
        break;
      }
      else if(http_conn::m_user_count>=MAX_FD){
        utils.show_errno(conn_fd, "Internal server busy");
        LOG_ERROR("%s", "Internal server busy");
        break;
      }
      timer(conn_fd,client_address);
    }
    return false;
  }
  return true;
}
bool Webserver::dealwithsignal(bool &timeout, bool &stop_server) {
  char signals[1024];
  int ret=recv(m_pipefd[0],signals, sizeof(signals),0);
  if(ret<=0){
    return false;
  }

  for(int i=0;i<ret;++i){
    switch (signals[i]) {
      case SIGALRM:{
        timeout= true;
        break;
      }
      case SIGTERM:
      {
        stop_server= true;
        break;
      }
    }
  }
  return true;
}
void Webserver::deal_timer(util_timer *timer, int sockfd) {
  timer->cb_func(&user_timer[sockfd]);
  if(timer){
    utils.m_time_lst.del_timer(timer);
  }
  LOG_INFO("close fd %d",user_timer[sockfd].sockfd);
}
void Webserver::dealwithread(int sockfd) {
  util_timer *timer=user_timer[sockfd].timer;
  if(m_actormodel==1){ //reactor
    if(timer){
      adjust_timer(timer);
    }
    //若监测到读事件，将该事件放入请求队列
    m_pool->append(users + sockfd, 0);

    while (true)
    {
      if (1 == users[sockfd].improv)
      {
        if (1 == users[sockfd].timer_flag)
        {
          deal_timer(timer, sockfd);
          users[sockfd].timer_flag = 0;
        }
        users[sockfd].improv = 0;
        break;
      }
    }

  }
  else{ // proactor

      if(users[sockfd].read_once()){
        LOG_INFO("deal with the client(%s)", inet_ntoa(users[sockfd].get_address()->sin_addr));

        //若监测到读事件，将该事件放入请求队列
        m_pool->append_p(users + sockfd);

        if (timer)
        {
          adjust_timer(timer);
        }
      }
      else
      {
        deal_timer(timer, sockfd);
      }
  }
}

void Webserver::dealwithwrite(int sockfd) {
  util_timer *timer = user_timer[sockfd].timer;
  //reactor
  if (1 == m_actormodel)
  {
    if (timer)
    {
      adjust_timer(timer);
    }

    m_pool->append(users + sockfd, 1);

    while (true)
    {
      if (1 == users[sockfd].improv)
      {
        if (1 == users[sockfd].timer_flag)
        {
          deal_timer(timer, sockfd);
          users[sockfd].timer_flag = 0;
        }
        users[sockfd].improv = 0;
        break;
      }
    }
  }
  else
  {
    //proactor
    if (users[sockfd].write())
    {
      LOG_INFO("send data to the client(%s)", inet_ntoa(users[sockfd].get_address()->sin_addr));

      if (timer)
      {
        adjust_timer(timer);
      }
    }
    else
    {
      deal_timer(timer, sockfd);
    }
  }
}

void Webserver::timer(int conn_fd, struct sockaddr_in client_address) {
  users[conn_fd].init(conn_fd,client_address,m_root,m_CONNTrigmode,m_close_log,m_user,m_password,m_dbName);

  //初始化client_data数据
  //创建定时器,设置回调函数和超时时间,绑定用户数据,将定时器添加到链表中
  user_timer[conn_fd].address=client_address;
  user_timer[conn_fd].sockfd=conn_fd;
  auto *timer=new util_timer;
  timer->user_data=&user_timer[conn_fd];
  timer->cb_func=cb_func;
  time_t cur= time(nullptr);
  timer->expire=cur+3*TIMESLOT;
  user_timer[conn_fd].timer=timer;
  utils.m_time_lst.add_timer(timer);
}

//若有数据传输，则将定时器往后延迟3个单位
//并对新的定时器在链表上的位置进行调整
void Webserver::adjust_timer(util_timer *timer) {
  time_t cur = time(nullptr);
  timer->expire = cur + 3 * TIMESLOT;
  utils.m_time_lst.adjust_timer(timer);
  LOG_INFO("%s", "adjust timer once");
}
