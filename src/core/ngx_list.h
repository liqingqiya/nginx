
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_LIST_H_INCLUDED_
#define _NGX_LIST_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef struct ngx_list_part_s  ngx_list_part_t;

struct ngx_list_part_s {
    void             *elts;     /*void*类型，？？*/
    ngx_uint_t        nelts;    /*元素个数？？？*/
    ngx_list_part_t  *next;     /*指向下一个结构*/
};


typedef struct {
    ngx_list_part_t  *last;     /*指向最后一个节点*/
    ngx_list_part_t   part;     /*我们的节点*/
    size_t            size;      /*链表大小*/
    ngx_uint_t        nalloc;   /*分配的内存大小*/
    ngx_pool_t       *pool;     /*为什么加上这个？ 内存池*/
} ngx_list_t;       /*链表结构头*/


ngx_list_t *ngx_list_create(ngx_pool_t *pool, ngx_uint_t n, size_t size);   /*创建链表*/

static ngx_inline ngx_int_t
ngx_list_init(ngx_list_t *list, ngx_pool_t *pool, ngx_uint_t n, size_t size)    /*初始化链表, nginx通常把初始化的工作放到了.h文件，并且限定为文件作用域,内链接*/
{
    list->part.elts = ngx_palloc(pool, n * size);
    if (list->part.elts == NULL) {
        return NGX_ERROR;
    }

    list->part.nelts = 0;       /*节点个数*/
    list->part.next = NULL;     /*part指向的下一个节点*/
    list->last = &list->part;   /*指向最后一个节点*/
    list->size = size;  /*每一个节点大小*/
    list->nalloc = n;   /*节点个数*/
    list->pool = pool;  /*内存池*/

    return NGX_OK;
}


/*
 *
 *  the iteration through the list:
 *
 *  part = &list.part;
 *  data = part->elts;
 *
 *  for (i = 0 ;; i++) {
 *
 *      if (i >= part->nelts) {
 *          if (part->next == NULL) {
 *              break;
 *          }
 *
 *          part = part->next;
 *          data = part->elts;
 *          i = 0;
 *      }
 *
 *      ...  data[i] ...
 *
 *  }
 */


void *ngx_list_push(ngx_list_t *list);


#endif /* _NGX_LIST_H_INCLUDED_ */
