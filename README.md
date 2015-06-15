![logo](/logo.png)

# Paracel Overview [![https://travis-ci.org/douban/paracel.png](https://travis-ci.org/douban/paracel.png)](https://travis-ci.org/douban/paracel)

[![Join the chat at https://gitter.im/douban/paracel](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/douban/paracel?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

Paracel is a distributed computational framework, designed for many machine learning problems: Logistic Regression, SVD, Matrix Factorization(BFGS, sgd, als, cg), LDA, Lasso...

Firstly, paracel splits both massive dataset and massive parameter space. Unlike Mapreduce-Like Systems, paracel offers a simple communication model, allowing you to work with a global and distributed key-value storage, which is called parameter server.

Upon using paracel, you can build algorithms with following rules: 'pull parameters before learning & push local updates after learning'. It is rather a simple model(compared to MPI) which is almost painless transforming from serial to parallel. 

Secondly, paracel tries to solve the 'last-reducer' problem of iterative tasks. We use bounded staleness and find a sweet spot between 'improve-iter' curve and 'iter-sec' curve. A global scheduler takes charge of asynchronous working. This method is already proved to be a generalization of Bsp/Pregel by CMU.

Another advantage of paracel is fault tolerance while MPI has no idea with that.

Paracel can also be used for scientific computing and building graph algorithms. You can load your input in distributed file system and construct a graph, sparse/dense matrix.

Paracel is originally motivated by Jeff Dean's [talk](http://infolab.stanford.edu/infoseminar/archive/WinterY2013/dean.pdf) @Stanford in 2013. You can get more details in his paper: "[Large Scale Distributed Deep Networks](http://static.googleusercontent.com/media/research.google.com/en//archive/large_deep_networks_nips2012.pdf)".


More documents could be found below:

Project Homepage: [paracel.io](http://paracel.io)

20-Minutes' Tutorial: [paracel.io/docs/quick_tutorial.html](http://paracel.io/docs/quick_tutorial.html)

API Reference Page: [paracel.io/docs/api_reference.html](http://paracel.io/docs/api_reference.html)
