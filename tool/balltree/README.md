# Install
If use its C++ interfaces, you could include header file from this folder directly: `#include "balltree.hpp"`
If a Python user, you need first to build out `"balltree.so"`: `CXX=g++ ./compile_cython.sh`

# Python Usage
take `usage.py` for example.

## Notice
1. In defining a `stree` object, you must specify the `ivector_list` parameter.
2. You must map the indexes yourself, from `0` to `len(ivector_list)-1`.
3. `black_ids` and `white_ids` must always be defined. Empty defined `black_ids`/`white_ids` means there are no these restrictions.

# C++ Usage
take 'test_balltree.cc' for example.

## Notice
1. In defining a `stree` object, you must specify the `ivector_list` parameter.
2. You must map the indexes yourself, from `0` to `ivector_list.size()-1`.
3. `black_ids` and `white_ids` must always be defined. Empty defined `black_ids`/`white_ids` means there are no these restrictions.

# Tips
1. Real application may be dealing with rather big data, in which case tree building time will be very long. To optimize, you should decoupler tree building process into offline `pickle` and online `build_from_file` steps.
2. Python search in this implementation of balltree is acceptablely slow than C++ case. In our usage, Python search could work efficiently online with several millions of items(single search for one million 100-dimensional vectors is about 100-200 milliseconds).

# Customize
1. Do modification in `balltree.hpp`.
2. [Optional for Python users] Modify cython wrapper in `balltree.pyx`.
3. [Optional for Python users] Rebuild `balltree.so`.
