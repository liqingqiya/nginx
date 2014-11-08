
#include <ngx_config.h>
#include <ngx_core.h>



extern ngx_module_t  ngx_core_module;                   /*num: 0*/
extern ngx_module_t  ngx_errlog_module;                 /*num: 1*/
extern ngx_module_t  ngx_conf_module;                   /*num: 2*/
extern ngx_module_t  ngx_events_module;                 /*num: 3*/
extern ngx_module_t  ngx_event_core_module;             /*num: 4*/
extern ngx_module_t  ngx_epoll_module;                  /*num: 5*/
extern ngx_module_t  ngx_regex_module;                   /*num: 6*/
extern ngx_module_t  ngx_http_module;                    /*num: 7*/
extern ngx_module_t  ngx_http_core_module;              /*num: 8*/
extern ngx_module_t  ngx_http_log_module;                /*num: 9*/
extern ngx_module_t  ngx_http_upstream_module;
extern ngx_module_t  ngx_http_static_module;
extern ngx_module_t  ngx_http_autoindex_module;
extern ngx_module_t  ngx_http_index_module;
extern ngx_module_t  ngx_http_auth_basic_module;
extern ngx_module_t  ngx_http_access_module;
extern ngx_module_t  ngx_http_limit_conn_module;
extern ngx_module_t  ngx_http_limit_req_module;
extern ngx_module_t  ngx_http_geo_module;
extern ngx_module_t  ngx_http_map_module;
extern ngx_module_t  ngx_http_split_clients_module;
extern ngx_module_t  ngx_http_referer_module;
extern ngx_module_t  ngx_http_rewrite_module;
extern ngx_module_t  ngx_http_proxy_module;
extern ngx_module_t  ngx_http_fastcgi_module;
extern ngx_module_t  ngx_http_uwsgi_module;
extern ngx_module_t  ngx_http_scgi_module;
extern ngx_module_t  ngx_http_memcached_module;
extern ngx_module_t  ngx_http_empty_gif_module;
extern ngx_module_t  ngx_http_browser_module;
extern ngx_module_t  ngx_http_upstream_ip_hash_module;
extern ngx_module_t  ngx_http_upstream_least_conn_module;
extern ngx_module_t  ngx_http_upstream_keepalive_module;
extern ngx_module_t  ngx_http_write_filter_module;
extern ngx_module_t  ngx_http_header_filter_module;
extern ngx_module_t  ngx_http_chunked_filter_module;
extern ngx_module_t  ngx_http_range_header_filter_module;
extern ngx_module_t  ngx_http_gzip_filter_module;
extern ngx_module_t  ngx_http_postpone_filter_module;
extern ngx_module_t  ngx_http_ssi_filter_module;
extern ngx_module_t  ngx_http_charset_filter_module;
extern ngx_module_t  ngx_http_userid_filter_module;
extern ngx_module_t  ngx_http_headers_filter_module;
extern ngx_module_t  ngx_http_copy_filter_module;
extern ngx_module_t  ngx_http_range_body_filter_module;
extern ngx_module_t  ngx_http_not_modified_filter_module;

ngx_module_t *ngx_modules[] = {
    &ngx_core_module,       /*core类型模块*/
    &ngx_errlog_module,     /*core类型模块*/
    &ngx_conf_module,       /*conf类型模块*/
    &ngx_events_module,     /*core类型模块*/
    &ngx_event_core_module, /*event类型模块*/
    &ngx_epoll_module,      /*event类型模块*/
    &ngx_regex_module,      /**/
    &ngx_http_module,       /*core类型模块*/
    &ngx_http_core_module,  /*http类型模块*/
    &ngx_http_log_module,         /*NGX_HTTP_LOG_PHASE阶段*/
    &ngx_http_upstream_module,
    &ngx_http_static_module,      /*NGX_HTTP_CONTENT_PHASE阶段*/
    &ngx_http_autoindex_module,   /*NGX_HTTP_CONTENT_PHASE阶段*/
    &ngx_http_index_module,        /*NGX_HTTP_CONTENT_PHASE阶段*/
    &ngx_http_auth_basic_module,      /*NGX_HTTP_ACCESS_PHASE阶段*/
    &ngx_http_access_module,           /*NGX_HTTP_ACCESS_PHASE阶段*/
    &ngx_http_limit_conn_module,            /*NGX_HTTP_PREACCESS_PHASE阶段*/
    &ngx_http_limit_req_module,             /*NGX_HTTP_PREACCESS_PHASE阶段*/
    &ngx_http_geo_module,
    &ngx_http_map_module,
    &ngx_http_split_clients_module,
    &ngx_http_referer_module,
    &ngx_http_rewrite_module,         /*NGX_HTTP_REWRITE_PHASE | NGX_HTTP_SERVER_REWRITE_PHASE阶段*/
    &ngx_http_proxy_module,
    &ngx_http_fastcgi_module,
    &ngx_http_uwsgi_module,
    &ngx_http_scgi_module,
    &ngx_http_memcached_module,
    &ngx_http_empty_gif_module,
    &ngx_http_browser_module,
    &ngx_http_upstream_ip_hash_module,
    &ngx_http_upstream_least_conn_module,
    &ngx_http_upstream_keepalive_module,
    &ngx_http_write_filter_module,          /* filter body step: 8 */
    &ngx_http_header_filter_module,         /* filter header step:9 */
    &ngx_http_chunked_filter_module,        /* filter header step:8 & body step:7 */
    &ngx_http_range_header_filter_module,   /* filter header step:7 */
    &ngx_http_gzip_filter_module,           /* filter header step:6 & body step:6 */
    &ngx_http_postpone_filter_module,       /* filter body step: 5 */
    &ngx_http_ssi_filter_module,            /* filter header step:5 & body step:4 */
    &ngx_http_charset_filter_module,        /* filter header step:4 & body step:3 */
    &ngx_http_userid_filter_module,         /* filter header step:3 */
    &ngx_http_headers_filter_module,        /* filter header step:2 */
    &ngx_http_copy_filter_module,           /* filter body step:2 */
    &ngx_http_range_body_filter_module,    /* filter body step:1 */
    &ngx_http_not_modified_filter_module,  /* filter header step:1 */
    NULL
};

