/**
 * Copyright (c) 2014, Gvozden Neskovic <neskovic@compeng.uni-frankfurt.de>
 */

#include <config.h>

#include "zfsmon.h"

#include <string.h>
#include <stddef.h>

#include <libuutil.h>
#include <libnvpair.h>

#include <libzfs.h>

#include "zpool_util.h"

typedef struct zpool_node {
    zpool_handle_t  *zn_handle;
    uu_avl_node_t   zn_avlnode;
    int     zn_mark;
} zpool_node_t;

typedef struct zpool_list {
    uu_avl_t        *zl_avl;
    uu_avl_pool_t   *zl_pool;
    zprop_list_t    **zl_proplist;
} zpool_list_t;


/*
 * Out of memory
 */
static
void no_memory(void)
{
    fprintf(stderr, "error: out of memory\n");
    exit(1);
}

static int
zpool_compare(const void *larg, const void *rarg, void *unused)
{
    zpool_handle_t *l = ((zpool_node_t *)larg)->zn_handle;
    zpool_handle_t *r = ((zpool_node_t *)rarg)->zn_handle;
    const char *lname = zpool_get_name(l);
    const char *rname = zpool_get_name(r);

    return (strcmp(lname, rname));
}

/*
 * Callback function for pool_list_get().  Adds the given pool to the AVL tree
 * of known pools.
 */
static int
add_pool(zpool_handle_t *zhp, void *data)
{
    zpool_list_t *zlp = data;
    zpool_node_t *node = xmalloc(sizeof(zpool_node_t));
    uu_avl_index_t idx;

    node->zn_handle = zhp;
    uu_avl_node_init(node, &node->zn_avlnode, zlp->zl_pool);

    if (uu_avl_find(zlp->zl_avl, node, NULL, &idx) == NULL) {
        if (zlp->zl_proplist &&
                zpool_expand_proplist(zhp, zlp->zl_proplist) != 0) {
            zpool_close(zhp);
            free(node);
            return (-1);
        }

        uu_avl_insert(zlp->zl_avl, node, idx);

    } else {
        zpool_close(zhp);
        free(node);
        return (-1);
    }

    return (0);
}

/*
 * Iterate over all pools in the list, executing the callback for each
 */
int
pool_list_iter(zpool_list_t *zlp, zpool_iter_f func, void *data)
{
    zpool_node_t *node, *next_node;
    int ret = 0;

    for (node = uu_avl_first(zlp->zl_avl); node != NULL; node = next_node) {
        next_node = uu_avl_next(zlp->zl_avl, node);
        ret |= func(node->zn_handle, data);
    }

    return (ret);
}


zpool_list_t *
pool_list_get(zprop_list_t **proplist, zfsmon_t *h)
{
    zpool_list_t *zlp;

    zlp = xmalloc(sizeof(zpool_list_t));

    zlp->zl_pool = uu_avl_pool_create("zfs_pool", sizeof(zpool_node_t),
                                      offsetof(zpool_node_t, zn_avlnode),
                                      zpool_compare, UU_DEFAULT);

    if (zlp->zl_pool == NULL) {
        no_memory();
    }

    if ((zlp->zl_avl = uu_avl_create(zlp->zl_pool, NULL,
                                     UU_DEFAULT)) == NULL) {
        no_memory();
    }

    zlp->zl_proplist = proplist;

    zpool_iter(h->zfs, add_pool, zlp);

    return (zlp);
}

/*
 * Free all the handles associated with this list.
 */
void pool_list_free(zpool_list_t *zlp)
{
    uu_avl_walk_t *walk;
    zpool_node_t *node;

    if ((walk = uu_avl_walk_start(zlp->zl_avl, UU_WALK_ROBUST)) == NULL) {
        (void) fprintf(stderr, "internal error: out of memory");
        exit(1);
    }

    while ((node = uu_avl_walk_next(walk)) != NULL) {
        uu_avl_remove(zlp->zl_avl, node);
        zpool_close(node->zn_handle);
        free(node);
    }

    uu_avl_walk_end(walk);
    uu_avl_destroy(zlp->zl_avl);
    uu_avl_pool_destroy(zlp->zl_pool);

    free(zlp);
}

int for_each_pool(zprop_list_t **proplist, zpool_iter_f func, void *data)
{
    zpool_list_t *list;
    int ret = 0;

    if ((list = pool_list_get(proplist, data)) == NULL) {
        return (1);
    }

    zpool_node_t *node, *next_node;

    for (node = uu_avl_first(list->zl_avl); node != NULL; node = next_node) {
        next_node = uu_avl_next(list->zl_avl, node);

        ret |= func(node->zn_handle, data);
    }

    pool_list_free(list);

    return (ret);
}
