#! /usr/bin/env python 
#
# Copyright (c) 2014, Douban Inc. 
#   All rights reserved. 
#
# Distributed under the BSD License. Check out the LICENSE file for full text.
#
# Paracel - A distributed optimization framework with parameter server.
#
# Downloading
#   git clone https://github.com/douban/paracel.git
#
# Authors: Hong Wu <xunzhangthu@gmail.com>
#

try:
    from sklearn import datasets
except:
    print 'sklearn module required'
    exit(0)

try:
    from optparse import OptionParser
except:
    print 'optparse module required'
    exit(0)

def dump_data(output, sample, label, sep):
    f = open(output, 'wb')
    m, n = sample.shape
    for i in xrange(m):
        for item in sample[i]:
            f.write(str(item) + sep)
        f.write(str(label[i]) + '\n')
    f.close()

def gen_cls_data(output, sz, k = 100, sep = ','):
    x, y = datasets.make_classification(sz, k)
    dump_data(output, x, y, sep)

def gen_reg_data(output, sz, k = 100, sep = ','):
    x, y = datasets.make_regression(sz, k)
    dump_data(output, x, y, sep)

def gen_sim_data(output, sz, k = 10, sep = ','):
    import numpy as np
    vl = np.random.rand(sz, k)
    f = open(output, 'wb')
    for i in vl:
        iid = np.random.randint(10000000)
        f.write(str(iid) + sep)
        for val in i[:-1]:
            f.write(str(val) + sep)
        f.write(str(i[-1]) + '\n')
    f.close()

def gen_kmeans_data(output, sz, centers, k = 10, sep = ','):
    sample, label = datasets.make_blobs(n_samples = sz, centers = centers, n_features = k)
    m, n = sample.shape
    f = open(output, 'wb')
    for i in xrange(m):
        f.write(str(i) + '\t')
        for k in xrange(len(sample[i]) - 1):
            f.write(str(sample[i][k]) + sep)
        f.write(str(sample[i][len(sample[i]) - 1]) + '\n')
    f.close()

    f = open(output + '.label', 'wb')
    for i in xrange(m):
        f.write(str(i) + '\t' + str(label[i]) + '\n')
    f.close()

if __name__ == '__main__':
    optpar = OptionParser()
    optpar.add_option('-m', '--method', action = 'store', type = 'string', dest =
                    'method', help = 'classification | regression | similarity | kmeans...')
    optpar.add_option('-o', '--out', action = 'store', type = 'string', dest = 'output')
    optpar.add_option('-s', '--sep', action = 'store', type = 'string', dest = 'sep')
    optpar.add_option('-n', '--datasize', action = 'store', type = 'int', dest = 'size')
    optpar.add_option('-k', '--nfeatures', action = 'store', type = 'int', dest = 'k')
    optpar.add_option('--ncenters', action = 'store', type = 'int', dest = 'ncenters')
    options, args = optpar.parse_args()

    # check input
    if options.method == 'classification':
		    if options.k and options.sep:
			      gen_cls_data(options.output, options.size, options.k, options.sep)
		    elif options.k:
			      gen_cls_data(options.output, options.size, options.k)
		    else:
			      gen_cls_data(options.output, options.size)
    if options.method == 'regression':
		    if options.k and options.sep:
			      gen_reg_data(options.output, options.size, options.k, options.sep)
		    elif options.k:
			      gen_reg_data(options.output, options.size, options.k)
		    else:
			      gen_reg_data(options.output, options.size)
    if options.method == 'similarity':
		    if options.k and options.sep:
			      gen_sim_data(options.output, options.size, options.k, options.sep)
		    elif options.k:
			      gen_sim_data(options.output, options.size, options.k)
		    else:
			      gen_sim_data(options.output, options.size)
    if options.method == 'kmeans':
        if options.k and options.sep:
            gen_kmeans_data(options.output, options.size, options.ncenters, options.k, options.sep)
        elif options.k:
            gen_kmeans_data(options.output, options.size, options.ncenters, options.k)
        else:
            gen_kmeans_data(options.output, options.size, options.ncenters)
