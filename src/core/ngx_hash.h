
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_HASH_H_INCLUDED_
#define _NGX_HASH_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef struct {
    void             *value;
    u_short           len;
    u_char            name[1];
} ngx_hash_elt_t;


typedef struct {
    ngx_hash_elt_t  **buckets;
    ngx_uint_t        size;
} ngx_hash_t; /*hash表管理结构*/


typedef struct {
    ngx_hash_t        hash;
    void             *value;
} ngx_hash_wildcard_t; /*带有通配符的域名的hash表*/


typedef struct {
    ngx_str_t         key;
    ngx_uint_t        key_hash;/*key_hash是对key使用hash函数计算出来的值*/
    void             *value;
} ngx_hash_key_t;/*存储hash表key的数组的结构*/


typedef ngx_uint_t (*ngx_hash_key_pt) (u_char *data, size_t len);


typedef struct {
    ngx_hash_t            hash;     /*普通hash表*/   
    ngx_hash_wildcard_t  *wc_head;/*包含前向通配符的hash表*/
    ngx_hash_wildcard_t  *wc_tail;/*包含后向通配符的hash表*/
} ngx_hash_combined_t;/*组合类型hash表*/


typedef struct {
    ngx_hash_t       *hash;/*该字段指向hash表*/
    ngx_hash_key_pt   key;/*指向从字符串生成hash值的hash函数。nginx的源代码中提供了默认的实现函数ngx_hash_key_lc*/

    ngx_uint_t        max_size;/*hash表中的桶的个数*/
    ngx_uint_t        bucket_size;/*每个桶的最大限制大小，单位是字节*/

    char             *name;/*该hash表的名字*/
    ngx_pool_t       *pool;/*该hash表分配内存使用的pool*/
    ngx_pool_t       *temp_pool;/*该hash表使用的临时pool，在初始化完成以后，该pool可以被释放和销毁掉*/
} ngx_hash_init_t;


#define NGX_HASH_SMALL            1
#define NGX_HASH_LARGE            2

#define NGX_HASH_LARGE_ASIZE      16384
#define NGX_HASH_LARGE_HSIZE      10007

#define NGX_HASH_WILDCARD_KEY     1
#define NGX_HASH_READONLY_KEY     2


typedef struct {
    ngx_uint_t        hsize;/*将要构建的hash表的桶的个数*/

    ngx_pool_t       *pool;
    ngx_pool_t       *temp_pool;

    ngx_array_t       keys;/*存放所有非通配符key的数组*/
    ngx_array_t      *keys_hash;/*这是个二维数组，第一个维度代表的是bucket的编号，那么keys_hash[i]中存放的是所有的key算出来的hash值对hsize取模以后的值为i的key*/

    ngx_array_t       dns_wc_head;/*放前向通配符key被处理完成以后的值*/
    ngx_array_t      *dns_wc_head_hash;/*该值在调用的过程中用来保存和检测是否有冲突的前向通配符的key值，也就是是否有重复*/

    ngx_array_t       dns_wc_tail;/*存放后向通配符key被处理完成以后的值*/
    ngx_array_t      *dns_wc_tail_hash;/*该值在调用的过程中用来保存和检测是否有冲突的后向通配符的key值，也就是是否有重复*/
} ngx_hash_keys_arrays_t;


typedef struct {
    ngx_uint_t        hash; 
    ngx_str_t         key;
    ngx_str_t         value;
    u_char           *lowcase_key;
} ngx_table_elt_t;                      /*headers链表的元素*/


void *ngx_hash_find(ngx_hash_t *hash, ngx_uint_t key, u_char *name, size_t len);
void *ngx_hash_find_wc_head(ngx_hash_wildcard_t *hwc, u_char *name, size_t len);
void *ngx_hash_find_wc_tail(ngx_hash_wildcard_t *hwc, u_char *name, size_t len);
void *ngx_hash_find_combined(ngx_hash_combined_t *hash, ngx_uint_t key,
    u_char *name, size_t len);

ngx_int_t ngx_hash_init(ngx_hash_init_t *hinit, ngx_hash_key_t *names,
    ngx_uint_t nelts);
ngx_int_t ngx_hash_wildcard_init(ngx_hash_init_t *hinit, ngx_hash_key_t *names,
    ngx_uint_t nelts);

#define ngx_hash(key, c)   ((ngx_uint_t) key * 31 + c)
ngx_uint_t ngx_hash_key(u_char *data, size_t len);
ngx_uint_t ngx_hash_key_lc(u_char *data, size_t len);
ngx_uint_t ngx_hash_strlow(u_char *dst, u_char *src, size_t n);


ngx_int_t ngx_hash_keys_array_init(ngx_hash_keys_arrays_t *ha, ngx_uint_t type);
ngx_int_t ngx_hash_add_key(ngx_hash_keys_arrays_t *ha, ngx_str_t *key,
    void *value, ngx_uint_t flags);


#endif /* _NGX_HASH_H_INCLUDED_ */
