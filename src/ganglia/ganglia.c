#include <zfsmon.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    // zfsmon_zpool_t *pools;

    // zfsmon_zpool_init(&pools);

    // const zfsmon_zpool_t *zp = zfsmon_zpool_begin(pools);
    // for (; zp != zfsmon_zpool_end(pools); zp = zfsmon_zpool_next(zp))
    // {
    //     zfsmon_print_zpool_props(zp);
    // }

    // zfsmon_zpool_fini(&pools);

    zfsmon_t mon;
    int i, cnt = 1;

    if (argc > 1) {
        cnt = atoi(argv[1]);
    }

    zfsmon_init(&mon);

    zfsmon_db_dump(&mon);
    zfsmon_zpool_iostat_print(&mon);

    for (i = 0; i < cnt - 1; i++) {
        sleep(1);
        zfsmon_update(&mon);
        zfsmon_db_dump(&mon);
        zfsmon_zpool_iostat_print(&mon);
    }

    zfsmon_fini(&mon);
    return 0;
}

