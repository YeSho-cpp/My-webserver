
#include <iostream>
#include <csignal>
#include "CGlmysql/sql_connection_pool.h"
using namespace std;

void handle_sigalrm(int sig) {
  printf("Timer fired!\n");
}

int main(int argc,char *argv[]) {

  struct itimerval timer;
  memset(&timer, 0, sizeof(timer));
  timer.it_value.tv_sec = 2;  // 初始延迟2秒
  timer.it_interval.tv_sec = 1;  // 之后每1秒触发一次

  signal(SIGALRM, handle_sigalrm);
  setitimer(ITIMER_REAL, &timer, NULL);

  while (1) {
    pause();  // 等待信号
  }

  return 0;
}

