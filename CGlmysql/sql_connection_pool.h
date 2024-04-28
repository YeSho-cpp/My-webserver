//
// Created by YeSho on 2024/4/15.
//

#ifndef WEBSERVER_SQL_CONNECTION_POOL_H
#define WEBSERVER_SQL_CONNECTION_POOL_H
#include "../log/log.h"
#include "../lock/lock.h"
#include <list>
#include <mysql/mysql.h>

using namespace std;
class connection_pool{

public:
  void init(string url,int port, string user,string password,string dbName,int close_log,int max_con);
  MYSQL* getConnection();
  bool releaseConnection(MYSQL *con);
  static connection_pool* GetInstance();
  void DestroyPool();

  [[nodiscard]] int getFreeCon() const;
  [[nodiscard]] int getCurCon() const;

public:
  string m_url;
  string m_user;
  string m_port;
  string m_password;
  string m_databaseName;
  int m_close_log{};
private:
  int m_free_con; // 空闲连接数
  int m_cur_con; // 正在使用的连接数
  int m_max_con; // 最大连接数
  list<MYSQL *>connList;
  locker lock;
  sem reserve{};
  connection_pool();
  ~connection_pool();
};

class connection_poolRAII{
public:
  connection_poolRAII(MYSQL **SQL,connection_pool *connPool);
  ~connection_poolRAII();
private:
  MYSQL *connRAII;
  connection_pool *poolRAII;
};

#endif//WEBSERVER_SQL_CONNECTION_POOL_H
