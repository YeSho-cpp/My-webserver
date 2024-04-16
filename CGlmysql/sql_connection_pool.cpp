//
// Created by YeSho on 2024/4/15.
//
#include "sql_connection_pool.h"
#include <mysql/mysql.h>


connection_pool::connection_pool() {
  m_free_con=0;
  m_cur_con=0;

}
void connection_pool::init(string url, int port, string user, string password, string dbName, int close_log,int max_con) {

  m_url=std::move(url);
  m_port= to_string(port);
  m_user=std::move(user);
  m_password=std::move(password);
  m_databaseName=std::move(dbName);
  m_close_log=close_log;

  for(int i=0;i<max_con;i++){
    MYSQL *con= nullptr;

    con=mysql_init(con);

    if(con== nullptr){
      LOG_ERROR("MySQL Error");
    }

    con=mysql_real_connect(con,m_url.c_str(),m_user.c_str(),m_password.c_str(),m_databaseName.c_str(),port, nullptr,0);
    if(con== nullptr){
      LOG_ERROR("MySQL Error");
    }
    connList.push_back(con);
    ++m_free_con;
  }
  m_max_con=m_free_con;

  sem_init(&reserve,0,max_con);

}
connection_pool *connection_pool::GetInstance() {

  static connection_pool instance;
  return &instance;
}
void connection_pool::DestroyPool() {
  lock.lock();
  if(!connList.empty()){
    for(auto con : connList){
      mysql_close(con);
    }
    m_free_con=0;
    m_cur_con=0;
    connList.clear();
  }

  lock.unlock();
}
connection_pool::~connection_pool() {
  DestroyPool();
}
MYSQL *connection_pool::getConnection() {
  if(connList.empty()) return nullptr;
  sem_wait(&reserve);
  lock.lock();

  MYSQL *con= nullptr;
  con=connList.front();
  connList.pop_front();
  --m_free_con;
  ++m_cur_con;
  lock.unlock();
  return con;
}
bool connection_pool::releaseConnection(MYSQL *con) {

  if(con== nullptr) return false;

  lock.lock();
  connList.push_back(con);
  ++m_free_con;
  --m_cur_con;
  lock.unlock();
  sem_post(&reserve);
  return true;
}
int connection_pool::getFreeCon() const {
  return m_free_con;
}
int connection_pool::getCurCon() const {
  return m_cur_con;
}

connection_poolRAII::connection_poolRAII(MYSQL **SQL, connection_pool *connPool) {
  *SQL=connPool->getConnection();
  connRAII=*SQL;
  poolRAII=connPool;
}
connection_poolRAII::~connection_poolRAII() {
  poolRAII->releaseConnection(connRAII);
}
