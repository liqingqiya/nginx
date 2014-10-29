
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_CYCLE_H_INCLUDED_
#define _NGX_CYCLE_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


#ifndef NGX_CYCLE_POOL_SIZE
#define NGX_CYCLE_POOL_SIZE     NGX_DEFAULT_POOL_SIZE
#endif


#define NGX_DEBUG_POINTS_STOP   1
#define NGX_DEBUG_POINTS_ABORT  2


typedef struct ngx_shm_zone_s  ngx_shm_zone_t;

typedef ngx_int_t (*ngx_shm_zone_init_pt) (ngx_shm_zone_t *zone, void *data);

struct ngx_shm_zone_s {
    void                     *data;
    ngx_shm_t                 shm;
    ngx_shm_zone_init_pt      init;
    void                     *tag;
};


struct ngx_cycle_s {
    void                  ****conf_ctx;/*保存着所有模块存储配置项的结构体的指针，首先是一个数组，数组成员又是一个指针，指向另一个存储着指针的数组*/
    ngx_pool_t               *pool;     /*内存池*/

    ngx_log_t                *log;
    ngx_log_t                 new_log;

    ngx_uint_t                log_use_stderr;  /* unsigned  log_use_stderr:1; */

    ngx_connection_t        **files;/*对于poll，rtsig这样的事件模块，会以有效文件句柄数来预先建立这些ngx_connection_t结构体，加速事件的收集，分发。这个时候files保存所有的ngx_connection_t的指针组成的数组*/
    ngx_connection_t         *free_connections;/*可用连接池*/
    ngx_uint_t                free_connection_n;    /*可用连接池中的总数*/

    ngx_queue_t               reusable_connections_queue; /*可重用网络连接队列*/

    ngx_array_t               listening;        /*监听的端口数组, 存储着ngx_listening_t结构体,一个监听socket对应一个listening结构*/
    ngx_array_t               paths;            /*保存nginx所要操作的目录。如果有目录不存在，那么就会创建，如果失败，那么nginx启动也会失败*/
    ngx_list_t                open_files;       /*单链表，元素类型为ngx_open_file_t，表示nginx已经打开的文件。nginx框架并不会向open_file链表中添加文件，只是由对此感兴趣的模块向其中添加文件路径名，nginx框架会在ngx_init_cycle方法中打开这些文件*/
    ngx_list_t                shared_memory;    /*元素类型为ngx_shm_zone_t结构体，每一个元素代表一块共享内存*/

    ngx_uint_t                connection_n; /*当前进程中所有连接对象的总数，与下面的connections配合使用*/
    ngx_uint_t                files_n;      /*连接池容量，与files配合使用*/

    ngx_connection_t         *connections;  /*指向当前进程中所有的连接对象，与connection_n配合使用*/
    ngx_event_t              *read_events;  /*指向当前进程中所有的读事件对象，connection_n同时表示所有的读事件总数*/
    ngx_event_t              *write_events; /*指向当前进程中所有的写事件对象，connection_n同时表示所有的写事件总数*/

    ngx_cycle_t              *old_cycle;    /*旧的cycle对象，用于引用上一个ngx_cycle_t对象中的成员。*/

    ngx_str_t                 conf_file;    /*配置文件 相对与 安装路径的名称*/
    ngx_str_t                 conf_param;   /*nginx处理配置文件时候需要特殊处理的命令行携带的参数，一般是-g选项携带的参数*/
    ngx_str_t                 conf_prefix;  /*nginx配置文件 所在 的目录路径*/
    ngx_str_t                 prefix;       /*nginx安装目录路径*/
    ngx_str_t                 lock_file;    /*用于进程间同步的文件锁名称*/
    ngx_str_t                 hostname;     /*使用gethostname系统调用得到的主机名称*/
};      /*nginx的主要结构体，存储了所需的全局变量*/


typedef struct {
     ngx_flag_t               daemon;    /*存储daemon配置的值*/
     ngx_flag_t               master;    /*存储master_process配置的值*/

     ngx_msec_t               timer_resolution;  /*存储timer_resolution配置的值*/

     ngx_int_t                worker_processes;  /*存储worker_processes配置的值*/
     ngx_int_t                debug_points;  /**/

     ngx_int_t                rlimit_nofile;  /**/
     ngx_int_t                rlimit_sigpending;  /**/
     off_t                    rlimit_core;  /**/

     int                      priority;  /**/

     ngx_uint_t               cpu_affinity_n;  /**/
     uint64_t                *cpu_affinity;  /**/

     char                    *username;
     ngx_uid_t                user;
     ngx_gid_t                group;

     ngx_str_t                working_directory;
     ngx_str_t                lock_file;

     ngx_str_t                pid;  /*存储pid配置的值*/
     ngx_str_t                oldpid;

     ngx_array_t              env;
     char                   **environment;

#if (NGX_THREADS)
     ngx_int_t                worker_threads;
     size_t                   thread_stack_size;
#endif

} ngx_core_conf_t;


typedef struct {
     ngx_pool_t              *pool;   /* pcre's malloc() pool */
} ngx_core_tls_t;


#define ngx_is_init_cycle(cycle)  (cycle->conf_ctx == NULL)


ngx_cycle_t *ngx_init_cycle(ngx_cycle_t *old_cycle);
ngx_int_t ngx_create_pidfile(ngx_str_t *name, ngx_log_t *log);
void ngx_delete_pidfile(ngx_cycle_t *cycle);
ngx_int_t ngx_signal_process(ngx_cycle_t *cycle, char *sig);
void ngx_reopen_files(ngx_cycle_t *cycle, ngx_uid_t user);
char **ngx_set_environment(ngx_cycle_t *cycle, ngx_uint_t *last);
ngx_pid_t ngx_exec_new_binary(ngx_cycle_t *cycle, char *const *argv);
uint64_t ngx_get_cpu_affinity(ngx_uint_t n);
ngx_shm_zone_t *ngx_shared_memory_add(ngx_conf_t *cf, ngx_str_t *name,
    size_t size, void *tag);


extern volatile ngx_cycle_t  *ngx_cycle;
extern ngx_array_t            ngx_old_cycles;
extern ngx_module_t           ngx_core_module;
extern ngx_uint_t             ngx_test_config;
extern ngx_uint_t             ngx_quiet_mode;
#if (NGX_THREADS)
extern ngx_tls_key_t          ngx_core_tls_key;
#endif


#endif /* _NGX_CYCLE_H_INCLUDED_ */
