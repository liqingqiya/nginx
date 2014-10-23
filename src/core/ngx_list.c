
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


ngx_list_t *
ngx_list_create(ngx_pool_t *pool, ngx_uint_t n, size_t size)
{
    ngx_list_t  *list;

    list = ngx_palloc(pool, sizeof(ngx_list_t)); /*创建一个列表结构*/
    if (list == NULL) {
        return NULL;
    }

    if (ngx_list_init(list, pool, n, size) != NGX_OK) { /*初始化列表, 节点个数为n， 元素大小为size */
        return NULL;
    }

    return list;
}


void *
ngx_list_push(ngx_list_t *l)
{
    void             *elt; /*元素指针*/
    ngx_list_part_t  *last; 

    last = l->last; /*存储最后一个节点*/ 

    if (last->nelts == l->nalloc) { /*链表可用地址没有了,就分配一个新的链表节点 ngx_list_part_t*/

        /* the last part is full, allocate a new list part */

        last = ngx_palloc(l->pool, sizeof(ngx_list_part_t)); /*分配一个链表节点*/
        if (last == NULL) {
            return NULL;
        }

        last->elts = ngx_palloc(l->pool, l->nalloc * l->size); /*分配每一个节点所需要的存储数据的内存块*/
        if (last->elts == NULL) {
            return NULL;
        }
        /*初始化这个新分配的节点*/
        last->nelts = 0;
        last->next = NULL;

        l->last->next = last;
        l->last = last;
    }
    /*返回可用地址*/
    elt = (char *) last->elts + l->size * last->nelts; /**/
    last->nelts++; /*元素个数加一*/

    return elt; /*返回可用地址，之后紧接着在这个地址赋值就行了*/
}
