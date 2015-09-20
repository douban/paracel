#! /bin/sh
python setup.py build_ext --inplace
g++ -pthread -fPIC -I/home/xunzhang/Desktop/Github/paracel/include -I/usr/include/python2.7 -c balltree.cpp -std=c++11 -o build/temp.linux-x86_64-2.7/balltree.o
g++ -pthread -shared -I/home/xunzhang/Desktop/Github/paracel/include build/temp.linux-x86_64-2.7/balltree.o -L/usr/lib64 -lpython2.7 -o balltree.so
