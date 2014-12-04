
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_CORE_H_INCLUDED_
#define _NGX_CORE_H_INCLUDED_


typedef struct ngx_module_s      ngx_module_t;		/*模块结构*/
typedef struct ngx_conf_s        ngx_conf_t;			/**/
typedef struct ngx_cycle_s       ngx_cycle_t;
typedef struct ngx_pool_s        ngx_pool_t;			/*内存管理结构*/
typedef struct ngx_chain_s       ngx_chain_t;
typedef struct ngx_log_s         ngx_log_t;
typedef struct ngx_open_file_s   ngx_open_file_t;
typedef struct ngx_command_s     ngx_command_t;		/*指令结构*/
typedef struct ngx_file_s        ngx_file_t;
typedef struct ngx_event_s       ngx_event_t;
typedef struct ngx_event_aio_s   ngx_event_aio_t;
typedef struct ngx_connection_s  ngx_connection_t;	/*连接结构*/

typedef void (*ngx_event_handler_pt)(ngx_event_t *ev);
typedef void (*ngx_connection_handler_pt)(ngx_connection_t *c);


#define  NGX_OK          0
#define  NGX_ERROR      -1
#define  NGX_AGAIN      -2
#define  NGX_BUSY       -3
#define  NGX_DONE       -4
#define  NGX_DECLINED   -5
#define  NGX_ABORT      -6

/*这些文件的导入是有严格的先后顺序的*/
#include <ngx_errno.h> 						/*linux 错误定义*/
#include <ngx_atomic.h> 						/*linux 原子量*/
#include <ngx_thread.h> 						/*linux 线程有关设置*/
#include <ngx_rbtree.h> 						/*红黑树*/
#include <ngx_time.h> 			 			/*linux 系统上时间的封装*/
#include <ngx_socket.h> 						/*linux 套接字*/
#include <ngx_string.h> 						/*字符串*/
#include <ngx_files.h>						/*linux 文件*/
#include <ngx_shmem.h> 						/*共享内存*/
#include <ngx_process.h> 					/*linux 进程*/
#include <ngx_user.h> 						/**/
#include <ngx_parse.h> 						/**/
#include <ngx_log.h> 						/*日志*/
#include <ngx_alloc.h> 						/**/
#include <ngx_palloc.h> 						/*内存池*/
#include <ngx_buf.h>   						/*缓存*/
#include <ngx_queue.h>   						/*队列*/
#include <ngx_array.h> 						/*数组*/
#include <ngx_list.h> 	 					/*链表*/
#include <ngx_hash.h> 						/*哈希*/
#include <ngx_file.h> 						/*文件接口*/
#include <ngx_crc.h>
#include <ngx_crc32.h>
#include <ngx_murmurhash.h>
#if (NGX_PCRE)
#include <ngx_regex.h>
#endif
#include <ngx_radix_tree.h>
#include <ngx_times.h> 						/*nginx关于时间的功能接口*/
#include <ngx_shmtx.h>
#include <ngx_slab.h>
#include <ngx_inet.h>
#include <ngx_cycle.h>
#include <ngx_resolver.h>
#if (NGX_OPENSSL)
#include <ngx_event_openssl.h>
#endif
#include <ngx_process_cycle.h>
#include <ngx_conf_file.h> 					/*框架定义*/
#include <ngx_open_file_cache.h>
#include <ngx_os.h>
#include <ngx_connection.h>
#include <ngx_proxy_protocol.h>


#define LF     (u_char) 10 					/*换行符号*/
#define CR     (u_char) 13 					/*回车*/
#define CRLF   "\x0d\x0a" 					/*请求行/首部行之间的标准间隔*/


#define ngx_abs(value)       (((value) >= 0) ? (value) : - (value))
#define ngx_max(val1, val2)  ((val1 < val2) ? (val2) : (val1))
#define ngx_min(val1, val2)  ((val1 > val2) ? (val2) : (val1))

void ngx_cpuinfo(void);

#if (NGX_HAVE_OPENAT)
#define NGX_DISABLE_SYMLINKS_OFF        0
#define NGX_DISABLE_SYMLINKS_ON         1
#define NGX_DISABLE_SYMLINKS_NOTOWNER   2
#endif

#endif /* _NGX_CORE_H_INCLUDED_ */
