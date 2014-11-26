
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>


#if (NGX_THREADS)
ngx_mutex_t  *ngx_event_timer_mutex;
#endif


ngx_thread_volatile ngx_rbtree_t  ngx_event_timer_rbtree;       /*全局变量, 定时器红黑树*/
static ngx_rbtree_node_t          ngx_event_timer_sentinel;      /*全局变量, 哨兵*/

/*
 * the event timer rbtree may contain the duplicate keys, however,
 * it should not be a problem, because we use the rbtree to find
 * a minimum timer value only
 */

ngx_int_t
ngx_event_timer_init(ngx_log_t *log)                    /*event_timer模块初始化函数*/
{
    ngx_rbtree_init(&ngx_event_timer_rbtree, &ngx_event_timer_sentinel,
                    ngx_rbtree_insert_timer_value);      /*初始化红黑树*/

#if (NGX_THREADS)  /*没有编译*/

    if (ngx_event_timer_mutex) {
        ngx_event_timer_mutex->log = log;
        return NGX_OK;
    }

    ngx_event_timer_mutex = ngx_mutex_init(log, 0);
    if (ngx_event_timer_mutex == NULL) {
        return NGX_ERROR;
    }

#endif

    return NGX_OK;
}


ngx_msec_t
ngx_event_find_timer(void)                          /*该函数就是从红黑树中找到key最小的节点，得到最近超时时间*/
{
    ngx_msec_int_t      timer;
    ngx_rbtree_node_t  *node, *root, *sentinel;

    if (ngx_event_timer_rbtree.root == &ngx_event_timer_sentinel) {     /*判断是否为空*/
        return NGX_TIMER_INFINITE;
    }

    ngx_mutex_lock(ngx_event_timer_mutex);          /*原子量加锁*/

    root = ngx_event_timer_rbtree.root;
    sentinel = ngx_event_timer_rbtree.sentinel;

    node = ngx_rbtree_min(root, sentinel);          /*找到最左的那个节点*/

    ngx_mutex_unlock(ngx_event_timer_mutex);

    timer = (ngx_msec_int_t) (node->key - ngx_current_msec);    /*timer<0表示已经有事件超时了*/

    return (ngx_msec_t) (timer > 0 ? timer : 0);                  /*超时的话,返回0,那么就会导致立即处理这些超时事件*/
}


void
ngx_event_expire_timers(void)                       /*超时检测函数, 主要作用:(1)对超时对象进行扫描,(2)对超时对象进行处理*/
{
    ngx_event_t        *ev;
    ngx_rbtree_node_t  *node, *root, *sentinel;

    sentinel = ngx_event_timer_rbtree.sentinel;

    for ( ;; ) {

        ngx_mutex_lock(ngx_event_timer_mutex);

        root = ngx_event_timer_rbtree.root;                                             /*取根*/

        if (root == sentinel) {
            return;                                                                        /*空树*/
        }

        node = ngx_rbtree_min(root, sentinel);                                          /*找到最近的即将超时的节点,最左节点*/

        /* node->key <= ngx_current_time */

        if ((ngx_msec_int_t) (node->key - ngx_current_msec) <= 0) {                    /*判断该对象是否超时*/
            ev = (ngx_event_t *) ((char *) node - offsetof(ngx_event_t, timer));      /*计算出事件字段的地址*/

#if (NGX_THREADS)

            if (ngx_threaded && ngx_trylock(ev->lock) == 0) {

                /*
                 * We cannot change the timer of the event that is being
                 * handled by another thread.  And we cannot easy walk
                 * the rbtree to find next expired timer so we exit the loop.
                 * However, it should be a rare case when the event that is
                 * being handled has an expired timer.
                 */

                ngx_log_debug1(NGX_LOG_DEBUG_EVENT, ev->log, 0,
                               "event %p is busy in expire timers", ev);
                break;
            }
#endif

            ngx_log_debug2(NGX_LOG_DEBUG_EVENT, ev->log, 0,
                           "event timer del: %d: %M",
                           ngx_event_ident(ev->data), ev->timer.key);

            ngx_rbtree_delete(&ngx_event_timer_rbtree, &ev->timer);         /*将该定时器节点,移除红黑树*/

            ngx_mutex_unlock(ngx_event_timer_mutex);

#if (NGX_DEBUG)
            ev->timer.left = NULL;
            ev->timer.right = NULL;
            ev->timer.parent = NULL;
#endif

            ev->timer_set = 0;

#if (NGX_THREADS)
            if (ngx_threaded) {
                ev->posted_timedout = 1;

                ngx_post_event(ev, &ngx_posted_events);

                ngx_unlock(ev->lock);

                continue;
            }
#endif

            ev->timedout = 1;       /*设置超时标记*/

            ev->handler(ev);        /*执行定时器事件的回调函数*/

            continue;
        }
        /*如果最左节点都没与超时,那么就直接结束循环*/
        break;
    }

    ngx_mutex_unlock(ngx_event_timer_mutex);
}
