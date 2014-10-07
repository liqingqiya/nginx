
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_HTTP_UPSTREAM_ROUND_ROBIN_H_INCLUDED_      /*条件编译，保护当该头文件被多个文件导入的时候可能重复导入的情景*/
#define _NGX_HTTP_UPSTREAM_ROUND_ROBIN_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


typedef struct {
    struct sockaddr                *sockaddr;
    socklen_t                       socklen;
    ngx_str_t                       name;

    ngx_int_t                       current_weight;     /*当前权重????*/
    ngx_int_t                       effective_weight;   /*有效权重????*/
    ngx_int_t                       weight;             /* 初始权重，默认为1,与加权轮询配合使用,当所有的服务器current_weight小于0时候，那么就会使用该值来进行重新初始化*/

    ngx_uint_t                      fails;              /*失败次数*/
    time_t                          accessed;
    time_t                          checked;

    ngx_uint_t                      max_fails;          /*最大失败次数*/
    time_t                          fail_timeout;       /*与max_fails配合使用，默认值为10s，如果某服务器在fail_timeout时间内，失败达到max_fails，那么就在fail_timeout时间内，是不能参与被选择的。*/

    ngx_uint_t                      down;          /* 主动标记为宕机状态，不参与被选择 unsigned  down:1; */

#if (NGX_HTTP_SSL)
    ngx_ssl_session_t              *ssl_session;   /* local to a process */
#endif
} ngx_http_upstream_rr_peer_t;       /*该结构体对应一个后端服务器*/               


typedef struct ngx_http_upstream_rr_peers_s  ngx_http_upstream_rr_peers_t;

struct ngx_http_upstream_rr_peers_s {
    ngx_uint_t                      number;         /*非后备服务器列表的个数*/

 /* ngx_mutex_t                    *mutex; */

    ngx_uint_t                      total_weight;

    unsigned                        single:1;
    unsigned                        weighted:1;

    ngx_str_t                      *name;

    ngx_http_upstream_rr_peers_t   *next;     /*指向后备服务器列表的指针*/

    ngx_http_upstream_rr_peer_t     peer[1]; /*不懂这里的意思？？？？*/
};          /*该结构体对应一个非后备服务器列表，这里实现为一个链表结构*/


typedef struct {
    ngx_http_upstream_rr_peers_t   *peers;
    ngx_uint_t                      current;
    uintptr_t                      *tried;
    uintptr_t                       data;
} ngx_http_upstream_rr_peer_data_t;     /*？？？*/


ngx_int_t ngx_http_upstream_init_round_robin(ngx_conf_t *cf,
    ngx_http_upstream_srv_conf_t *us);/*初始化加权轮询*/
ngx_int_t ngx_http_upstream_init_round_robin_peer(ngx_http_request_t *r,
    ngx_http_upstream_srv_conf_t *us);/*初始化加权轮询的服务器结构*/
ngx_int_t ngx_http_upstream_create_round_robin_peer(ngx_http_request_t *r,
    ngx_http_upstream_resolved_t *ur);/*创建服务器？？？*/
ngx_int_t ngx_http_upstream_get_round_robin_peer(ngx_peer_connection_t *pc,
    void *data);/*选取服务器？？？*/
void ngx_http_upstream_free_round_robin_peer(ngx_peer_connection_t *pc,
    void *data, ngx_uint_t state);/*释放某个服务器*/

/*下面两个函数是运行在ssl层上的时候，才会用到，我们暂时不管*/
#if (NGX_HTTP_SSL)
ngx_int_t
    ngx_http_upstream_set_round_robin_peer_session(ngx_peer_connection_t *pc,
    void *data);
void ngx_http_upstream_save_round_robin_peer_session(ngx_peer_connection_t *pc,
    void *data);
#endif


#endif /* _NGX_HTTP_UPSTREAM_ROUND_ROBIN_H_INCLUDED_ */
