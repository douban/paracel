# Descripton
Alternating least square solver for user/item factors. The current version only fits for the case that number of users is much bigger than number of items, or number of items is much bigger than number of users. For example, user size is 10000000 and item size is 1000000, it could solve user factor using als method.

# Usage
1. Enter Paracel's home directory  
```cd paracel;``` 
2. Generate dataset   
```python ./tool/datagen.py -m als -o ./data/```
3. Set up link library path: 
```export LD_LIBRARY_PATH=your_paracel_install_path/lib``` 
4. Create a json file named `cfg.json`, see example in [Parameters](#parameters) section below.
5. Run (100 workers, 20 servers, mesos mode in the following example)   
```./prun.py -w 100 -p 20 -c cfg.json -m mesos --ppn 10 --mem_limit 1000 your_paracel_install_path/bin/als```

# Parameters
Default parameters are set in a JSON format file. For example, we create a cfg.json as below(modify `your_paracel_install_path`):

{    
    "rating_input" : "./data/als_rating.dat",    
    "factor_input" : "./data/als_H.dat",    
    "output" : "./als_result/",   
    "pattern" : "fmap",    
    "lambda" : 0.1        
}    

# Data Format
## Input
`als_rating.dat`: small training dataset from netflix movie rating data, each line presents a tetrad with "user_id,movie_id,rating".

`als_H.dat`: movie factor input data

## Output
`W_0`: user factor output data
