
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


ngx_array_t *
ngx_array_create(ngx_pool_t *p, ngx_uint_t n, size_t size)
{
    ngx_array_t *a;

    a = ngx_palloc(p, sizeof(ngx_array_t));     /*开辟数组内存结构*/
    if (a == NULL) {
        return NULL;
    }

    if (ngx_array_init(a, p, n, size) != NGX_OK) {  /*初始化数组内存结构*/
        return NULL;
    }

    return a;           /*返回数组结构地址*/
}


void
ngx_array_destroy(ngx_array_t *a)
{
    ngx_pool_t  *p;

    p = a->pool;        /*内存池*/

    if ((u_char *) a->elts + a->size * a->nalloc == p->d.last) {
        p->d.last -= a->size * a->nalloc;
    }

    if ((u_char *) a + sizeof(ngx_array_t) == p->d.last) {  /*回收内存， ？？？*/
        p->d.last = (u_char *) a;
    }
}


void *
ngx_array_push(ngx_array_t *a)
{
    void        *elt, *new;
    size_t       size;
    ngx_pool_t  *p;

    if (a->nelts == a->nalloc) {    /*如果数组元素个数恰好超出了容量*/

        /* the array is full */

        size = a->size * a->nalloc;     /*计算数据区域总共多少内存*/

        p = a->pool;        /*内存池*/

        if ((u_char *) a->elts + size == p->d.last  /*如果内存池的last指针指向了数组的最后一个元素*/
            && p->d.last + a->size <= p->d.end)         
        {
            /*
             * the array allocation is the last in the pool
             * and there is space for new allocation
             */

            p->d.last += a->size;  /*分配一个size大小的空间, a->size为一个元素的内存大小*/
            a->nalloc++;         /*拓展一个容量*/

        } else {
            /* allocate a new array */

            new = ngx_palloc(p, 2 * size);  /*容量拓展为以前的两倍内存空间*/
            if (new == NULL) {
                return NULL;
            }

            ngx_memcpy(new, a->elts, size);     /*复制原来的数据到新的数据区域中*/
            a->elts = new;                        /*指向新的数据区域*/  
            a->nalloc *= 2;                       /*容量为2倍*/
        }
    }

    elt = (u_char *) a->elts + a->size * a->nelts;  /*数据区中实际已经存放数据的子区的末尾*/
    a->nelts++;         /*元素个数++*/

    return elt;
}


void *
ngx_array_push_n(ngx_array_t *a, ngx_uint_t n)
{
    void        *elt, *new;
    size_t       size;
    ngx_uint_t   nalloc;
    ngx_pool_t  *p;

    size = n * a->size;

    if (a->nelts + n > a->nalloc) {     /*满了*/

        /* the array is full */

        p = a->pool;                    /*所在内存池*/

        if ((u_char *) a->elts + a->size * a->nalloc == p->d.last   /*最后一个元素的指针==内存的last指针*/
            && p->d.last + size <= p->d.end)
        {
            /*
             * the array allocation is the last in the pool
             * and there is space for new allocation
             */

            p->d.last += size;          /*移动last指针*/
            a->nalloc += n;             /*容量+=n*/

        } else {
            /* allocate a new array */

            nalloc = 2 * ((n >= a->nalloc) ? n : a->nalloc);    /*n和nalloc中取大的，拓展2倍容量*/

            new = ngx_palloc(p, nalloc * a->size);
            if (new == NULL) {
                return NULL;
            }

            ngx_memcpy(new, a->elts, a->nelts * a->size);   /*复制*/
            a->elts = new;
            a->nalloc = nalloc;
        }
    }

    elt = (u_char *) a->elts + a->size * a->nelts;
    a->nelts += n;

    return elt;
}
