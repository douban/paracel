# Description
Regression with l2 regularization term.

# Usage
1. Enter Paracel's home directory  
```cd paracel;``` 
2. Generate training dataset for classification
```python ./tool/datagen.py -m regression -o training.dat -n 2500 -k 100```
3. Set up link library path:  
```export LD_LIBRARY_PATH=your_paracel_install_path/lib```    
4. Create a json file named `cfg.json`, see example in [Parameters](#parameters) section below.  
5. Run (4 workers, local mode in the following example)  
```./prun.py -w 4 -p 2 -c cfg.json -m local your_paracel_install_path/bin/ridge_regression```

# Parameters
Default parameters are set in a JSON format file. For example, we create a cfg.json as below(modify `your_paracel_install_path`):

{    
    "training_input" : "training.dat",    
    "test_input" : "training.dat",    
    "predict_input" : "training.dat",    
    "output" : "./ridge_result/",    
    "update_file" : "your_paracel_install_path/lib/libridge_update.so",    
    "update_func" : "ridge_theta_update",    
    "rounds" : 100,    
    "alpha" : 0.001,    
    "beta" : 0.01,    
    "debug" : false     
}     
In the configuration file, `test_input` and `predict_input` is set to be the same as `training_input`, you can modify them if you have a test or predict dataset. `update_file` and `update_func` stores the information of registry function needed in our implementation of ridge regression. `rounds` refers to the number of training iterations. `alpha` refers to the learning rate of the [sgd](http://en.wikipedia.org/wiki/Stochastic_gradient_descent) algorithm while `beta` refers to the regularization parameter.
 
# Data Format
## Input
Training data, test data have the same format as below:    
feature1,feature2, ...,featurek,val
feature1,feature2, ...,featurek,val
feature1,feature2, ...,featurek,val
...    
Each line represents a sample containing a label value in the last dimension. Predict data format is similar except that it do not contain the label dimension. But you can use the same format as training data, in this case, our program will ignore data in the last dimension.

## Output
`ridge_theta_0`: weight value for each dimension.    
`pred_v_x` : predict result which stores predict label value in the last dimension of each line.    

# Notes
1. In output files, we append an extra dimension valued 1.0 in the first column.
