DISTRIBUTION_INPUT = 'dis.dat'
INPUT_FILE = 'sim.dat'
OUTPUT_FILE = 'train.dat'
OUTPUT_DISTRIBUTION_FILE = 'rdis.dat'

def fmt():
    distribution_dict = {}
    f = file(DISTRIBUTION_INPUT)
    fo2 = file(OUTPUT_DISTRIBUTION_FILE, 'w')
    for line in f:
        tmp = line.strip().split(' ')
        distribution_dict[tmp[0]] = tmp[1]
        fo2.write('%s,%s\n' % (tmp[0] + 'L', float(tmp[1]) / 2))
        fo2.write('%s,%s\n' % (tmp[0] + 'R', float(tmp[1]) / 2))
    
    fi = file(INPUT_FILE)
    fo = file(OUTPUT_FILE, 'w')
    for line in fi:
        tmp = line.strip().split(',')
        #suffix = str(float(distribution_dict[tmp[0]]) / 2)
        lnode = tmp[0] + 'L'
        lst = tmp[1:]
        for v in lst:
            rnode, wgt = v.split('|')
            rnode = rnode + 'R'
            fo.write('%s,%s,%s\n' % (lnode, rnode, wgt))
        fo.write('%s,%s,%s\n' % (tmp[0] + 'R', tmp[0] + 'L', 1.0))
    fi.close()
    fo.close()
    fo2.close()

if __name__ == '__main__':
    fmt()
