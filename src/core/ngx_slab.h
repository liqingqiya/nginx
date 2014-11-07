
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_SLAB_H_INCLUDED_
#define _NGX_SLAB_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef struct ngx_slab_page_s  ngx_slab_page_t;

struct ngx_slab_page_s {
    uintptr_t         slab;
    ngx_slab_page_t  *next;
    uintptr_t         prev;
};


typedef struct {
    ngx_shmtx_sh_t    lock;

    size_t            min_size; /**/
    size_t            min_shift;

    ngx_slab_page_t  *pages; /*管理页数组*/
    ngx_slab_page_t   free;    /*管理free的页数组*/

    u_char           *start;    /*数据区的起始地址*/
    u_char           *end;    /*数据区的结束地址*/

    ngx_shmtx_t       mutex;    /*互斥锁*/

    u_char           *log_ctx;    /*todo*/
    u_char            zero;    /*todo*/

    unsigned          log_nomem:1;    /*todo*/

    void             *data;    /*todo*/
    void             *addr;    /*todo*/
} ngx_slab_pool_t;


void ngx_slab_init(ngx_slab_pool_t *pool);
void *ngx_slab_alloc(ngx_slab_pool_t *pool, size_t size);
void *ngx_slab_alloc_locked(ngx_slab_pool_t *pool, size_t size);
void ngx_slab_free(ngx_slab_pool_t *pool, void *p);
void ngx_slab_free_locked(ngx_slab_pool_t *pool, void *p);


#endif /* _NGX_SLAB_H_INCLUDED_ */
