
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


/*
 * find the middle queue element if the queue has odd number of elements
 * or the first element of the queue's second part otherwise
 */

ngx_queue_t *
ngx_queue_middle(ngx_queue_t *queue)
{
    ngx_queue_t  *middle, *next;

    middle = ngx_queue_head(queue);

    if (middle == ngx_queue_last(queue)) {
        return middle;
    }

    next = ngx_queue_head(queue);

    for ( ;; ) {
        middle = ngx_queue_next(middle);

        next = ngx_queue_next(next);

        if (next == ngx_queue_last(queue)) {
            return middle;
        }

        next = ngx_queue_next(next);

        if (next == ngx_queue_last(queue)) {
            return middle;
        }
    }
}


/* the stable insertion sort */

void
ngx_queue_sort(ngx_queue_t *queue,
    ngx_int_t (*cmp)(const ngx_queue_t *, const ngx_queue_t *)) /*双端链表的插入排序*/
{
    ngx_queue_t  *q, *prev, *next;

    q = ngx_queue_head(queue);  /*记录头*/

    if (q == ngx_queue_last(queue)) { /*为空，则退出*/
        return;
    }
    /*ngx_queue_next(q)得到下一个元素节点*/
    for (q = ngx_queue_next(q); q != ngx_queue_sentinel(queue); q = next) {

        prev = ngx_queue_prev(q); /*取q的上一个节点*/
        next = ngx_queue_next(q); /*取q的下一个节点*/

        ngx_queue_remove(q);  /*先将q节点从链表中移除出来*/
        /*该do-while循环找到节点插入的位置，用prev记录，然后把q插入到这个节点的后面.*/
        do {
            if (cmp(prev, q) <= 0) { /*小于, 排序结果是递增*/
                break;
            }

            prev = ngx_queue_prev(prev);  /*prev其一个元素*/

        } while (prev != ngx_queue_sentinel(queue));

        ngx_queue_insert_after(prev, q);
    }
}
