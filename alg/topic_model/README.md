# Descripton
gLDA is a paralleled C++ implementation of Latent Dirichlet Allocation (LDA) using Gibbs Sampling technique for parameter estimation.

# Usage
1. Enter Paracel's home directory  
```cd paracel;``` 
2. Generate test dataset, size of 1000 documents.    
```python tool/datagen.py -m lda -n 1000 -o data.txt```
3. Set up link library path:  
```export LD_LIBRARY_PATH=your_paracel_install_path/lib```  
4. Create a json file named `cfg.json`, see example in [Parameters](#parameters) section below.
5. Run (`100` workers, `20` servers, mesos mode in the following example)  
  ```./prun.py -w 100 -p 20 -c cfg.json -m mesos your_paracel_install_path/bin/gLDA```

# Parameters
Default parameters are set in a JSON format file. For example, we create a cfg.json as below(modify `your_paracel_install_path`):


{    
    "input" : "data.txt",    
    "output" : "/your_output_path/model",    
    "alpha" : 0.1,    
    "beta" : 0.1,    
    "k_topics": 10,    
    "iters": 100,    
    "top_words": 50,     
    "update_lib" : "your_paracel_install_path/lib/libgLDA_update.so",    
    "debug": false    
 }


In the above configuration file:

* `alpha` and `beta` are hyper-parameter which control topics distribution from documents and words distribution from topics. 
* `k_topics` is the number of topics, `iters` is the number of Gibbs sampling iterations. 
* `top_words` is the number of most likely words for each topic. 
* If `debug` mode is enabled, more information such as `Log Likelihood` are displayed, but the time and memory of computing are increased significantly.

# Data Format
## Input Format
Each line is a document, words are spilted by `\t` or space. 

For example, either `2 19 4 24 9 3 2 2 9 2 2 1 24 24 2 1 1 9 3 24 0 3 2 4 0 0` or `this is a document words are splited by spaces` is valid.
## Output Format
Each line is a topic, which contains `top_words` most likely words of each topic. Words are sorted decently by probability.

For example, `0topic 2:0.202521|17:0.202118|22:0.199597|7:0.196369|12:0.195563`

# Notes
1. The documents and words here are abstract and should not only be understood as normal text documents. Also, keep in mind that we should first preprocess the data (removing stop words and rare words, stemming, etc.) before estimating with gLDA.
2. Data generated from `tool/datagen.py` are toy data. words are sampled from 10 topics and vocabulary is 5 * 5 = 25, which describes the words distribution matrix from topics. So the output will be rows or columns in distribution matrix. See references for detail.

# Reference
1. David M. Blei, Andrew Y. Ng, Michael I. Jordan. Latent dirichlet allocation. the Journal of machine Learning research, 2003.
2. Thomas L. Griffiths, and Mark Steyvers. Finding scientific topics. In PNAS 2004.
3. David Newman, Arthur Asuncion, Padhraic Smyth, Max Welling. Distributed Inference for Latent Dirichlet Allocation. Advances in neural information processing systems, 2007.
