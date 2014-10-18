
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_PALLOC_H_INCLUDED_
#define _NGX_PALLOC_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


/*
 * NGX_MAX_ALLOC_FROM_POOL should be (ngx_pagesize - 1), i.e. 4095 on x86.
 * On Windows NT it decreases a number of locked pages in a kernel.
 */
#define NGX_MAX_ALLOC_FROM_POOL  (ngx_pagesize - 1)

#define NGX_DEFAULT_POOL_SIZE    (16 * 1024)

#define NGX_POOL_ALIGNMENT       16
#define NGX_MIN_POOL_SIZE                                                     \
    ngx_align((sizeof(ngx_pool_t) + 2 * sizeof(ngx_pool_large_t)),            \
              NGX_POOL_ALIGNMENT)


typedef void (*ngx_pool_cleanup_pt)(void *data);

typedef struct ngx_pool_cleanup_s  ngx_pool_cleanup_t;

struct ngx_pool_cleanup_s {
    ngx_pool_cleanup_pt   handler;
    void                 *data;
    ngx_pool_cleanup_t   *next;
};


typedef struct ngx_pool_large_s  ngx_pool_large_t;

struct ngx_pool_large_s {
    ngx_pool_large_t     *next;
    void                 *alloc;
};      /*大内存节点, 当一个申请的内存空间大小比内存池的大小还要大的时候，malloc一块大的空间，再内存池用保留这个地址的指针*/


typedef struct {
    u_char               *last;     /*数据区域的结束地址*/
    u_char               *end;      /*内存池的结束地址*/
    ngx_pool_t           *next;     /*指向下一个内存池节点*/
    ngx_uint_t            failed;   /*标志申请内存的时候失败的次数*/
} ngx_pool_data_t;      /*内存数据区域*/


struct ngx_pool_s {
    ngx_pool_data_t       d;    /*内存池数据结构*/
    size_t                max;  /*最大数据区大小*/
    ngx_pool_t           *current;  /*指向当前的内存池结构*/
    ngx_chain_t          *chain;    /*todo*/
    ngx_pool_large_t     *large;    /*携带的大存储块，不超过3个*/
    ngx_pool_cleanup_t   *cleanup;
    ngx_log_t            *log;
}; /*内存池管理结构*/


typedef struct {
    ngx_fd_t              fd;
    u_char               *name;
    ngx_log_t            *log;
} ngx_pool_cleanup_file_t;


void *ngx_alloc(size_t size, ngx_log_t *log);
void *ngx_calloc(size_t size, ngx_log_t *log);

ngx_pool_t *ngx_create_pool(size_t size, ngx_log_t *log);       /*创建内存池*/
void ngx_destroy_pool(ngx_pool_t *pool);                          /*销毁内存池*/  
void ngx_reset_pool(ngx_pool_t *pool);

void *ngx_palloc(ngx_pool_t *pool, size_t size);                /*从内存池中分配内存*/
void *ngx_pnalloc(ngx_pool_t *pool, size_t size);
void *ngx_pcalloc(ngx_pool_t *pool, size_t size);
void *ngx_pmemalign(ngx_pool_t *pool, size_t size, size_t alignment);
ngx_int_t ngx_pfree(ngx_pool_t *pool, void *p);


ngx_pool_cleanup_t *ngx_pool_cleanup_add(ngx_pool_t *p, size_t size);
void ngx_pool_run_cleanup_file(ngx_pool_t *p, ngx_fd_t fd);
void ngx_pool_cleanup_file(void *data);
void ngx_pool_delete_file(void *data);


#endif /* _NGX_PALLOC_H_INCLUDED_ */
