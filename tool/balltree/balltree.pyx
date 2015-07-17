# distutils: language = c++

from libcpp.vector cimport vector
from libcpp.string cimport string
from libcpp.pair cimport pair
from libcpp.unordered_set cimport unordered_set
from cython.operator cimport dereference as deref

cdef extern from "balltree.hpp" namespace "paracel":
    cdef cppclass query:
        query(vector[double]) except +
        query(vector[double], int) except +
        query(unordered_set[long], vector[double], int) except +
        query(unordered_set[long], unordered_set[long], vector[double], int) except +
        int get_k()
        vector[double] get_item()
        unordered_set[long] get_blacklist()
        unordered_set[long] get_whitelist()

cdef unordered_set[long] default_blist 
cdef unordered_set[long] default_wlist 

cdef class PyQuery:
    cdef query *thisptr
    def __cinit__(self, vector[double] item, int k = 0, unordered_set[long] black_list = default_blist, unordered_set[long] white_list = default_wlist):
        if k == 0 and len(black_list) == 0 and len(white_list) == 0:
            self.thisptr = new query(item)
        elif len(black_list) == 0 and len(white_list) == 0:
            self.thisptr = new query(item, k)
        elif len(white_list) == 0:
            self.thisptr = new query(black_list, item, k)
        else:
            self.thisptr = new query(white_list, black_list, item, k)
    def __dealloc__(self):
        del self.thisptr
    def get_k(self):
        return self.thisptr.get_k()
    def get_item(self):
        return self.thisptr.get_item()
    def get_blacklist(self):
        return self.thisptr.get_blacklist()
    def get_whitelist(self):
        return self.thisptr.get_whitelist()

cdef extern from "balltree.hpp" namespace "paracel":
    cdef cppclass balltree[double]:
        balltree(vector[vector[double]]) except +
        void build()
        void pickle(string)
        void build_from_file(string)

cdef class PyBalltree:
    cdef balltree *thisptr
    def __cinit__(self, vector[vector[double]] items):
        self.thisptr = new balltree(items)
    def __dealloc__(self):
        del self.thisptr
    def build(self):
        self.thisptr.build()
    def pickle(self, fn):
        self.thisptr.pickle(fn)
    def build_from_file(self, fn):
        self.thisptr.build_from_file(fn)

cdef extern from "balltree.hpp" namespace "paracel":
    int search(query & q, balltree & stree, vector[long] & result)
    int search(query & q, const vector[vector[double]] & buf, vector[long] & result)
    int search(query & q, balltree & stree, vector[pair[long, double]] & result)
    int search(query & q, vector[vector[double]] & buf, vector[pair[long, double]] & result)

def balltree_search(PyQuery q, PyBalltree stree):
    cdef vector[long] result
    search(deref(q.thisptr), deref(stree.thisptr), result)
    return result

def linear_search(PyQuery q, const vector[vector[double]] buf):
    cdef vector[long] result
    search(deref(q.thisptr), buf, result)
    return result

def balltree_search_with_dp(PyQuery q, PyBalltree stree):
    cdef vector[pair[long, double]] result
    search(deref(q.thisptr), deref(stree.thisptr), result)
    return result

def linear_search_with_dp(PyQuery q, const vector[vector[double]] buf):
    cdef vector[pair[long, double]] result
    search(deref(q.thisptr), buf, result)
    return result
