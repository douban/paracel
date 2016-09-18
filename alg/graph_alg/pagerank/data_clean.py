#! /usr/bin/env python

# make sure no duplicates exist
# create 'node v which link nothing but being linked' with v,SENTINEL
# default SENTINEL will be set with 2147483648
import sys

def clean_and_fmt(fn, fo):
    D = {}
    S0 = set([])
    S1 = set([])
    for line in fn:
        lst = line.strip().split(',')
        S0.add(lst[0])
        S1.add(lst[1])
        if len(lst) != 2:
            continue
            #print 'invalid line: ', line
        if D.get(line):
            continue
            #print 'duplicate line: ', line
        D[line] = 1
        fo.write(line)
    for item in list(S1 - S0):
        fo.write('%s,2147483648\n' % item)

if __name__ == '__main__':
    f = file(sys.argv[1])
    f_o = file(sys.argv[2], 'w')
    clean_and_fmt(f, f_o)
    f.close()
    f_o.close()
