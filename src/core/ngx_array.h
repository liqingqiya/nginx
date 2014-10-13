
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_ARRAY_H_INCLUDED_      /*防止头文件多次导入的差错*/
#define _NGX_ARRAY_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef struct {
    void        *elts;      /*数组数据区域*/
    ngx_uint_t   nelts;     /*element个数*/
    size_t       size;      /*一个元素内存的字节大小*/
    ngx_uint_t   nalloc;    /*数组容量*/
    ngx_pool_t  *pool;      /*数组所在的内存池*/
} ngx_array_t;              /*数组管理器，就代表一个数组*/


ngx_array_t *ngx_array_create(ngx_pool_t *p, ngx_uint_t n, size_t size);
void ngx_array_destroy(ngx_array_t *a);
void *ngx_array_push(ngx_array_t *a);
void *ngx_array_push_n(ngx_array_t *a, ngx_uint_t n);


static ngx_inline ngx_int_t
ngx_array_init(ngx_array_t *array, ngx_pool_t *pool, ngx_uint_t n, size_t size)     /*模块初始化函数放在模块头文件里，且声明为static，具有内链接特性。*/
{
    /*
     * set "array->nelts" before "array->elts", otherwise MSVC thinks
     * that "array->nelts" may be used without having been initialized
     */

    array->nelts = 0;        /*元素个数*/
    array->size = size;     /*单个节点的内存大小*/
    array->nalloc = n;      /*数组容量*/
    array->pool = pool;     /*内存池*/

    array->elts = ngx_palloc(pool, n * size);   /*分配内存空间*/
    if (array->elts == NULL) {
        return NGX_ERROR;
    }

    return NGX_OK;
}


#endif /* _NGX_ARRAY_H_INCLUDED_ */
