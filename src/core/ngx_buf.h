
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_BUF_H_INCLUDED_
#define _NGX_BUF_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef void *            ngx_buf_tag_t;

typedef struct ngx_buf_s  ngx_buf_t;

struct ngx_buf_s {
    u_char          *pos;   /* 当前buffer真实内容的起始位置 */
    u_char          *last;  /* 当前buffer真实内容的结束位置 */
    off_t            file_pos;  /* 在文件中真实内容的起始位置   */
    off_t            file_last; /* 在文件中真实内容的结束位置   */

    u_char          *start;         /* start of buffer */   /* buffer内存的开始分配的位置 */
    u_char          *end;           /* end of buffer */     /* buffer内存的结束分配的位置 */
    ngx_buf_tag_t    tag;           /* buffer属于哪个模块的标志 */
    ngx_file_t      *file;           /* buffer所引用的文件 */
    ngx_buf_t       *shadow;        /* 用来引用替换过后的buffer，以便当所有buffer输出以后，这个buffer可以被释放。*/


    /* the buf's content could be changed */
    unsigned         temporary:1;/*临时内存标志位，为1时表示数据在内存中且这段内存可以修改*/

    /*
     * the buf's content is in a memory cache or in a read only memory
     * and must not be changed
     */
    unsigned         memory:1;/*标志位，为1时表示数据在内存中且这段内存不可以被修改*/

    /* the buf's content is mmap()ed and must not be changed */
    unsigned         mmap:1;/*标志位，为1时表示这段内存是用mmap系统调用映射过来的，不可以被修改*/

    unsigned         recycled:1;    /* 内存可以被输出并回收 */
    unsigned         in_file:1;     /* buffer的内容在文件中 */
    unsigned         flush:1;       /* 标志位，为1时表示需要执行flush操作,马上全部输出buffer的内容, gzip模块里面用得比较多 */
    unsigned         sync:1;        /*标志位，对于操作这块缓冲区时是否使用同步方式*/
    unsigned         last_buf:1;    /* 标志位，表示是否是最后一块缓冲区，因为ngx_buf_t可以由ngx_chain_t链表串联起来，因此，当last_buf为1时，表示当前是最后一块待处理的缓冲区??todo */
    unsigned         last_in_chain:1;   /* 标志位，表示是否是ngx_chain_t中的最后一块缓冲区 */

    unsigned         last_shadow:1;
    unsigned         temp_file:1;   /* 标志位，表示当前缓冲区是否属于临时文件 */

    /* STUB */ int   num;   /* 统计用，表示使用次数 */
};


struct ngx_chain_s {
    ngx_buf_t    *buf;
    ngx_chain_t  *next;
};  /*链表结构*/


typedef struct {
    ngx_int_t    num; /*buf节点个数*/
    size_t       size; /*buf节点大小*/
} ngx_bufs_t; /*用于创建一整个的chain*/


typedef struct ngx_output_chain_ctx_s  ngx_output_chain_ctx_t;

typedef ngx_int_t (*ngx_output_chain_filter_pt)(void *ctx, ngx_chain_t *in);

#if (NGX_HAVE_FILE_AIO)
typedef void (*ngx_output_chain_aio_pt)(ngx_output_chain_ctx_t *ctx,
    ngx_file_t *file);
#endif

struct ngx_output_chain_ctx_s {
    ngx_buf_t                   *buf;
    ngx_chain_t                 *in;
    ngx_chain_t                 *free;
    ngx_chain_t                 *busy;

    unsigned                     sendfile:1;
    unsigned                     directio:1;
#if (NGX_HAVE_ALIGNED_DIRECTIO)
    unsigned                     unaligned:1;
#endif
    unsigned                     need_in_memory:1;
    unsigned                     need_in_temp:1;
#if (NGX_HAVE_FILE_AIO)
    unsigned                     aio:1;

    ngx_output_chain_aio_pt      aio_handler;
#endif

    off_t                        alignment;

    ngx_pool_t                  *pool;
    ngx_int_t                    allocated;
    ngx_bufs_t                   bufs;
    ngx_buf_tag_t                tag;

    ngx_output_chain_filter_pt   output_filter;
    void                        *filter_ctx;
};


typedef struct {
    ngx_chain_t                 *out;
    ngx_chain_t                **last;
    ngx_connection_t            *connection;
    ngx_pool_t                  *pool;
    off_t                        limit;
} ngx_chain_writer_ctx_t;


#define NGX_CHAIN_ERROR     (ngx_chain_t *) NGX_ERROR


#define ngx_buf_in_memory(b)        (b->temporary || b->memory || b->mmap) /*返回这个buf里面的内容是否在内存里*/
#define ngx_buf_in_memory_only(b)   (ngx_buf_in_memory(b) && !b->in_file) /*返回这个buf里面的内容是否仅仅在内存里，并且没有在文件里*/

#define ngx_buf_special(b)                                                   \
    ((b->flush || b->last_buf || b->sync)                                    \
     && !ngx_buf_in_memory(b) && !b->in_file) /*返回该buf是否是一个特殊的buf，只含有特殊的标志和没有包含真正的数据*/

#define ngx_buf_sync_only(b)                                                 \
    (b->sync                                                                 \
     && !ngx_buf_in_memory(b) && !b->in_file && !b->flush && !b->last_buf) /*返回该buf是否是一个只包含sync标志而不包含真正数据的特殊buf*/

#define ngx_buf_size(b)                                                      \
    (ngx_buf_in_memory(b) ? (off_t) (b->last - b->pos):                      \
                            (b->file_last - b->file_pos)) /*返回该buf所含数据的大小，不管这个数据是在文件里还是在内存里*/

ngx_buf_t *ngx_create_temp_buf(ngx_pool_t *pool, size_t size);
ngx_chain_t *ngx_create_chain_of_bufs(ngx_pool_t *pool, ngx_bufs_t *bufs);


#define ngx_alloc_buf(pool)  ngx_palloc(pool, sizeof(ngx_buf_t))
#define ngx_calloc_buf(pool) ngx_pcalloc(pool, sizeof(ngx_buf_t)) /*结构体大小*/

ngx_chain_t *ngx_alloc_chain_link(ngx_pool_t *pool);
#define ngx_free_chain(pool, cl)                                             \
    cl->next = pool->chain;                                                  \
    pool->chain = cl



ngx_int_t ngx_output_chain(ngx_output_chain_ctx_t *ctx, ngx_chain_t *in);
ngx_int_t ngx_chain_writer(void *ctx, ngx_chain_t *in);

ngx_int_t ngx_chain_add_copy(ngx_pool_t *pool, ngx_chain_t **chain,
    ngx_chain_t *in);
ngx_chain_t *ngx_chain_get_free_buf(ngx_pool_t *p, ngx_chain_t **free);
void ngx_chain_update_chains(ngx_pool_t *p, ngx_chain_t **free,
    ngx_chain_t **busy, ngx_chain_t **out, ngx_buf_tag_t tag);


#endif /* _NGX_BUF_H_INCLUDED_ */
