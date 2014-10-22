
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


#ifndef _NGX_QUEUE_H_INCLUDED_
#define _NGX_QUEUE_H_INCLUDED_


typedef struct ngx_queue_s  ngx_queue_t;

struct ngx_queue_s {
    ngx_queue_t  *prev;      /*指向前一个节点*/
    ngx_queue_t  *next;     /*指向后一个节点*/
};

/*宏定义，就是一个队列, 在下面的命名的惯例一般是把队列命名为q，第一个元素命名为h*/
#define ngx_queue_init(q)                                                     \
    (q)->prev = q;                                                            \
    (q)->next = q


#define ngx_queue_empty(h)                                                    \
    (h == (h)->prev)  /*队列为空*/

/*在h节点后插入一个节点x*/
#define ngx_queue_insert_head(h, x)                                           \
    (x)->next = (h)->next;                                                    \
    (x)->next->prev = x;                                                      \
    (x)->prev = h;                                                            \
    (h)->next = x


#define ngx_queue_insert_after   ngx_queue_insert_head  /*宏定义*/

/*将元素节点插在队列后面*/
#define ngx_queue_insert_tail(h, x)                                           \
    (x)->prev = (h)->prev;                                                    \
    (x)->prev->next = x;                                                      \
    (x)->next = h;                                                            \
    (h)->prev = x

/*第一个节点*/
#define ngx_queue_head(h)                                                     \
    (h)->next

/*最后一个节点*/
#define ngx_queue_last(h)                                                     \
    (h)->prev

/*管理节点,也是代表我们的这个队列的节点*/
#define ngx_queue_sentinel(h)                                                 \
    (h)

/*q节点的下一个节点*/
#define ngx_queue_next(q)                                                     \
    (q)->next

/*q节点的上一个节点*/
#define ngx_queue_prev(q)                                                     \
    (q)->prev


#if (NGX_DEBUG)
/*非调试状态，(x)->prev=NULL;(x)->next=NULL都不执行，为了执行效率*/
#define ngx_queue_remove(x)                                                   \
    (x)->next->prev = (x)->prev;                                              \
    (x)->prev->next = (x)->next;                                              \
    (x)->prev = NULL;                                                         \
    (x)->next = NULL

#else

#define ngx_queue_remove(x)                                                   \
    (x)->next->prev = (x)->prev;                                              \
    (x)->prev->next = (x)->next

#endif

/*拆分队列 todo*/
#define ngx_queue_split(h, q, n)                                              \
    (n)->prev = (h)->prev;                                                    \
    (n)->prev->next = n;                                                      \
    (n)->next = q;                                                            \
    (h)->prev = (q)->prev;                                                    \
    (h)->prev->next = h;                                                      \
    (q)->prev = n;

/*队列的合并，将n的队列合并到h的队列末尾*/
#define ngx_queue_add(h, n)                                                   \
    (h)->prev->next = (n)->next;                                              \
    (n)->next->prev = (h)->prev;                                              \
    (h)->prev = (n)->prev;                                                    \
    (h)->prev->next = h;

/*非常重要的一个宏定义，能够将ngx_queue_t换算成节点结构体的地址*/
#define ngx_queue_data(q, type, link)                                         \
    (type *) ((u_char *) q - offsetof(type, link))


ngx_queue_t *ngx_queue_middle(ngx_queue_t *queue);/**/
void ngx_queue_sort(ngx_queue_t *queue,
    ngx_int_t (*cmp)(const ngx_queue_t *, const ngx_queue_t *));/*简单排序*/


#endif /* _NGX_QUEUE_H_INCLUDED_ */
