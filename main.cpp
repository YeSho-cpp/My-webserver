
#include <iostream>
#include "CGlmysql/sql_connection_pool.h"
using namespace std;


int main(int argc,char *argv[]) {

  // 初始化连接池
  connection_pool* pool = connection_pool::GetInstance();
  pool->init("localhost", 3306, "root", "701121", "WebTest", 0, 5);

  // 获取一个数据库连接
  MYSQL *conn = pool->getConnection();
  if (conn == nullptr) {
    cout << "Failed to get a connection" << endl;
    return 1;
  } else {
    cout << "Successfully obtained a connection" << endl;
  }

  cout<<pool->getCurCon()<<" "<<pool->getFreeCon()<<endl;

  // 使用获取的连接执行查询
  if (mysql_query(conn, "SELECT VERSION()")) {
    fprintf(stderr, "%s\n", mysql_error(conn));
  } else {
    MYSQL_RES *result = mysql_store_result(conn);
    if (result) {
      MYSQL_ROW row;
      while ((row = mysql_fetch_row(result))) {
        cout << "MySQL Server Version: " << row[0] << endl;
      }
      mysql_free_result(result);
    }
  }

  // 释放连接
  if (pool->releaseConnection(conn)) {
    cout << "Connection returned to the pool" << endl;
  } else {
    cout << "Failed to return connection to the pool" << endl;
  }

  // 销毁连接池
  pool->DestroyPool();

  return 0;
}

