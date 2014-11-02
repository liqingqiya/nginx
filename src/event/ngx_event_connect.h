
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_EVENT_CONNECT_H_INCLUDED_
#define _NGX_EVENT_CONNECT_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>


#define NGX_PEER_KEEPALIVE           1
#define NGX_PEER_NEXT                2
#define NGX_PEER_FAILED              4


typedef struct ngx_peer_connection_s  ngx_peer_connection_t;

typedef ngx_int_t (*ngx_event_get_peer_pt)(ngx_peer_connection_t *pc,
    void *data); /*使用长连接与上游服务器通信时， 可以通过该方法由连接池中获取一个新的连接*/
typedef void (*ngx_event_free_peer_pt)(ngx_peer_connection_t *pc, void *data,
    ngx_uint_t state); /*使用长连接与上游服务器通信时，通过该方法将使用完毕的连接释放给连接池*/
#if (NGX_SSL)

typedef ngx_int_t (*ngx_event_set_peer_session_pt)(ngx_peer_connection_t *pc,
    void *data);
typedef void (*ngx_event_save_peer_session_pt)(ngx_peer_connection_t *pc,
    void *data);
#endif


struct ngx_peer_connection_s { /*表示一个主动连接，用于nginx和上游服务器建立的连接*/
    ngx_connection_t                *connection; /*一个主动连接实际也是需要ngx_connection_t结构中的大部分成员的，处于重用的考虑而定义了一个connection成员*/

    struct sockaddr                 *sockaddr;/*上游服务器socket地址*/
    socklen_t                        socklen;/*sockaddr地址长度*/
    ngx_str_t                       *name;/*远端服务器的名称*/

    ngx_uint_t                       tries;/*表示在连接一个远端服务器时，当前出现异常失败后可以重试的次数*/

    ngx_event_get_peer_pt            get;/*获取连接的方法*/
    ngx_event_free_peer_pt           free;/*释放连接的方法*/
    void                            *data;/*这个data指针仅用于和上面的get，free方法配合传递的参数，具体含义与实现的get，free方法的模块相关*/

#if (NGX_SSL)
    ngx_event_set_peer_session_pt    set_session;/**/
    ngx_event_save_peer_session_pt   save_session;/**/
#endif

#if (NGX_THREADS)
    ngx_atomic_t                    *lock;/**/
#endif

    ngx_addr_t                      *local;/*本机地址信息*/

    int                              rcvbuf;/*套接字的接收缓存区大小*/

    ngx_log_t                       *log;/**/

    unsigned                         cached:1;/*为1时，表示上面的connection连接已经缓存*/

                                     /* ngx_connection_log_error_e */
    unsigned                         log_error:2;/**/
};


ngx_int_t ngx_event_connect_peer(ngx_peer_connection_t *pc);
ngx_int_t ngx_event_get_peer(ngx_peer_connection_t *pc, void *data);


#endif /* _NGX_EVENT_CONNECT_H_INCLUDED_ */
