Paracel toolkits also contains some sequential algorithms for data processing, some of them are corresponding to the parallel version in the `alg` directory.

# Lasso

```python ./tool/datagen.py -m regression -k 100 -n 10000 -o training.dat```    
```export LD_LIBRARY_PATH=your_paracel_install_path/lib```    
```your_paracel_install_path/bin/tool/lasso_serial --cfg_file cfg.json```   

cfg.json file example:
{    
    "input", : "training.dat",    
    "output" : "./lasso_result/",    
    "lambda" : 0.001,    
    "rounds" : 2000     
}    

`lambda` refers to learning rate while `rounds` refers to the number of training iterations.

# Logistic_regression_l1

```python ./tool/datagen.py -m classification -o training.dat -n 2500 -k 100```    
```export LD_LIBRARY_PATH=your_paracel_install_path/lib```    
```your_paracel_install_path/bin/tool/lr_l1_serial --cfg_file cfg.json```   

cfg.json file example:
{    
    "input", : "training.dat",    
    "output" : "./lr_l1_result/",    
    "lambda" : 0.001,    
    "rounds" : 2000     
}    

# svd

```python ./tool/datagen.py -m svd -o svd.dat```    
```export LD_LIBRARY_PATH=your_paracel_install_path/lib```    
```your_paracel_install_path/bin/tool/svd_serial --cfg_file cfg.json```   

cfg.json file example:
{    
    "input", : "svd.dat",    
    "output" : "./svd_result/",    
    "k" : 3    
}    
