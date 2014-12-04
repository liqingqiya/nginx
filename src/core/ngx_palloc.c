
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


static void *ngx_palloc_block(ngx_pool_t *pool, size_t size);
static void *ngx_palloc_large(ngx_pool_t *pool, size_t size);


ngx_pool_t *
ngx_create_pool(size_t size, ngx_log_t *log)
{
    ngx_pool_t  *p;
    /*
    进行16个字节的内存对齐分配，对其处理一般是为了从性能上考虑,
    这里的ngx_memalign本质上是调用了系统调用posix_memalign()，nginx这里进行了一点封装，主要是为了日志打印的功能记录.
    这里操作是进行16字节的内存分配，也就是说，开辟size大小的内存，返回的内存地址是NGX_POOL_ALIGNMENT的倍数.
    */
    p = ngx_memalign(NGX_POOL_ALIGNMENT, size, log);    
    if (p == NULL) {
        return NULL;
    }

    p->d.last = (u_char *) p + sizeof(ngx_pool_t);    /*该内存池数据区域的最后地址*/
    p->d.end = (u_char *) p + size;                     /*分配的内存块的最后地址*/
    p->d.next = NULL;                                   /*指向下一个节点*/
    p->d.failed = 0;

    size = size - sizeof(ngx_pool_t);
    p->max = (size < NGX_MAX_ALLOC_FROM_POOL) ? size : NGX_MAX_ALLOC_FROM_POOL; /*todo*/

    p->current = p;     /*整个系统我们的内存池只有一个, 所以要知道内存池地址, 当前内存池的地址*/
    p->chain = NULL;
    p->large = NULL;
    p->cleanup = NULL;
    p->log = log;

    return p;
}


void
ngx_destroy_pool(ngx_pool_t *pool)
{
    ngx_pool_t          *p, *n;
    ngx_pool_large_t    *l;
    ngx_pool_cleanup_t  *c;
    /*调用设定好的回调函数*/
    for (c = pool->cleanup; c; c = c->next) {
        if (c->handler) {
            ngx_log_debug1(NGX_LOG_DEBUG_ALLOC, pool->log, 0,
                           "run cleanup: %p", c);
            c->handler(c->data);
        }
    }
    /*销毁大块内存块*/
    for (l = pool->large; l; l = l->next) {

        ngx_log_debug1(NGX_LOG_DEBUG_ALLOC, pool->log, 0, "free: %p", l->alloc);

        if (l->alloc) {
            ngx_free(l->alloc);
        }
    }

#if (NGX_DEBUG)

    /*
     * we could allocate the pool->log from this pool
     * so we cannot use this log while free()ing the pool
     */

    for (p = pool, n = pool->d.next; /* void */; p = n, n = n->d.next) {
        ngx_log_debug2(NGX_LOG_DEBUG_ALLOC, pool->log, 0,
                       "free: %p, unused: %uz", p, p->d.end - p->d.last);

        if (n == NULL) {
            break;
        }
    }

#endif
    /*小块的内存块销毁，内存池的数据区域，真正的内存池的核心结构*/
    for (p = pool, n = pool->d.next; /* void */; p = n, n = n->d.next) {
        ngx_free(p);

        if (n == NULL) {
            break;
        }
    }
}


void
ngx_reset_pool(ngx_pool_t *pool)
{
    ngx_pool_t        *p;
    ngx_pool_large_t  *l;
    /*释放大块存储区域*/
    for (l = pool->large; l; l = l->next) {
        if (l->alloc) {
            ngx_free(l->alloc);     /*大块内存, 就直接用ngx_free()释放*/
        }
    }
    /*内存池链中的每一个内存池节点都清零failed，并且last也复位，这个时候就没有数据区域了*/
    for (p = pool; p; p = p->d.next) {
        p->d.last = (u_char *) p + sizeof(ngx_pool_t);        /*清零*/
        p->d.failed = 0;
    }

    pool->current = pool;                                       /*当前内存池*/
    pool->chain = NULL;
    pool->large = NULL;
}

/*核心函数, 分配内存*/
void *
ngx_palloc(ngx_pool_t *pool, size_t size)   /*尝试从pool内存池里分配size大小的内存空间*/
{
    u_char      *m;
    ngx_pool_t  *p;

    if (size <= pool->max) {    /*size在内存池设定的可分配的最大值的范围内*/

        p = pool->current;

        do {
            m = ngx_align_ptr(p->d.last, NGX_ALIGNMENT);        /*定义在ngx_config.h*/

            if ((size_t) (p->d.end - m) >= size) {
                p->d.last = m + size;

                return m;   /*返回可用内存地址*/
            }

            p = p->d.next;  /*当前节点不够分配，那么移到下一个内存节点*/

        } while (p);

        return ngx_palloc_block(pool, size);    /*都不够分配，那么重新开辟一个内存节点节点, 并返回地址*/
    }

    return ngx_palloc_large(pool, size);    /*size太大，那么就开辟一个大内存节点结构 ngx_pool_large_t,并返回地址*/
}


void *
ngx_pnalloc(ngx_pool_t *pool, size_t size)
{
    u_char      *m;
    ngx_pool_t  *p;

    if (size <= pool->max) {

        p = pool->current;

        do {
            m = p->d.last;

            if ((size_t) (p->d.end - m) >= size) {
                p->d.last = m + size;

                return m;
            }

            p = p->d.next;

        } while (p);

        return ngx_palloc_block(pool, size);    /*新建一个内存池，组成内存池链表*/
    }

    return ngx_palloc_large(pool, size);    /*开辟一个大存储块*/
}


static void *
ngx_palloc_block(ngx_pool_t *pool, size_t size) /*新建一个内存池，并与先前的内存池组成一个内存池链表*/
{
    u_char      *m;
    size_t       psize;
    ngx_pool_t  *p, *new, *current;

    psize = (size_t) (pool->d.end - (u_char *) pool);   /*当前内存池的大小*/

    m = ngx_memalign(NGX_POOL_ALIGNMENT, psize, pool->log); /*对齐分配psize大小的内存*/
    if (m == NULL) {
        return NULL;
    }

    new = (ngx_pool_t *) m; /*强制转化指针*/

    new->d.end = m + psize; /*指向新开辟的内存池的结尾*/
    new->d.next = NULL;     /*下一个内存池*/
    new->d.failed = 0;      /*内存分配失败的次数*/

    m += sizeof(ngx_pool_data_t);   
    m = ngx_align_ptr(m, NGX_ALIGNMENT);
    new->d.last = m + size; /*指向数据区域的结尾*/

    current = pool->current;    /*指向当前的内存池*/
    /*遍历到内存池链表的末尾*/
    for (p = current; p->d.next; p = p->d.next) {
        if (p->d.failed++ > 4) {    /*4是一个经验值*/
            current = p->d.next;    /*如果当前的内存池内存分配失败次数 >4, 那么使用下一个内存池，并且failed++*/
        }
    }

    p->d.next = new;    /*内存池末尾指向新开辟的内存池节点*/

    pool->current = current ? current : new;        /*current指向当前可用的内存池*/

    return m;       /*返回新的内存池的地址*/
}


static void *
ngx_palloc_large(ngx_pool_t *pool, size_t size)
{
    void              *p;
    ngx_uint_t         n;
    ngx_pool_large_t  *large;

    p = ngx_alloc(size, pool->log);
    if (p == NULL) {
        return NULL;
    }

    n = 0;

    for (large = pool->large; large; large = large->next) {
        if (large->alloc == NULL) {
            large->alloc = p;
            return p;
        }

        if (n++ > 3) {  /*大存储块不超过3个*/
            break;
        }
    }

    large = ngx_palloc(pool, sizeof(ngx_pool_large_t)); /*重新开辟一个大存储块*/
    if (large == NULL) {
        ngx_free(p);
        return NULL;
    }

    large->alloc = p;
    large->next = pool->large;
    pool->large = large;

    return p;
}

/*为什么这里也构造一个对齐分配的函数??todo*/
void *
ngx_pmemalign(ngx_pool_t *pool, size_t size, size_t alignment)
{
    void              *p;
    ngx_pool_large_t  *large;

    p = ngx_memalign(alignment, size, pool->log);
    if (p == NULL) {
        return NULL;
    }

    large = ngx_palloc(pool, sizeof(ngx_pool_large_t));
    if (large == NULL) {
        ngx_free(p);
        return NULL;
    }

    large->alloc = p;
    large->next = pool->large;
    pool->large = large;

    return p;
}


ngx_int_t
ngx_pfree(ngx_pool_t *pool, void *p)
{
    ngx_pool_large_t  *l;

    for (l = pool->large; l; l = l->next) {     /*大存储块*/
        if (p == l->alloc) {                     /*TODO*/
            ngx_log_debug1(NGX_LOG_DEBUG_ALLOC, pool->log, 0,
                           "free: %p", l->alloc);
            ngx_free(l->alloc);
            l->alloc = NULL;

            return NGX_OK;
        }
    }

    return NGX_DECLINED;
}


void *
ngx_pcalloc(ngx_pool_t *pool, size_t size) /*分配了, 还附带了清零*/
{
    void *p;

    p = ngx_palloc(pool, size);
    if (p) {
        ngx_memzero(p, size);
    }

    return p;
}


ngx_pool_cleanup_t *
ngx_pool_cleanup_add(ngx_pool_t *p, size_t size)
{
    ngx_pool_cleanup_t  *c;

    c = ngx_palloc(p, sizeof(ngx_pool_cleanup_t));
    if (c == NULL) {
        return NULL;
    }

    if (size) {
        c->data = ngx_palloc(p, size);
        if (c->data == NULL) {
            return NULL;
        }

    } else {
        c->data = NULL;
    }

    c->handler = NULL;
    c->next = p->cleanup;

    p->cleanup = c;

    ngx_log_debug1(NGX_LOG_DEBUG_ALLOC, p->log, 0, "add cleanup: %p", c);

    return c;
}


void
ngx_pool_run_cleanup_file(ngx_pool_t *p, ngx_fd_t fd)
{
    ngx_pool_cleanup_t       *c;
    ngx_pool_cleanup_file_t  *cf;

    for (c = p->cleanup; c; c = c->next) {               /*遍历清理函数*/
        if (c->handler == ngx_pool_cleanup_file) {      /*关闭文件回调函数*/

            cf = c->data;

            if (cf->fd == fd) {
                c->handler(cf);
                c->handler = NULL;
                return;
            }
        }
    }
}


void
ngx_pool_cleanup_file(void *data) /*关闭文件句柄*/
{
    ngx_pool_cleanup_file_t  *c = data;

    ngx_log_debug1(NGX_LOG_DEBUG_ALLOC, c->log, 0, "file cleanup: fd:%d",
                   c->fd);

    if (ngx_close_file(c->fd) == NGX_FILE_ERROR) {
        ngx_log_error(NGX_LOG_ALERT, c->log, ngx_errno,
                      ngx_close_file_n " \"%s\" failed", c->name);
    }
}


void
ngx_pool_delete_file(void *data)
{
    ngx_pool_cleanup_file_t  *c = data;

    ngx_err_t  err;

    ngx_log_debug2(NGX_LOG_DEBUG_ALLOC, c->log, 0, "file cleanup: fd:%d %s",
                   c->fd, c->name);

    if (ngx_delete_file(c->name) == NGX_FILE_ERROR) {
        err = ngx_errno;

        if (err != NGX_ENOENT) {
            ngx_log_error(NGX_LOG_CRIT, c->log, err,
                          ngx_delete_file_n " \"%s\" failed", c->name);
        }
    }

    if (ngx_close_file(c->fd) == NGX_FILE_ERROR) {
        ngx_log_error(NGX_LOG_ALERT, c->log, ngx_errno,
                      ngx_close_file_n " \"%s\" failed", c->name);
    }
}


#if 0

static void *
ngx_get_cached_block(size_t size)
{
    void                     *p;
    ngx_cached_block_slot_t  *slot;

    if (ngx_cycle->cache == NULL) {
        return NULL;
    }

    slot = &ngx_cycle->cache[(size + ngx_pagesize - 1) / ngx_pagesize];

    slot->tries++;

    if (slot->number) {
        p = slot->block;
        slot->block = slot->block->next;
        slot->number--;
        return p;
    }

    return NULL;
}

#endif
