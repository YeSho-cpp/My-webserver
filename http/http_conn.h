//
// Created by YeSho on 2024/4/17.
//

#ifndef WEBSERVER_HTTP_CONN_H
#define WEBSERVER_HTTP_CONN_H
#include "../CGlmysql/sql_connection_pool.h"
#include "../lock/lock.h"
#include "../log/log.h"
#include "../threadpool/threadpool.h"
#include <netinet/in.h>
#include <sys/epoll.h>
#include <map>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/uio.h>

class http_conn{
public:
  static const int FILENAME_LEN = 200;
  static const int READ_BUFFER_SIZE = 2048;
  static const int WRITE_BUFFER_SIZE = 1024;
  enum METHOD
  {
    GET = 0,
    POST,
    HEAD,
    PUT,
    DELETE,
    TRACE,
    OPTIONS,
    CONNECT,
    PATCH
  };
  enum CHECK_STATE
  {
    CHECK_STATE_REQUESTLINE = 0, // 当前正在分析请求行
    CHECK_STATE_HEADER, // 当前正在分析请求头
    CHECK_STATE_CONTENT // 当前正在分析请求体
  };
  enum HTTP_CODE
  {
    NO_REQUEST, // 表示请求不完整，需要继续读取客户数据  这是解析请求的不同阶段的默认返回值
    GET_REQUEST, // 表示获得了一个完整的客户请求  只有这个完成后我们才能进行
    BAD_REQUEST, // 表示客户请求有语法错误
    NO_RESOURCE, // 请求的资源不存在
    FORBIDDEN_REQUEST, // 表示客户对资源没有足够的访问权限
    FILE_REQUEST, // 这是一个文件请求
    INTERNAL_ERROR, // 表示服务器内部错误；
    CLOSED_CONNECTION // 表示客户端已经关闭连接了
  };
  enum LINE_STATUS
  {
    LINE_OK = 0,
    LINE_BAD,
    LINE_OPEN
  };

public:
  http_conn() {}
  ~http_conn() {}

public:
  void init(int sockfd, const sockaddr_in &addr, char *root, int TRIGMode,int close_log, string user, string passwd, string sqlname);
  void close_conn(bool real_close=true);
  void process();
  bool read_once();
  bool write();
  sockaddr_in *get_address(){
    return &m_address;
  }

  void initmysql_result(connection_pool *connPool);

  int timer_flag;
  int improv;

private:
  void unmap();
  void reset();
  LINE_STATUS parse_line();
  HTTP_CODE parse_request_line(char *text);
  HTTP_CODE parse_headers(char *text);
  HTTP_CODE parse_content(char *text);
  HTTP_CODE process_read();
  HTTP_CODE do_request();
  char *get_line() {return m_read_buf+m_start_line;};
  bool add_response(const char *format, ...);
  bool add_content(const char *content);
  bool add_status_line(int status, const char *title);
  bool add_headers(int content_length);
  bool add_content_type();
  bool add_content_length(int content_length);
  bool add_linger();
  bool add_blank_line();
  bool process_write(HTTP_CODE ret);
public:
  static int m_epoll_fd;
  static int m_user_count;
  MYSQL *mysql;
  int m_state; // 0 读 1 写
private:
  int m_sock_fd;
  sockaddr_in m_address;
  int m_TRIGMode; // 触发模式
  char *m_file_address; // 请求文件被mmap到内存中的位置
  char m_read_buf[READ_BUFFER_SIZE];
  int m_read_idx;
  int m_check_idx;
  char m_write_buf[WRITE_BUFFER_SIZE];
  int m_write_idx;
  int m_start_line;
  char m_real_file[FILENAME_LEN];
  char* doc_root; // 服务器上用于存放网页文件的根目录的路径
  CHECK_STATE m_check_state;
  METHOD m_method;
  char *m_url;
  char *m_version;
  char *m_host;
  struct stat m_file_stat; // 目标文件的状态信息
  struct iovec m_iv[2]; // 用于writev操作的结构体数组。
  int m_iv_count; // 被用于输出的iovec结构体数量
  int cgi; //是否启用的POST
  char* m_string; //存储请求体数据
  int m_close_log;
  char sql_user[100];
  char sql_password[100];
  char sql_name[100];
  int bytes_to_send;
  int bytes_has_send;
  // 用户信息和数据库配置
  map<string,string>m_users;
  bool m_linger; // 是否保持连接
  long m_content_length;
};


#endif//WEBSERVER_HTTP_CONN_H
