
#include <iostream>
#include <csignal>
#include "CGlmysql/sql_connection_pool.h"
#include <thread>
using namespace std;



int main(int argc,char *argv[]) {
  cout<<std::thread::hardware_concurrency()<<endl;
}

