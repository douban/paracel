# Descripton
Calculate the similarities between items. Each item is represented as a k-dimensional dense vector.

# Usage
1. Enter Paracel's home directory  
```cd paracel;``` 
2. Generate dataset   
```python ./tool/datagen.py -m similarity -n 10000 -k 80 -o training.dat```
3. Set up link library path:  
```export LD_LIBRARY_PATH=your_paracel_install_path/lib```  
4. Create a json file named `cfg.json`, see example in [Parameters](#parameters) section below.
5. Run (10 workers, mesos mode in the following example)   
```./prun.py -w 10 -p 1 -c cfg.json -m mesos your_paracel_install_path/bin/sim_dense```

# Parameters
Default parameters are set in a JSON format file. For example, we create a cfg.json as below(modify `your_paracel_install_path`):

{    
    "input" : "training.dat",    
    "output" : "./sim_dense_output/",    
    "simbar" : 0.2,    
    "topk" : 10    
}    
`simbar` refers to the minimum similarity bound below which items will be filtered. `topk` keep k items with largest k similarities.

# Data Format
## Input
item_id1,feature1,feature2,...,featurek    
item_id2,feature1,feature2,...,featurek    
...    

## Output
item_id1 item_id2:sim|item_id3:sim...   
item_id2 item_id3:sim|item_id4:sim...   
Each line represents the k most similar items and corresponding simiarity values with each item.

# AB Cosine Similarity
If you want to calculate ab cosine similarities, here we offer you another executable file named ab_sim_dense.
The only difference is the configuration file. For example, we create a ab_cos_cfg.json as below:

{    
    "input_a" : "training1.dat",        
    "input_b" : "training2.dat",        
    "output" : "./sim_dense_output/",       
    "simbar" : 0.2,       
    "topk" : 10           
}        
