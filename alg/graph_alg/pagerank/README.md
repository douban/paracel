# Description
PageRank is an algorithm used by Google Search to rank websites in their search engine results. PageRank was named after Larry Page, one of the founders of Google. PageRank is a way of measuring the importance of website pages. 

# Usage
1. Enter Paracel's home directory  
```cd paracel;``` 
2. Generate training dataset 
```python ./tool/datagen.py -m pagerank -o pr.dat```
3. Set up link library path:  
```export LD_LIBRARY_PATH=your_paracel_install_path/lib```    
4. Create a json file named `cfg.json`, see example in [Parameters](#parameters) section below.  
5. Run (8 workers, local mode in the following example)  
```./prun.py -w 4 -p 4 -c cfg.json -m local your_paracel_install_path/bin/pagerank```

# Parameters
Default parameters are set in a JSON format file. For example, we create a cfg.json as below(modify `your_pa   racel_install_path`):  

{    
    "input" : "pr_data.csv",    
    "output" : "./pagerank_result/",    
    "rounds" : 8,    
    "damping_factor" : 0.85,    
    "handle_file" : "your_paracel_install_path/lib/libpagerank_handle.so",    
    "update_function" : "init_updater",    
    "filter_function" : "pr_filter"    
}    

In the configuration file, `damping_factor` refers to the damping factor which is set to 0.85. `handle_file`, `update_function`and `filter_function` stores the information of registry function needed in the implementation of pagerank algorithm. `rounds` refers to the number of training iterations.

# Data Format
## Input
webpage1,webpage2    
webpage1,webpage3    
webpage2,webpage4    
...    

Each line represents a linking relation between the first column and the second column.

## Output
webpage1 rank_value    
webpage2 rank_value    
webpage3 rank_value    
...    
