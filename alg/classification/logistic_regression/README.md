# Description
Logistic regression, despite its name, is a linear model for classification rather than regression. Logistic regression is also known in the literature as logit regression, maximum-entropy classification (MaxEnt) or the log-linear classifier. In this model, the probabilities describing the possible outcomes of a single trial are modeled using a [logistic function](http://en.wikipedia.org/wiki/Logistic_function).

# Usage
1. Enter Paracel's home directory  
```cd paracel;``` 
2. Generate training dataset for classification
```python ./tool/datagen.py -m classification -o training.dat -n 10000 -k 100```
3. Set up link library path:  
```export LD_LIBRARY_PATH=your_paracel_install_path/lib```    
4. Create a json file named `cfg.json`, see example in [Parameters](#parameters) section below.  
5. Run (4 workers, local mode in the following example)  
```./prun.py -w 4 -p 2 -c cfg.json -m local your_paracel_install_path/bin/lr```

# Parameters
Default parameters are set in a JSON format file. For example, we create a cfg.json as below(modify `your_paracel_install_path`):

{    
    "training_input" : "training.dat",    
    "test_input" : "training.dat",    
    "predict_input" : "training.dat",    
    "output" : "./lr_result/",    
    "update_file" : "your_paracel_install_path/lib/liblr_update.so",    
    "update_func" : "lr_theta_update",    
    "method" : "ipm",    
    "rounds" : 5,    
    "alpha" : 0.001,    
    "beta" : 0.01,    
    "debug" : false 
}    

In the configuration file, `test_input` and `predict_input` is set to be the same as `training_input`, you can modify them if you have a test or predict dataset. `update_file` and `update_func` stores the information of registry function needed in our implementation of logistic regression. `rounds` refers to the number of training iterations. `alpha` refers to the learning rate of the [sgd](http://en.wikipedia.org/wiki/Stochastic_gradient_descent) algorithm while `beta` refers to the regularization parameter. There are four types of learning method you can choose with the `method` parameter:
 
 * [dgd](http://martin.zinkevich.org/publications/nips2010.pdf): distributed gradient descent learning
 * [ipm](http://research.google.com/pubs/pub36948.html): iterative parameter mixtures learning
 * [downpour](http://research.google.com/archive/large_deep_networks_nips2012.html): asynchrounous gradient descent learning
 * [agd](http://www.eecs.berkeley.edu/~brecht/papers/hogwildTR.pdf): slow asynchronous gradient descent learning

# Data Format
## Input
Training data, test data have the same format as below:    
feature1,feature2, ...,featurek,1
feature1,feature2, ...,featurek,0
feature1,feature2, ...,featurek,1
...    
Each line represents a sample containing a label in the last dimension. Predict data format is similar except that it do not contain the label dimension. But you can use the same format as training data, in this case, our program will ignore data in the last dimension.

## Output
`lr_theta_0`: weight value for each dimension.    
`pred_v_x` : predict result which stores predict label information in the last dimension of each line.    

# Notes
1. You do not need to know the theory behind all the leraning method, we recommend `ipm` method. For more information, click on their link and see reference paper below.
2. In output files, we append an extra dimension valued 1.0 in the first column.

# Reference
Hall, Keith B., Scott Gilpin, and Gideon Mann. "MapReduce/Bigtable for distributed optimization." NIPS LCCC Workshop. 2010.
