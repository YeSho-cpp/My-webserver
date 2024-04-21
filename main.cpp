
#include "CGlmysql/sql_connection_pool.h"
#include "log/log.h"
#include <csignal>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <sys/uio.h>
#include <thread>
#include <unistd.h>
using namespace std;



int main(int argc,char *argv[]) {

  char *name="yehso";
  char *password="701121";
  char *sql_insert = (char *)malloc(sizeof(char) * 200);
  strcpy(sql_insert, "INSERT INTO user(username, passwd) VALUES(");
  strcat(sql_insert, "'");
  strcat(sql_insert, name);
  strcat(sql_insert, "', '");
  strcat(sql_insert, password);
  strcat(sql_insert, "')");
  cou


}

