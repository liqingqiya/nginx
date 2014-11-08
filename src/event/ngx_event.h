
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_EVENT_H_INCLUDED_
#define _NGX_EVENT_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


#define NGX_INVALID_INDEX  0xd0d0d0d0


#if (NGX_HAVE_IOCP)

typedef struct {
    WSAOVERLAPPED    ovlp;
    ngx_event_t     *event;
    int              error;
} ngx_event_ovlp_t;

#endif


typedef struct {
    ngx_uint_t       lock;

    ngx_event_t     *events;
    ngx_event_t     *last;
} ngx_event_mutex_t;


struct ngx_event_s {  /*代表一个事件*/
    void            *data; /*事件相关对象.通常data都是指向ngx_connection_t连接对象。开启文件异步I/O时候，它可能会指向ngx_event_aio_t结构体*/

    unsigned         write:1;/*是否可写, 通常表示对应的TCP连接目前状态可写，也就是收连接处于可以发送网络包的状态*/

    unsigned         accept:1;/*为1时，表示此事件可以建立新的连接。通常监听套接字收到客户端请求，fork一个处理套接字的话，就会标志为1*/

    /* used to detect the stale events in kqueue, rtsig, and epoll */
    unsigned         instance:1;/*标志位，用于区分当前事件是否过期，该标志仅仅用于事件驱动模块，而不用于事件消费模块*/

    /*
     * the event was passed or would be passed to a kernel;
     * in aio mode - operation was posted.
     */
    unsigned         active:1; /*为1是表示当前事件活跃，这个状态对应着事件驱动模块处理方式的不同*/

    unsigned         disabled:1; /*为1是表示禁用该事件, kqueue||rtsig事件驱动模块中有效，epoll驱动模块中无效*/

    /* the ready event; in aio mode 0 means that no operation can be posted */
    unsigned         ready:1;/*为1时，表示当前事件已经准备就绪了*/

    unsigned         oneshot:1; /*kqueue||eventport等模块有效，epoll事件驱动模块无效*/

    /* aio operation is complete */
    unsigned         complete:1; /*用于异步AIO事件的处理*/

    unsigned         eof:1; /*为1时，表示当前处理的字符流已经结束*/
    unsigned         error:1;/*为1时，表示事件在处理过程中出现错误*/

    unsigned         timedout:1;/*是否超时,0表示没超时*/
    unsigned         timer_set:1;/*为1时，表示这个事件存在于定时器中*/

    unsigned         delayed:1;/*为1时，表示需要延迟处理这个事件，仅仅用于限速功能*/

    unsigned         deferred_accept:1;/*未使用*/

    /* the pending eof reported by kqueue, epoll or in aio chain operation */
    unsigned         pending_eof:1;/*为1时，表示等待字符流已经结束*/

#if !(NGX_THREADS)
    unsigned         posted_ready:1; /*为1时，表示处理post事件的时，当前事件已经准备就绪*/
#endif

#if (NGX_WIN32)
    /* setsockopt(SO_UPDATE_ACCEPT_CONTEXT) was successful */
    unsigned         accept_context_updated:1;
#endif

#if (NGX_HAVE_KQUEUE)
    unsigned         kq_vnode:1;

    /* the pending errno reported by kqueue */
    int              kq_errno;
#endif

    /*
     * kqueue only:
     *   accept:     number of sockets that wait to be accepted
     *   read:       bytes to read when event is ready
     *               or lowat when event is set with NGX_LOWAT_EVENT flag
     *   write:      available space in buffer when event is ready
     *               or lowat when event is set with NGX_LOWAT_EVENT flag
     *
     * iocp: TODO
     *
     * otherwise:
     *   accept:     1 if accept many, 0 otherwise
     */

#if (NGX_HAVE_KQUEUE) || (NGX_HAVE_IOCP)
    int              available;
#else
    unsigned         available:1; /*epoll事件驱动机制表示尽可能多地建立TCP连接，与 multi_accept配置项对应*/
#endif

    ngx_event_handler_pt  handler; /*这个事件发生的时候的处理方法，每一个事件消费模块会重新实现它*/


#if (NGX_HAVE_AIO)

#if (NGX_HAVE_IOCP)
    ngx_event_ovlp_t ovlp;
#else
    struct aiocb     aiocb; /*linux aio机制定义的结构体*/
#endif

#endif

    ngx_uint_t       index; /*epoll事件驱动方式不使用index*/

    ngx_log_t       *log;

    ngx_rbtree_node_t   timer; /*定时器节点，加入红黑树时需要的辅助节点，红黑树就是通过该字段来组织所有的超时事件对象*/

    unsigned         closed:1; /*为1表示事件已经关闭，epoll事件驱动模块没有使用到*/

    /* to test on worker exit */
    unsigned         channel:1;
    unsigned         resolver:1;

#if (NGX_THREADS)

    unsigned         locked:1;

    unsigned         posted_ready:1;
    unsigned         posted_timedout:1;
    unsigned         posted_eof:1;

#if (NGX_HAVE_KQUEUE)
    /* the pending errno reported by kqueue */
    int              posted_errno;
#endif

#if (NGX_HAVE_KQUEUE) || (NGX_HAVE_IOCP)
    int              posted_available;
#else
    unsigned         posted_available:1;
#endif

    ngx_atomic_t    *lock;
    ngx_atomic_t    *own_lock;

#endif

    /* the links of the posted queue */
    ngx_event_t     *next; /*post事件将会构成一个队列再统一处理，这个队列以next，prev为链表指针，构成一个简单的双向队列*/
    ngx_event_t    **prev; /**/


#if 0

    /* the threads support */

    /*
     * the event thread context, we store it here
     * if $(CC) does not understand __thread declaration
     * and pthread_getspecific() is too costly
     */

    void            *thr_ctx;

#if (NGX_EVENT_T_PADDING)

    /* event should not cross cache line in SMP */

    uint32_t         padding[NGX_EVENT_T_PADDING];
#endif
#endif
};


#if (NGX_HAVE_FILE_AIO)

struct ngx_event_aio_s {
    void                      *data;
    ngx_event_handler_pt       handler;
    ngx_file_t                *file;

    ngx_fd_t                   fd;

#if (NGX_HAVE_EVENTFD)
    int64_t                    res;
#if (NGX_TEST_BUILD_EPOLL)
    ngx_err_t                  err;
    size_t                     nbytes;
#endif
#else
    ngx_err_t                  err;
    size_t                     nbytes;
#endif

#if (NGX_HAVE_AIO_SENDFILE)
    off_t                      last_offset;
#endif

    ngx_aiocb_t                aiocb;
    ngx_event_t                event;
};

#endif


typedef struct {
    ngx_int_t  (*add)(ngx_event_t *ev, ngx_int_t event, ngx_uint_t flags);  /*将某描述符的事件添加到事件驱动列表中*/
    ngx_int_t  (*del)(ngx_event_t *ev, ngx_int_t event, ngx_uint_t flags);/*将某描述符的事件从事件驱动列表中删除*/

    ngx_int_t  (*enable)(ngx_event_t *ev, ngx_int_t event, ngx_uint_t flags);/*启动对某个事件的监控*/
    ngx_int_t  (*disable)(ngx_event_t *ev, ngx_int_t event, ngx_uint_t flags);/*禁止对某个事件的监控*/

    ngx_int_t  (*add_conn)(ngx_connection_t *c);/*将指定的连接关联的描述符添加到多路io复用的监控里*/
    ngx_int_t  (*del_conn)(ngx_connection_t *c, ngx_uint_t flags);/*将指定的连接关联的描述符从多路复用的监控里删除*/

    ngx_int_t  (*process_changes)(ngx_cycle_t *cycle, ngx_uint_t nowait);
    ngx_int_t  (*process_events)(ngx_cycle_t *cycle, ngx_msec_t timer,
                   ngx_uint_t flags);/*阻塞等待事件发生,对发生的事件进行逐个处理*/

    ngx_int_t  (*init)(ngx_cycle_t *cycle, ngx_msec_t timer);/*初始化*/
    void       (*done)(ngx_cycle_t *cycle);/*释放资源*/
} ngx_event_actions_t; /*I/O多路复用模型统一接口*/


extern ngx_event_actions_t   ngx_event_actions;


/*
 * The event filter requires to read/write the whole data:
 * select, poll, /dev/poll, kqueue, epoll.
 */
#define NGX_USE_LEVEL_EVENT      0x00000001

/*
 * The event filter is deleted after a notification without an additional
 * syscall: kqueue, epoll.
 */
#define NGX_USE_ONESHOT_EVENT    0x00000002

/*
 * The event filter notifies only the changes and an initial level:
 * kqueue, epoll.
 */
#define NGX_USE_CLEAR_EVENT      0x00000004

/*
 * The event filter has kqueue features: the eof flag, errno,
 * available data, etc.
 */
#define NGX_USE_KQUEUE_EVENT     0x00000008

/*
 * The event filter supports low water mark: kqueue's NOTE_LOWAT.
 * kqueue in FreeBSD 4.1-4.2 has no NOTE_LOWAT so we need a separate flag.
 */
#define NGX_USE_LOWAT_EVENT      0x00000010

/*
 * The event filter requires to do i/o operation until EAGAIN: epoll, rtsig.
 */
#define NGX_USE_GREEDY_EVENT     0x00000020

/*
 * The event filter is epoll.
 */
#define NGX_USE_EPOLL_EVENT      0x00000040

/*
 * No need to add or delete the event filters: rtsig.
 */
#define NGX_USE_RTSIG_EVENT      0x00000080

/*
 * No need to add or delete the event filters: overlapped, aio_read,
 * aioread, io_submit.
 */
#define NGX_USE_AIO_EVENT        0x00000100

/*
 * Need to add socket or handle only once: i/o completion port.
 * It also requires NGX_HAVE_AIO and NGX_USE_AIO_EVENT to be set.
 */
#define NGX_USE_IOCP_EVENT       0x00000200

/*
 * The event filter has no opaque data and requires file descriptors table:
 * poll, /dev/poll, rtsig.
 */
#define NGX_USE_FD_EVENT         0x00000400

/*
 * The event module handles periodic or absolute timer event by itself:
 * kqueue in FreeBSD 4.4, NetBSD 2.0, and MacOSX 10.4, Solaris 10's event ports.
 */
#define NGX_USE_TIMER_EVENT      0x00000800

/*
 * All event filters on file descriptor are deleted after a notification:
 * Solaris 10's event ports.
 */
#define NGX_USE_EVENTPORT_EVENT  0x00001000

/*
 * The event filter support vnode notifications: kqueue.
 */
#define NGX_USE_VNODE_EVENT      0x00002000


/*
 * The event filter is deleted just before the closing file.
 * Has no meaning for select and poll.
 * kqueue, epoll, rtsig, eventport:  allows to avoid explicit delete,
 *                                   because filter automatically is deleted
 *                                   on file close,
 *
 * /dev/poll:                        we need to flush POLLREMOVE event
 *                                   before closing file.
 */
#define NGX_CLOSE_EVENT    1

/*
 * disable temporarily event filter, this may avoid locks
 * in kernel malloc()/free(): kqueue.
 */
#define NGX_DISABLE_EVENT  2

/*
 * event must be passed to kernel right now, do not wait until batch processing.
 */
#define NGX_FLUSH_EVENT    4


/* these flags have a meaning only for kqueue */
#define NGX_LOWAT_EVENT    0
#define NGX_VNODE_EVENT    0


#if (NGX_HAVE_EPOLL) && !(NGX_HAVE_EPOLLRDHUP)
#define EPOLLRDHUP         0
#endif


#if (NGX_HAVE_KQUEUE)

#define NGX_READ_EVENT     EVFILT_READ
#define NGX_WRITE_EVENT    EVFILT_WRITE

#undef  NGX_VNODE_EVENT
#define NGX_VNODE_EVENT    EVFILT_VNODE

/*
 * NGX_CLOSE_EVENT, NGX_LOWAT_EVENT, and NGX_FLUSH_EVENT are the module flags
 * and they must not go into a kernel so we need to choose the value
 * that must not interfere with any existent and future kqueue flags.
 * kqueue has such values - EV_FLAG1, EV_EOF, and EV_ERROR:
 * they are reserved and cleared on a kernel entrance.
 */
#undef  NGX_CLOSE_EVENT
#define NGX_CLOSE_EVENT    EV_EOF

#undef  NGX_LOWAT_EVENT
#define NGX_LOWAT_EVENT    EV_FLAG1

#undef  NGX_FLUSH_EVENT
#define NGX_FLUSH_EVENT    EV_ERROR

#define NGX_LEVEL_EVENT    0
#define NGX_ONESHOT_EVENT  EV_ONESHOT
#define NGX_CLEAR_EVENT    EV_CLEAR

#undef  NGX_DISABLE_EVENT
#define NGX_DISABLE_EVENT  EV_DISABLE


#elif (NGX_HAVE_DEVPOLL || NGX_HAVE_EVENTPORT)

#define NGX_READ_EVENT     POLLIN
#define NGX_WRITE_EVENT    POLLOUT

#define NGX_LEVEL_EVENT    0
#define NGX_ONESHOT_EVENT  1


#elif (NGX_HAVE_EPOLL)

#define NGX_READ_EVENT     (EPOLLIN|EPOLLRDHUP)
#define NGX_WRITE_EVENT    EPOLLOUT

#define NGX_LEVEL_EVENT    0
#define NGX_CLEAR_EVENT    EPOLLET
#define NGX_ONESHOT_EVENT  0x70000000
#if 0
#define NGX_ONESHOT_EVENT  EPOLLONESHOT
#endif


#elif (NGX_HAVE_POLL)

#define NGX_READ_EVENT     POLLIN
#define NGX_WRITE_EVENT    POLLOUT

#define NGX_LEVEL_EVENT    0
#define NGX_ONESHOT_EVENT  1


#else /* select */

#define NGX_READ_EVENT     0
#define NGX_WRITE_EVENT    1

#define NGX_LEVEL_EVENT    0
#define NGX_ONESHOT_EVENT  1

#endif /* NGX_HAVE_KQUEUE */


#if (NGX_HAVE_IOCP)
#define NGX_IOCP_ACCEPT      0
#define NGX_IOCP_IO          1
#define NGX_IOCP_CONNECT     2
#endif


#ifndef NGX_CLEAR_EVENT
#define NGX_CLEAR_EVENT    0    /* dummy declaration */
#endif


#define ngx_process_changes  ngx_event_actions.process_changes
#define ngx_process_events   ngx_event_actions.process_events
#define ngx_done_events      ngx_event_actions.done

#define ngx_add_event        ngx_event_actions.add /*添加事件*/
#define ngx_del_event        ngx_event_actions.del  /*删除事件*/
#define ngx_add_conn         ngx_event_actions.add_conn
#define ngx_del_conn         ngx_event_actions.del_conn

#define ngx_add_timer        ngx_event_add_timer
#define ngx_del_timer        ngx_event_del_timer


extern ngx_os_io_t  ngx_io;

#define ngx_recv             ngx_io.recv
#define ngx_recv_chain       ngx_io.recv_chain
#define ngx_udp_recv         ngx_io.udp_recv
#define ngx_send             ngx_io.send
#define ngx_send_chain       ngx_io.send_chain


#define NGX_EVENT_MODULE      0x544E5645  /* "EVNT" */
#define NGX_EVENT_CONF        0x02000000


typedef struct {
    ngx_uint_t    connections;                  /*连接数*/
    ngx_uint_t    use;                           /*todo*/  

    ngx_flag_t    multi_accept;                 /*todo*/
    ngx_flag_t    accept_mutex;                 /*负载均衡锁*/

    ngx_msec_t    accept_mutex_delay;           /*todo*/

    u_char       *name;                           /*todo*/

#if (NGX_DEBUG)
    ngx_array_t   debug_connection;             /*todo*/
#endif
} ngx_event_conf_t;


typedef struct {
    ngx_str_t              *name;/*事件模块名称*/

    void                 *(*create_conf)(ngx_cycle_t *cycle);/*配置解析前，这个回调函数用于创建存储配置参数的结构体*/
    char                 *(*init_conf)(ngx_cycle_t *cycle, void *conf); /*回调函数结束后，init_conf调用，用以综合处理当前模块感兴趣的全部配置项*/

    ngx_event_actions_t     actions; /*对于事件驱动机制，每个事件模块需要实现的10个抽象方法*/
} ngx_event_module_t;


extern ngx_atomic_t          *ngx_connection_counter;

extern ngx_atomic_t          *ngx_accept_mutex_ptr;
extern ngx_shmtx_t            ngx_accept_mutex;
extern ngx_uint_t             ngx_use_accept_mutex;
extern ngx_uint_t             ngx_accept_events;
extern ngx_uint_t             ngx_accept_mutex_held;
extern ngx_msec_t             ngx_accept_mutex_delay;
extern ngx_int_t              ngx_accept_disabled;


#if (NGX_STAT_STUB)

extern ngx_atomic_t  *ngx_stat_accepted;
extern ngx_atomic_t  *ngx_stat_handled;
extern ngx_atomic_t  *ngx_stat_requests;
extern ngx_atomic_t  *ngx_stat_active;
extern ngx_atomic_t  *ngx_stat_reading;
extern ngx_atomic_t  *ngx_stat_writing;
extern ngx_atomic_t  *ngx_stat_waiting;

#endif


#define NGX_UPDATE_TIME         1
#define NGX_POST_EVENTS         2
#define NGX_POST_THREAD_EVENTS  4


extern sig_atomic_t           ngx_event_timer_alarm;
extern ngx_uint_t             ngx_event_flags;
extern ngx_module_t           ngx_events_module;
extern ngx_module_t           ngx_event_core_module;


#define ngx_event_get_conf(conf_ctx, module)                                  \
             (*(ngx_get_conf(conf_ctx, ngx_events_module))) [module.ctx_index]; /*获取事件模块的配置结构体*/



void ngx_event_accept(ngx_event_t *ev);
ngx_int_t ngx_trylock_accept_mutex(ngx_cycle_t *cycle);
u_char *ngx_accept_log_error(ngx_log_t *log, u_char *buf, size_t len);


void ngx_process_events_and_timers(ngx_cycle_t *cycle);
ngx_int_t ngx_handle_read_event(ngx_event_t *rev, ngx_uint_t flags);
ngx_int_t ngx_handle_write_event(ngx_event_t *wev, size_t lowat);


#if (NGX_WIN32)
void ngx_event_acceptex(ngx_event_t *ev);
ngx_int_t ngx_event_post_acceptex(ngx_listening_t *ls, ngx_uint_t n);
u_char *ngx_acceptex_log_error(ngx_log_t *log, u_char *buf, size_t len);
#endif


ngx_int_t ngx_send_lowat(ngx_connection_t *c, size_t lowat);


/* used in ngx_log_debugX() */
#define ngx_event_ident(p)  ((ngx_connection_t *) (p))->fd


#include <ngx_event_timer.h>
#include <ngx_event_posted.h>
#include <ngx_event_busy_lock.h>

#if (NGX_WIN32)
#include <ngx_iocp_module.h>
#endif


#endif /* _NGX_EVENT_H_INCLUDED_ */
