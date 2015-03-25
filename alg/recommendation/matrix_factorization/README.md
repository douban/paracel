# Descripton
Decompose Matrix A(m x n) with user factor matrix W(m x k) and item factor matrix H(n x k) where A = W * H'. A is usally a sparse matrix and k is much smaller than m and n. We implement an asynchronous, distributed version of matrix factorization in Paracel.

# Usage
1. Enter Paracel's home directory  
```cd paracel;``` 
2. Generate dataset   
```python ./tool/datagen.py -m mf -o netflix.dat```
3. Set up link library path:  
```export LD_LIBRARY_PATH=your_paracel_install_path/lib```  
4. Create a json file named `cfg.json`, see example in [Parameters](#parameters) section below.
5. Run (100 workers, 20 servers, mesos mode in the following example)   
```./prun.py -w 100 -p 20 -c cfg.json -m mesos --ppn 10 --mem_limit 1000 your_paracel_install_path/bin/mf```

# Parameters
Default parameters are set in a JSON format file. For example, we create a cfg.json as below(modify `your_paracel_install_path`):

{    
    "input" : "netflix.dat",    
    "predict_input" : "netflix.dat.predict",    
    "output" : "./mf_result/",    
    "update_file" : "your_paracel_install_path/lib/libmf_update.so",    
    "update_functions" : ["cnt_updater", "mf_fac_updater", "mf_bias_updater"],    
    "filter_file" : "your_paracel_install_path/lib/libmf_filter.so",    
    "filter_functions" : ["mf_ubias_filter", "mf_ibias_filter", "mf_W_filter", "mf_H_filter"],    
    "k" : 100,    
    "rounds" : 5,    
    "alpha" : 0.001,     
    "beta" : 0.001,    
    "debug" : false,    
    "ssp_switch" : true,    
    "limit_s" : 3        
}    
`update_file`, `update_functions`, `filter_file` and `filter_functions` store the information of registry function used in the implementation of matrix factorization program. `k`refers to the factor dimension of matrix `W` and matrix `H`. `rounds` refers to the number of training iterations. `alpha` is the learning rate of [sgd](http://en.wikipedia.org/wiki/Stochastic_gradient_descent) algorithm and `beta` is the regularization parameter. If you set `ssp_swith` with true value, this means the training process will be running as asynchrounous mode. `limit_s` is also used for asynchrounous training. For example, if it equals to 3, it means that the fastest worker will lead no more than three iterations than the slowest worker. If it equals to 0, it is the classic [BSP](http://en.wikipedia.org/wiki/Bulk_synchronous_parallel) training model.

# Data Format
## Input
`netflix.dat`: training dataset from netflix movie rating data, each line presents a tetrad with "user_id movie_id date rating".

`netflix.dat.predict`: data to be predicted, each line presents a pair with "user_id,movie_id"

## Output
`miu_0`: record rating size and global mean value.

`ubias_0`: record user bias values.

`W_0`: record user factors with k dimension.

`ibias_0`: record item bias values.

`H_0`: record item factors with k dimension. 

`pred_v_x`: predict rating value of specified users and movies which are specified in `netflix.dat.predict`.

# Reference
Koren, Yehuda, Robert Bell, and Chris Volinsky. "Matrix factorization techniques for recommender systems." Computer 8 (2009): 30-37.
