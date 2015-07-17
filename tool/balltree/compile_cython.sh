#! /bin/sh
python setup.py build_ext --inplace
g++-4.7.2 -pthread -fPIC -I/usr/include/python2.7 -c balltree.cpp -std=c++11 -obuild/temp.linux-x86_64-2.7/balltree.o
g++-4.7.2 -pthread -shared build/temp.linux-x86_64-2.7/balltree.o -L/usr/lib64 -lpython2.7 -o /mfs/user/wuhong/plato/online/balltree.so
