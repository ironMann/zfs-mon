/**
 * Copyright (c) 2014, Gvozden Neskovic <neskovic@compeng.uni-frankfurt.de>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <zfsmon.h>

int main(int argc, char **argv)
{

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
