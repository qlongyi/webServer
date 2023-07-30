#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <dirent.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <map>
#include <mutex>

#include "../CGImysql/sql_connection_pool.h"
#include "../timer/lst_timer.h"
#include "../log/log.h"

class http_conn
{
public:
    static const int FILENAME_LEN = 200;
    static const int READ_BUFFER_SIZE = 4096;
    static const int WRITE_BUFFER_SIZE = 4096;
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
        PATH
    };
    enum CHECK_STATE
    {
        CHECK_STATE_REQUESTLINE = 0,
        CHECK_STATE_HEADER,
        CHECK_STATE_CONTENT
    };
    enum HTTP_CODE
    {
        NO_REQUEST,     //没有请求
        GET_REQUEST,    //get请求
        BAD_REQUEST,    //请求报文有误
        NO_RESOURCE,    //没有资源
        FORBIDDEN_REQUEST,  //权限不够
        FILE_REQUEST,       //文件请求
        DIR_REQUEST,        //文件夹请求
        INTERNAL_ERROR,     //未知错误
        CLOSED_CONNECTION   //关闭连接
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
    void init(int sockfd, const sockaddr_in &addr, char *, int, int, string user, string passwd, string sqlname);
    void close_conn(bool real_close = true);
    void process();
    bool read_once();
    bool write();
    sockaddr_in *get_address()
    {
        return &m_address;
    }
    void initmysql_result(connection_pool *connPool);
    int timer_flag; //为1则删除该timer
    int improv;     //reactor模式下，如果检测到缓存区有数据则返回，proactor模式则无


private:
    void init();
    HTTP_CODE process_read();
    bool process_write(HTTP_CODE ret);
    HTTP_CODE parse_request_line(char *text);
    HTTP_CODE parse_headers(char *text);
    HTTP_CODE parse_content(char *text);
    HTTP_CODE do_request();
    char *get_line() { return m_read_buf + m_start_line; };
    LINE_STATUS parse_line();
    const char* get_file_type(char* name);
    void unmap();
    bool add_response(const char *format, ...);
    bool add_content(const char *content);
    bool add_status_line(int status, const char *title);
    bool add_headers(int content_length, const char* file_type);
    bool add_content_type(const char* file_type);
    bool add_content_length(int content_length);
    bool add_linger();
    bool add_blank_line();

public:
    static int m_epollfd;   //epoll树的值
    static int m_user_count;
    MYSQL *mysql;   //所连接的数据库
    //int m_state;  //读为0, 写为1

private:
    int m_sockfd;
    sockaddr_in m_address;                  //地址结构
    char m_read_buf[READ_BUFFER_SIZE];      //读缓冲区
    long m_read_idx;                        //读缓冲区放入的数据长
    long m_checked_idx;                     //读缓冲区中数据读到的位置
    int m_start_line;                       //读缓冲区开始行
    char m_write_buf[WRITE_BUFFER_SIZE];    //写缓冲区
    int m_write_idx;                        //写缓冲区已用字节
    CHECK_STATE m_check_state;              //检查状态
    METHOD m_method;                        //请求行请求方式
    char m_real_file[FILENAME_LEN];
    char *m_url;                            //网址
    char *m_version;                        //版本
    char *m_host;                           //MySQL主机名
    long m_content_length;                  //内容长度
    bool m_linger;                          //完成一次通信后是否保持连接                  
    char *m_file_address;                   //文件地址
    struct stat m_file_stat;    //获取文件属性
    struct iovec m_iv[2];   //保留写缓冲区和所读取文件的地址和长度
    int m_iv_count;    //缓冲区个数
    int cgi;        //是否启用的POST
    char *m_string; //存储请求头数据
    int bytes_to_send;  //需要发送的字节
    int bytes_have_send;
    char *doc_root;

    map<string, string> m_users;
    int m_TRIGMode;
    int m_close_log;

    char sql_user[100];
    char sql_passwd[100];
    char sql_name[100];
};

void encode_str(char *to, int tosize, const char *from);
#endif
