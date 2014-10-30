
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_PROCESS_H_INCLUDED_
#define _NGX_PROCESS_H_INCLUDED_


#include <ngx_setaffinity.h>
#include <ngx_setproctitle.h>


typedef pid_t       ngx_pid_t;

#define NGX_INVALID_PID  -1

typedef void (*ngx_spawn_proc_pt) (ngx_cycle_t *cycle, void *data);

typedef struct {
    ngx_pid_t           pid;            /*进程ID*/
    int                 status;         /*由waitpid系统调用获取到的进程状态*/
    ngx_socket_t        channel[2];         /*由socketpair系统调用产生出的用于进程通信的socket句柄,这一对socket句柄可以相互通信，目前用于master父进程和worker子进程的通信*/

    ngx_spawn_proc_pt   proc;           /*子进程的循环执行方法，当父进程调用ngx_spawn_process 生成子进程的时候使用*/
    void               *data;           /*ngx_spawn_proc_pt的第二个参数, 可选*/
    char               *name;           /*进程名称，操作系统中显示的进程名称与name相同*/

    unsigned            respawn:1;          /*标志位，为1时，表示在重新生成子进程*/
    unsigned            just_spawn:1;           /*标志位，为1时，表示正在生成自己成*/
    unsigned            detached:1;         /*标志位，为1时，表示在进行父，子进程的分离*/
    unsigned            exiting:1;          /*标志位， 为1时， 表示进程正在退出*/
    unsigned            exited:1;           /*标志位， 为1时， 表示进程已经退出*/
} ngx_process_t; /*子进程相关结构体*/


typedef struct {
    char         *path;
    char         *name;
    char *const  *argv;
    char *const  *envp;
} ngx_exec_ctx_t;


#define NGX_MAX_PROCESSES         1024 /*最大进程数*/

#define NGX_PROCESS_NORESPAWN     -1
#define NGX_PROCESS_JUST_SPAWN    -2
#define NGX_PROCESS_RESPAWN       -3
#define NGX_PROCESS_JUST_RESPAWN  -4
#define NGX_PROCESS_DETACHED      -5


#define ngx_getpid   getpid     /*系统调用，获取进程id号*/

#ifndef ngx_log_pid
#define ngx_log_pid  ngx_pid
#endif


ngx_pid_t ngx_spawn_process(ngx_cycle_t *cycle,
    ngx_spawn_proc_pt proc, void *data, char *name, ngx_int_t respawn);
ngx_pid_t ngx_execute(ngx_cycle_t *cycle, ngx_exec_ctx_t *ctx);
ngx_int_t ngx_init_signals(ngx_log_t *log);
void ngx_debug_point(void);


#if (NGX_HAVE_SCHED_YIELD)
#define ngx_sched_yield()  sched_yield()
#else
#define ngx_sched_yield()  usleep(1)
#endif


extern int            ngx_argc;
extern char         **ngx_argv;
extern char         **ngx_os_argv;

extern ngx_pid_t      ngx_pid;
extern ngx_socket_t   ngx_channel;
extern ngx_int_t      ngx_process_slot;
extern ngx_int_t      ngx_last_process;
extern ngx_process_t  ngx_processes[NGX_MAX_PROCESSES];


#endif /* _NGX_PROCESS_H_INCLUDED_ */
