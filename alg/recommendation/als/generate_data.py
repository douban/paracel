#! /usr/bin/env python
# generate new training data
#   uid,iid,r - mu - bi
#   iid,1.|pi...

import glob
from dpark import DparkContext

MU_PATH = '/nfs/wuhong/offline_use/global_params_0'
IBIAS_PATH = '/nfs/wuhong/offline_use/ibias_0/'
RATING_PATH = '/nfs/wuhong/fm_data/user_music_factor_model/user_track_rating_for_training/'
ITEM_FACTOR_PATH = '/nfs/wuhong/offline_use/H_0/'

NEW_RATING_PATH = '/nfs/wuhong/offline_use/rating_new/'
NEW_ITEM_FACTOR_PATH = '/nfs/wuhong/offline_use/H_new/'

dpark = DparkContext()

f_global = file(MU_PATH)
sline = ''
for l in f_global:
    sline = l
mu = float(sline.strip().split('\t')[1])
f_global.close()
mu = dpark.broadcast(mu)

def local_mapper(line):
    iid, v, _ = line.strip().split('\t')
    return (iid, float(v))

ibias = {}
ibias = dpark.textFile(glob.glob(IBIAS_PATH)).map(
    local_mapper
    ).collectAsMap()
ibias = dpark.broadcast(ibias)

def local_mapper2(line):
    uid, iid, _, v = line.strip().split('\t')
    return '%s,%s,%s\n' % (uid, iid, float(v) - mu - ibias[iid])

# generate new rating data
dpark.textFile(glob.glob(RATING_PATH)).filter(
    lambda line: ibias.get(line.strip().split('\t')[1])
    ).map(
        local_mapper2
    ).saveAsTextFile(NEW_RATING_PATH)

def local_mapper3(line):
    iid, ifac = line.strip().split('\t')
    fmt_ifac = '1.0|' + ifac
    return '%s\t%s\n' % (iid, fmt_ifac)

# generate new item factor data
dpark.textFile(glob.glob(ITEM_FACTOR_PATH)).map(
    local_mapper3
    ).saveAsTextFile(NEW_ITEM_FACTOR_PATH)
