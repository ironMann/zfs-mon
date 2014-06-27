/**
 * Copyright (c) 2014, Gvozden Neskovic <neskovic@compeng.uni-frankfurt.de>
 */

struct zfsmon;

void zfsmon_zpool_metric_reg_health(apr_pool_t *p, const char *zpool_name, Ganglia_25metric *gmi);
void zfsmon_zpool_metric_reg_size(apr_pool_t *p, const char *zpool_name, Ganglia_25metric *gmi);
void zfsmon_zpool_metric_reg_free(apr_pool_t *p, const char *zpool_name, Ganglia_25metric *gmi);
void zfsmon_zpool_metric_reg_allocated(apr_pool_t *p, const char *zpool_name, Ganglia_25metric *gmi);
void zfsmon_zpool_metric_reg_capacity(apr_pool_t *p, const char *zpool_name, Ganglia_25metric *gmi);
void zfsmon_zpool_metric_reg_dedupratio(apr_pool_t *p, const char *zpool_name, Ganglia_25metric *gmi);
void zfsmon_zpool_metric_reg_iostat_rd(apr_pool_t *p, const char *zpool_name, Ganglia_25metric *gmi);
void zfsmon_zpool_metric_reg_iostat_wr(apr_pool_t *p, const char *zpool_name, Ganglia_25metric *gmi);

void zfsmon_zpool_metric_get_health(struct zfsmon *h, const char *zpool_name, g_val_t *val);
void zfsmon_zpool_metric_get_size(struct zfsmon *h, const char *zpool_name, g_val_t *val);
void zfsmon_zpool_metric_get_free(struct zfsmon *h, const char *zpool_name, g_val_t *val);
void zfsmon_zpool_metric_get_allocated(struct zfsmon *h, const char *zpool_name, g_val_t *val);
void zfsmon_zpool_metric_get_capacity(struct zfsmon *h, const char *zpool_name, g_val_t *val);
void zfsmon_zpool_metric_get_dedupratio(struct zfsmon *h, const char *zpool_name, g_val_t *val);
void zfsmon_zpool_metric_get_iostat_rd(struct zfsmon *h, const char *zpool_name, g_val_t *val);
void zfsmon_zpool_metric_get_iostat_wr(struct zfsmon *h, const char *zpool_name, g_val_t *val);
