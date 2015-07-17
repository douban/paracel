# 编译安装
如果是C++程序，可以直接`#include "balltree.hpp"`
如果是Python用户，需要先编译出`balltree.so`: `CXX=g++-4.7.2 ./compile_cython.sh`

# Python使用
见脚本`usage.py`

## 注意 
1. 定义`stree`对象的时候需要带参数`ivector_list`。
2. `stree.build`的时候默认下标从`0`开始到`len(ivector_list)-1`，需要自己做坐标对应
3. black_ids和white_ids均需要定义，如果为空则默认不开启黑白名单。

# C++使用
见`test_balltree.cc`

## 注意
1. 定义`stree`对象的时候需要带参数`ivector_list`。
2. `stree.build`的时候默认下标从`0`开始到`ivector_list.size()-1`，需要自己做坐标对应。
3. black_ids和white_ids均需要定义，如果为空则默认不开启黑白名单。

# 其他说明
1. 对于大数据，`build`时间可能比较长，因此可以拆成`pickle`（线下）和`build_from_file`（线上）来建树。
2. Python的性能比C++略慢，但是可以接受。对于百万量级的item建树Python可以在线上直接用（如100w个100维向量单次查询约150-200ms）。

# 开发
如果你要加功能，需要先改`balltree.hpp`，然后改对应的Cython，最后重新编译成`balltree.so`。
