#!/usr/bin/env python

import os, sys, time, errno
import shelve
import glob
import subprocess
import csv

"""Fix python3's long."""
if sys.version > '3':
    long = int

"""Set DEBUG flag if passed as the first argument."""
DEBUG = True if len(sys.argv) == 2 and str(sys.argv[1]) == 'debug' else False

"""Tools and files used."""
IOSTAT_DB   = '/var/lib/zfs-mon/zpool_iostat.pydb'
ZPOOL_CMD   = '/usr/sbin/zpool'
GMETRIC_CMD = '/usr/bin/gmetric'

ZFSMON_METRICS = {
    'zpool_iostat_rd':      {'units': 'B/s', 'type': 'double', 'title': 'zpool %s : read iostat'},
    'zpool_iostat_wr':      {'units': 'B/s', 'type': 'double', 'title': 'zpool %s : write iostat'},
    'zpool_prop_health':    {'units': '',    'type': 'string', 'title': 'zpool %s : health'},
    'zpool_prop_capacity':  {'units': '',    'type': 'string', 'title': 'zpool %s : capacity'},
    'zpool_prop_size':      {'units': 'B',   'type': 'double', 'title': 'zpool %s : size'},
    'zpool_prop_free':      {'units': 'B',   'type': 'double', 'title': 'zpool %s : free space'},
    'zpool_prop_allocated': {'units': 'B',   'type': 'double', 'title': 'zpool %s : allocated space'}
}

"""
Ganglia gmetric tool helpers
"""
devnull = open(os.devnull, "w")
gmetric_pids = []


def gmetric(zpool, mname, mvalue, mslope='both'):
    """Calls gmetric tool asynchronously."""
    global gmetric_pids
    cmd = [GMETRIC_CMD,
           '--name=zfsmon_%(z)s_%(m)s' % {'z': zpool, 'm': mname},
           '--type=%s' % ZFSMON_METRICS[mname]['type'],
           '--units=%s' % ZFSMON_METRICS[mname]['units'],
           '--value=%s' % mvalue,
           '--slope=%s' % mslope,
           '--title=%s' % (ZFSMON_METRICS[mname]['title'] % zpool),
           '--group=zfsmon',
           '--dmax=600']
    if DEBUG: print ' '.join(cmd)
    gmetric_pids.append(subprocess.Popen(cmd, stdout=devnull, stderr=devnull))


def gmetric_wait():
    """waits for all gmetric processes to finish."""
    global gmetric_pids
    [p.wait() for p in gmetric_pids]


"""
ZFS iostat database methods.
"""


def save_iostat(iodb, zpool_name, stats):
    """Saves current io values for the zpool."""
    iodb[zpool_name] = stats
    iodb.sync()


def load_iostat(iodb, zpool_name):
    """Loads io values for the zpool.

    In case that zpool is not found, return 0s.
    """
    try:
        return iodb[zpool_name]
    except KeyError:
        return {'timestamp': long(0), 'nread': long(0), 'nwritten': long(0)}


def clean_iostat(iodb, zpools):
    """Cleans iostat db."""
    for key in set(iodb.keys()) - set(zpools):
        del iodb[key]
    iodb.sync()


"""
Check if all tools and permissions are set.
"""

"""Check if gmetric tool exists."""
if not os.access(GMETRIC_CMD, os.F_OK | os.X_OK):
    print >> sys.stderr, 'error: gmetric tool not found:', GMETRIC_CMD
    sys.exit(errno.ENOENT)


do_iostat = True
do_props = True
"""Check permissions."""
"""iostat db access rights."""
if not os.path.exists(os.path.dirname(IOSTAT_DB)):
    try:
        os.makedirs(os.path.dirname(IOSTAT_DB))
    except:
        pass

"""Check if iostat db directory is writable."""
if do_iostat and not os.access(os.path.dirname(IOSTAT_DB), os.W_OK):
    do_iostat = False

"""Check if iostat db file exists and is WD|WR for us."""
if do_iostat and os.access(IOSTAT_DB, os.F_OK) and not os.access(IOSTAT_DB, os.W_OK | os.R_OK):
    do_iostat = False

"""Check if zpool tools exist."""
if do_props and not os.access(ZPOOL_CMD, os.F_OK | os.X_OK):
    do_props = False


"""
ZFS pools iostat reporting.
"""

if do_iostat:
    iostatdb = shelve.open(IOSTAT_DB, flag='c') # Open database, creating it if necessary.
    iostat_zpools = []  # list of all zpools

    for iofile in sorted(glob.glob('/proc/spl/kstat/zfs/*/io')):
        # get the zpool name from the path, instead of '*'
        zpool_name = os.path.basename(os.path.dirname(iofile))
        iostat_zpools.append(zpool_name)
        old_iostat = load_iostat(iostatdb, zpool_name)

        # parse iostat data from /proc for the pool
        nrd, nwr = long(0), long(0)
        try:
            nrd, nwr = [long(v) for v in open(iofile, 'r').readlines()[2].split()[:2]]
        except:
            print >> sys.stderr, 'warning: iostat data parse error:', iofile
            print >> sys.stderr, 'warning: iostat data will not be sent for zpool:', zpool_name
            continue

        # persist current iostat data for use in next run
        curr_iostat = {'timestamp': time.time(), 'nread': nrd, 'nwritten': nwr}
        save_iostat(iostatdb, zpool_name, curr_iostat)

        # time scale calculation
        time_diff = (curr_iostat['timestamp'] - old_iostat['timestamp'])
        time_scale = float(1.0) if time_diff == long(0) else float(1.0)/float(time_diff)

        # iostat calculation
        rd_bw = max(time_scale * float(curr_iostat['nread'] - old_iostat['nread']), float(0.0))
        wr_bw = max(time_scale * float(curr_iostat['nwritten'] - old_iostat['nwritten']), float(0.0))

        # update metric for the current pool
        gmetric(zpool_name, "zpool_iostat_rd", long(rd_bw))
        gmetric(zpool_name, "zpool_iostat_wr", long(wr_bw))

    # clean and close the iostat database
    clean_iostat(iostatdb, iostat_zpools)
    iostatdb.close()

"""
ZFS zpool properties reporting.
"""
if do_props:
    zpool_props = {}
    zpool_get_process = subprocess.Popen([ZPOOL_CMD, 'get', '-p', 'all'], stdout=subprocess.PIPE, stderr=devnull)
    zpool_get_stdout = zpool_get_process.communicate()[0]

    if zpool_get_process.returncode == 0:

        # parse csv output returned by `zpool get -p all`
        reader = csv.DictReader(zpool_get_stdout.decode('utf8').splitlines(),
                                delimiter=' ', skipinitialspace=True)

        for row in reader:
            try:
                zpool_props[row['NAME']][row['PROPERTY']] = row['VALUE']
            except KeyError:
                zpool_props[row['NAME']] = {row['PROPERTY']: row['VALUE']}

        for zpool_name in zpool_props:
            props = zpool_props[zpool_name]

            gmetric(zpool_name, 'zpool_prop_health', props['health'])
            gmetric(zpool_name, 'zpool_prop_capacity', props['capacity'])
            gmetric(zpool_name, 'zpool_prop_size', props['size'], 'string')
            gmetric(zpool_name, 'zpool_prop_free', props['free'])
            gmetric(zpool_name, 'zpool_prop_allocated', props['allocated'])


gmetric_wait()  # wait for all calls to gmetric to finish
