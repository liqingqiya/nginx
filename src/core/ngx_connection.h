
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_CONNECTION_H_INCLUDED_
#define _NGX_CONNECTION_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef struct ngx_listening_s  ngx_listening_t;    /*代表一个监听端口*/

struct ngx_listening_s {
    ngx_socket_t        fd;     /*套接字句柄*/

    struct sockaddr    *sockaddr;   /*套接字地址*/
    socklen_t           socklen;    /* size of sockaddr */
    size_t              addr_text_max_len;/*存储IP地址的字符串addr_text最大长度，即它指定了addr_text所分配的内存大小*/
    ngx_str_t           addr_text;  /*以字符串类型存储ip地址*/

    int                 type;       /*套接字类型，比如SOCK_STREAM代表TCP*/

    int                 backlog;/*TCP实现监听时的backlog队列，表示允许正在通过三次握手建立TCP连接，但还没有任何进程开始处理的连接的最大个数*/
    int                 rcvbuf;     /*内核中对于这个套接字的接收缓冲区的大小*/
    int                 sndbuf;     /*内核中对于这个套接字的发送缓冲区的大小*/
#if (NGX_HAVE_KEEPALIVE_TUNABLE)
    int                 keepidle; /*长连接*/
    int                 keepintvl;
    int                 keepcnt;
#endif

    /* handler of accepted connection */
    ngx_connection_handler_pt   handler;    /*当新的TCP连接成功建立后的处理方法*/

    void               *servers;  /* array of ngx_http_in_addr_t, for example */

    ngx_log_t           log;
    ngx_log_t          *logp;

    size_t              pool_size;  /*如果新的TCP 连接创建内存池，初始大小*/
    /* should be here because of the AcceptEx() preread */
    size_t              post_accept_buffer_size;
    /* should be here because of the deferred accept */
    ngx_msec_t          post_accept_timeout;/*如果post_accept_timeout秒后，仍然没有收到用户的数据，则内核直接丢弃连接*/

    ngx_listening_t    *previous;/*多个ngx_listenging_t结构体之间由previous指针组成单链表*/
    ngx_connection_t   *connection; /*当前监听句柄对应着的 ngx_connection_t 结构体*/

    unsigned            open:1; /*标志位，1表示当前监听有效,且执行ngx_init_cycle时，不关闭监听端口，为0时则正常关闭*/
    unsigned            remain:1;/*标志位，1表示使用已有的ngx_cycle_t来初始化新的ngx_cycle_t结构体时，不关闭原先打开的监听端口，这个对运行时升级有效，remain为0时，表示正常关闭曾经打开的监听端口。*/
    unsigned            ignore:1;/*标志位，1表示跳过设置当前ngx_listening_t结构体的套接字，为0时，正常初始化套接字*/

    unsigned            bound:1;       /* already bound */
    unsigned            inherited:1;   /* inherited from previous process */
    unsigned            nonblocking_accept:1;
    unsigned            listen:1;/*为1表示当前套接字已经监听*/
    unsigned            nonblocking:1;
    unsigned            shared:1;    /* shared between threads or processes */
    unsigned            addr_ntop:1;

#if (NGX_HAVE_INET6 && defined IPV6_V6ONLY)
    unsigned            ipv6only:1;
#endif
    unsigned            keepalive:2;

#if (NGX_HAVE_DEFERRED_ACCEPT)
    unsigned            deferred_accept:1; /*todo*/
    unsigned            delete_deferred:1; /*todo*/
    unsigned            add_deferred:1;
#ifdef SO_ACCEPTFILTER
    char               *accept_filter;
#endif
#endif
#if (NGX_HAVE_SETFIB)
    int                 setfib;
#endif

#if (NGX_HAVE_TCP_FASTOPEN)
    int                 fastopen;
#endif

};


typedef enum {
     NGX_ERROR_ALERT = 0,
     NGX_ERROR_ERR,
     NGX_ERROR_INFO,
     NGX_ERROR_IGNORE_ECONNRESET,
     NGX_ERROR_IGNORE_EINVAL
} ngx_connection_log_error_e;


typedef enum {
     NGX_TCP_NODELAY_UNSET = 0,
     NGX_TCP_NODELAY_SET,
     NGX_TCP_NODELAY_DISABLED
} ngx_connection_tcp_nodelay_e;


typedef enum {
     NGX_TCP_NOPUSH_UNSET = 0,
     NGX_TCP_NOPUSH_SET,
     NGX_TCP_NOPUSH_DISABLED
} ngx_connection_tcp_nopush_e;


#define NGX_LOWLEVEL_BUFFERED  0x0f
#define NGX_SSL_BUFFERED       0x01
#define NGX_SPDY_BUFFERED      0x02


struct ngx_connection_s {
    void               *data; /*连接未使用时，data成员用于充当连接池中空闲连接表中的next指针。当连接使用时，data意义由使用它的nginx模块而定，如在http框架中，data指向ngx_http_request_t请求*/
    ngx_event_t        *read; /*对应的读事件*/
    ngx_event_t        *write; /*对应的写事件*/

    ngx_socket_t        fd; /*套接字句柄*/

    ngx_recv_pt         recv;/*直接接收网络字符流的方法*/
    ngx_send_pt         send;/*直接发送网络字符流的方法*/
    ngx_recv_chain_pt   recv_chain;/*以ngx_chain_t链表为参数来接收网络字符流的方法*/
    ngx_send_chain_pt   send_chain;/*以ngx_chain_t链表为参数来发送网络字符流的方法*/

    ngx_listening_t    *listening;/*这个连接对应的ngx_listening_t监听对象，此连接由listening监听端口的事件建立*/

    off_t               sent;/*这个连接上已经发送出去的字节数*/

    ngx_log_t          *log;

    ngx_pool_t         *pool;

    struct sockaddr    *sockaddr; /*连接客户端的sockaddr结构*/
    socklen_t           socklen;/*sockaddr结构体长度*/
    ngx_str_t           addr_text; /*字符串形式的IP地址*/

    ngx_str_t           proxy_protocol_addr;

#if (NGX_SSL)
    ngx_ssl_connection_t  *ssl;
#endif

    struct sockaddr    *local_sockaddr;
    socklen_t           local_socklen;

    ngx_buf_t          *buffer; /*用于接收缓存客户端发来的字符流，每个事件消费模块可以自由决定从连接池中分配多大的空间给buffer这个接收缓存字段。*/

    ngx_queue_t         queue; /*该字段用于将当前以双向链表元素的形式添加到 ngx_cycle_t 核心结构体的 reusable_connections_queue 双向链表中，表示可以重用的连接*/

    ngx_atomic_uint_t   number;/*连接使用次数*/

    ngx_uint_t          requests;/*处理的请求次数*/

    unsigned            buffered:8;

    unsigned            log_error:3;     /* ngx_connection_log_error_e */

    unsigned            unexpected_eof:1;
    unsigned            timedout:1;
    unsigned            error:1;
    unsigned            destroyed:1;

    unsigned            idle:1; /*标志位，为1时表示处于空闲状态*/
    unsigned            reusable:1;/*标志位，为1时表示连接可重用，与上面的queue字段对应*/
    unsigned            close:1;/*为1时表示连接关闭*/

    unsigned            sendfile:1;/*为1时表示正在将文件中的数据发往连接的另一端*/
    unsigned            sndlowat:1;/*为1时表示只有在连接套接字对应的发送缓冲区必须满足最低设置的大小阀值，事件驱动模型才会分发该事件*/
    unsigned            tcp_nodelay:2;   /* ngx_connection_tcp_nodelay_e */
    unsigned            tcp_nopush:2;    /* ngx_connection_tcp_nopush_e */

    unsigned            need_last_buf:1;

#if (NGX_HAVE_IOCP)
    unsigned            accept_context_updated:1;
#endif

#if (NGX_HAVE_AIO_SENDFILE)
    unsigned            aio_sendfile:1; /*为1时表示使用异步I/O的方式将磁盘上文件发送给网络连接的另一端*/
    unsigned            busy_count:2;
    ngx_buf_t          *busy_sendfile;/*使用异步I/O方式发送的文件，busy_sendfile缓冲区保存待发送文件的信息*/
#endif

#if (NGX_THREADS)
    ngx_atomic_t        lock;
#endif
};      /*对应一个socket连接*/


ngx_listening_t *ngx_create_listening(ngx_conf_t *cf, void *sockaddr,
    socklen_t socklen);
ngx_int_t ngx_set_inherited_sockets(ngx_cycle_t *cycle);
ngx_int_t ngx_open_listening_sockets(ngx_cycle_t *cycle);
void ngx_configure_listening_sockets(ngx_cycle_t *cycle);
void ngx_close_listening_sockets(ngx_cycle_t *cycle);
void ngx_close_connection(ngx_connection_t *c);
ngx_int_t ngx_connection_local_sockaddr(ngx_connection_t *c, ngx_str_t *s,
    ngx_uint_t port);
ngx_int_t ngx_connection_error(ngx_connection_t *c, ngx_err_t err, char *text);

ngx_connection_t *ngx_get_connection(ngx_socket_t s, ngx_log_t *log);
void ngx_free_connection(ngx_connection_t *c);

void ngx_reusable_connection(ngx_connection_t *c, ngx_uint_t reusable);

#endif /* _NGX_CONNECTION_H_INCLUDED_ */
