import glob
from dpark import DparkContext

RATING_PATH = '/nfs/wuhong/offline_use/rating_new/'
TRAINING_PATH = '/nfs/wuhong/paracel/data/als_fm/train'
TEST_PATH = '/nfs/wuhong/paracel/data/als_fm/test'

dpark = DparkContext()

def local_filter1(line):
    tmp = line.strip().split(',')[1]
    if tmp.endswith('0') or tmp.endswith('1') or tmp.endswith('2'):
        return False
    return True

def local_filter2(line):
    tmp = line.strip().split(',')[1]
    if tmp.endswith('0') or tmp.endswith('1') or tmp.endswith('2'):
        return True
    return False

dpark.textFile(glob.glob(RATING_PATH)).filter(
    local_filter1
    ).saveAsTextFile(TRAINING_PATH)

dpark.textFile(glob.glob(RATING_PATH)).filter(
    local_filter2
    ).saveAsTextFile(TEST_PATH)
