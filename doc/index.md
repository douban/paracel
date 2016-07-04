---
title: Paracel - API Reference

language_tabs:
  - cpp 
  - python
  - shell
  - json

toc_footers:
  - <a href='http://paracel.io'>Project Homepage</a>
  - © 2014-2015, Powered by <a href='http://www.douban.com'>豆瓣</a>

includes:
  - errors

search: false 
---

# Overview 

```cpp
std::cout 
  << "(0.5,1)->(0,0.5)->(1,0.5)->(0.5,1)->(0.5,0.25)->(0.25,0.25)";
```

```python
print '(0.5,1)->(0,0.5)->(1,0.5)->(0.5,1)->(0.5,0.25)->(0.25,0.25)'
```

```shell
echo "(0.5,1)->(0,0.5)->(1,0.5)->(0.5,1)->(0.5,0.25)->(0.25,0.25)"
```

```json
{ 
  "logo": "(0.5,1)->(0,0.5)->(1,0.5)->(0.5,1)->(0.5,0.25)->(0.25,0.25)" 
}
```

Welcome to the Paracel API Reference page!

You can use the interfaces listed here to build your own distributed algorithms or applications following Paracel&#39;s paradigm. Till now, Paracel only provides C++ interface for the consideration of computational efficiency. You can view code examples in the right area.

# Data Representation

Paracel use `graph` and `matrix` to represent the training data.

We define four types of `graph`:

* [bigraph](#bigraph)
* [bigraph_continuous](#bigraph_continuous)
* [digraph](#digraph)
* [undirected_graph](#undirected_graph)

We import [Eigen3](http://eigen.tuxfamily.org/index.php?title=Main_Page) library for matrix/vector operations. Eigen3 support two types of matrix:

* [Eigen::SparseMatrix](#eigen::sparseMatrix)
* [Eigen::MatrixXd](#eigen::matrixxd)

<aside class="notice">
<p>
1. You can use neither of them and define your own data structure to store the data.<p>
2. All type of graph can be communicated between worker and server while matrix can not.
</aside>

## Bigraph

```cpp
#include "graph.hpp" // paracel::bigraph
#include "paracel_types.hpp"

/*
 * G:
 *  a, A, 3
 *  a, B, 4
 *  a, D, 2
 *  b, C, 5
 *  b, D, 4
 *  b, E, 5
 *  c, D, 3
 *  c, E, 1
 *  c, F, 2
 *  d, C, 3
 */
void foo(paracel::bigraph<std::string> & G) {
  
  auto data = G.get_data();
  
  auto print_lambda = [] (std::string u,
                          std::string v,
                          double wgt) {
    std::cout << u << "|" << v << "|" << wgt << std::endl;
  };  
  G.traverse(print_lambda);
  
  G.add_edge("c", "G", 3.6);
  G.traverse("c", print_lambda);

  auto ubag = G.left_vertex_bag(); // a, b, c, d
  auto uset = G.left_vertex_set(); // a, b, c, d

  std::vector<std::tuple<std::string,
                  	std::string, double> > dump_tpls;
  G.dump2triples(dump_tpls);

  std::unordered_map<std::string,
                   std::unordered_map<std::string, double> > dump_dict;
  G.dump2dict(dump_dict);

  auto v_sz = G.v(); // 4
  auto e_sz = G.e(); // 11
  auto od = G.outdegree("a"); // 3
  auto id = G.indegree("E"); // 2

  auto adj_info = G.adjacent("c");
}
```

In mathematical field of graph theory, a bipartite graph (or bigraph) is a graph whose vertices can be divided into two disjoint sets `U` and `V` (that is, `U` and `V` are each independent sets) such that every edge connects a vertex in `U` to one in `V`.

###template &lt;class T = paracel::default_id_type&gt;<br>class bigraph {<br><br>&nbsp;public:<br>&nbsp;&nbsp;bigraph();
###&nbsp;&nbsp;bigraph(std::unordered_map&lt;T, std::unordered_map&lt;T, double&gt; &gt; edge_info);
###&nbsp;&nbsp;bigraph(std::vector&lt;std::tuple&lt;T, T&gt; &gt; tpls);
###&nbsp;&nbsp;bigraph(std::vector&lt;std::tuple&lt;T, T, double&gt; &gt; tpls);
###&nbsp;&nbsp;void add_edge(const T & v, const T & w); 
###&nbsp;&nbsp;void add_edge(const T & v, const T & w, double wgt); 
###&nbsp;&nbsp;// return bigraph data<br>&nbsp;&nbsp;std::unordered_map&lt;T, std::unordered_map&lt;T, double&gt; &gt; get_data(); 
###&nbsp;&nbsp;// traverse bigraph edge using functor `func`<br>&nbsp;&nbsp;template &lt;class F&gt;<br>&nbsp;&nbsp;void traverse(F & func);
###&nbsp;&nbsp;// traverse vertex `v`'s related edges using functor `func`<br>&nbsp;&nbsp;template &lt;class F&gt; <br>&nbsp;&nbsp;void traverse(const T & v, F & func);
###&nbsp;&nbsp;// return `U` bag<br>&nbsp;&nbsp;std::vector&lt;T&gt; left_vertex_bag();
###&nbsp;&nbsp;// return `U` set<br>&nbsp;&nbsp;std::unordered_set&lt;T&gt; left_vertex_set();
###&nbsp;&nbsp;// out: `tpls`<br>&nbsp;&nbsp;void dump2triples(std::vector&lt;std::tuple&lt;T, T, double&gt; &gt; & tpls);
###&nbsp;&nbsp;// out: `dict`<br>&nbsp;&nbsp;void dump2dict(std::unordered_map<T, std::unordered_map<T, double> > & dict);
###&nbsp;&nbsp;// return number of vertexes in `U`<br>&nbsp;&nbsp;inline size_t v();
###&nbsp;&nbsp;// return number of edges in bigraph<br>&nbsp;&nbsp;inline size_t e();
###&nbsp;&nbsp;// return adjacent info of vertex `v`<br>&nbsp;&nbsp;std::unordered_map&lt;T, double> adjacent(const T & v);
###&nbsp;&nbsp;// return outdegree of vertex `u` in `U`<br>&nbsp;&nbsp;inline size_t outdegree(const T & u);
###&nbsp;&nbsp;// return indegree of vertex `v` in `V`<br>&nbsp;&nbsp;inline size_t indegree(const T & v);
###};

## Bigraph_continuous

```cpp
#include "graph.hpp" // paracel::bigraph_continuous
#include "paracel_types.hpp"

/*
 * G:
 *  0, 1, 3
 *  0, 2, 4
 *  0, 4, 2
 *  1, 3, 5
 *  1, 4, 4
 *  1, 5, 5
 *  2, 4, 3
 *  2, 5, 1
 *  2, 6, 2
 *  3, 3, 3
 */
void foo() {
  
  paracel::bigraph_continuous G;
  G.add_edge(0, 1, 3.);
  G.add_edge(0, 2, 4.);
  G.add_edge(0, 4, 2.);
  G.add_edge(1, 3, 5.);
  G.add_edge(1, 4, 4.);
  G.add_edge(1, 5, 5.);
  G.add_edge(2, 4, 3.);
  G.add_edge(2, 5, 1.);
  G.add_edge(2, 6, 2.);
  G.add_edge(3, 3, 3.);
  
  auto print_lambda = [] (paracel::default_id_type u,
                          paracel::default_id_type v,
                          double wgt) {
    std::cout << u << "|" << v << "|" << wgt << std::endl;
  };  
  G.traverse(print_lambda);
  G.traverse(0, print_lambda);

  auto v_sz = G.v(); // 4
  auto e_sz = G.e(); // 10
  auto od = G.outdegree(0); // 3
  auto id = G.indegree(5); // 2

  auto adj_info = G.adjacent(2);
}
```

`paracel::bigraph_continous` can also be used to represent a bipartite graph. Vertexes of a `paracel::bigraph_continuous` must be indexed from 0 to N-1(N is the number of total number of vertexes in `U`). It will cost less memory comparing to `paracel::bigraph<paracel::default_id_type>` and will be more efficent since it is continous, the interface below is similar.

###class bigraph_continuous {<br><br>&nbsp;public:<br>&nbsp;&nbsp;bigraph_continuous();
###&nbsp;&nbsp;bigraph_continuous(paracel::default_id_type n);
###&nbsp;&nbsp;// sequential interface, first row of file `filename` is size of `U`<br>&nbsp;&nbsp;bigraph_continuous(const std::string & filename);
###&nbsp;&nbsp;void add_edge(paracel::default_id_type src,<br>&ensp;&ensp;&ensp;&ensp;paracel::default_id_type dst);
###&nbsp;&nbsp;void add_edge(paracel::default_id_type src,<br>&ensp;&ensp;&ensp;&ensp;paracel::default_id_type dst,<br>&ensp;&ensp;&ensp;&ensp;double rating);
###&nbsp;&nbsp;// return number of vertexes in `U`<br>&nbsp;&nbsp;paracel::default_id_type v();
###&nbsp;&nbsp;// return number of edges in bigraph_continuous<br>&nbsp;&nbsp;paracel::default_id_type e();
###&nbsp;&nbsp;// return adjacent info of vertex `v`<br>&nbsp;&nbsp;paracel::bag_type&lt;std::pair&lt;paracel::default_id_type, double&gt; &gt; &nbsp;&nbsp;adjacent(paracel::default_id_type v);
###&nbsp;&nbsp;// return outdegree of vertex `u` in `U`<br>&nbsp;&nbsp;inline size_t outdegree(paracel::default_id_type u);
###&nbsp;&nbsp;// return indegree of vertex `v` in `V`<br>&nbsp;&nbsp;inline size_t indegree(paracel::default_id_type v);
###&nbsp;&nbsp;// traverse bigraph edge using functor `func`<br>&nbsp;&nbsp;template &lt;class F&gt;<br>&nbsp;&nbsp;void traverse(F & func);
###&nbsp;&nbsp;// traverse vertex `v`’s related edges using functor `func`<br>&nbsp;&nbsp;template &lt;class F&gt;<br>&nbsp;&nbsp;void traverse(paracel::default_id_type v, F & func);
###};

## Digraph

```cpp
#include "graph.hpp" // paracel::digraph
#include "paracel_types.hpp"

/*
 * G:
 *   0, 0, 3.0
 *   0, 2, 5.0
 *   1, 0, 4.0
 *   1, 1, 3.0
 *   1, 2, 1.0
 *   2, 0, 2.0
 *   2, 3, 1.0
 *   3, 1, 3.0
 *   3, 3, 1.0
 */
void foo(paracel::digraph<size_t> & G) {

  G.add_edge(3, 4, 5.);

  auto data = G.get_data();
  auto vbag = G.vertex_bag(); // std::vector<size_t>({0, 1, 2, 3, 4})
  auto vset = G.vertex_set(); // unordered_set<size_t>({0, 1, 2, 3, 4})
  auto adj = G.adjacent(0); // {0 : 3.0; 2 : 5.0}

  auto print_lambda = [] (size_t a, size_t b, double c) {
    std::cout << a << " | " << b << " | " << c << std::endl;
  };  
  grp.traverse(print_lambda);
  grp.traverse(1, print_lambda); // 1|3|4.0\n1|1|3.0\n1|2|1.0, order does not matter
  
  std::vector<std::tuple<size_t, size_t, double> > dump_tpls;
  G.dump2triples(dump_tpls);
  std::unordered_map<size_t,
                   std::unordered_map<size_t, double> > dump_dict;
  G.dump2dict(dump_dict);
  
  auto v_sz = G.v(); // 5
  auto e_sz = G.e(); // 10
  auto od = G.outdegree(0); // 2
  auto id = G.indegree(0); // 2
  auto ad = G.avg_degree(); // 2
  auto sl = G.selfloops(); // 3

  grp.reverse();
  v_sz = G.v(); // 5
  e_sz = G.e(); // 10
  od = G.outdegree(0); // 3
  id = G.indegree(0); // 2
  ad = G.avg_degree(); // 2
  sl = G.selfloops(); // 3

}
```

`paracel::digraph` can be used to represent a weighted directed graph. You can use it to develop graph algorithms such as [pagerank](http://en.wikipedia.org/wiki/PageRank). Iterfaces of `digraph` are quite similar with [paracel::undirected_graph](#undirected_graph).

###template &lt;class T = paracel::default_id_type&gt;<br>class digraph {<br><br>&nbsp;public:<br>&nbsp;&nbsp;digraph();
###&nbsp;&nbsp;digraph(std::unordered_map&lt;T, std::unordered_map&lt;T, double&gt; &gt; edge_info);
###&nbsp;&nbsp;digraph(std::vector&lt;std::tuple&lt;T, T&gt; &gt; tpls);
###&nbsp;&nbsp;digraph(std::vector&lt;std::tuple&lt;T, T, double&gt; &gt; tpls);
###&nbsp;&nbsp;void add_edge(const T & v, const T & w);
###&nbsp;&nbsp;void add_edge(const T & v, const T & w, double wgt);
###&nbsp;&nbsp;// return digraph data<br>&nbsp;&nbsp;std::unordered_map<T, std::unordered_map<T, double> > get_data();
###&nbsp;&nbsp;// traverse bigraph edge using functor `func`<br>&nbsp;&nbsp;template &lt;class F&gt;<br>&nbsp;&nbsp;void traverse(F & func);
###&nbsp;&nbsp;// traverse vertex `v`’s related edges using functor `func`<br>&nbsp;&nbsp;template &lt;class F&gt;<br>&nbsp;&nbsp;void traverse(const T & v, F & func);
###&nbsp;&nbsp;template &lt;class F&gt;<br>&nbsp;&nbsp;void traverse_by_vertex(F & func);
###&nbsp;&nbsp;// return vertexes of digraph with std::vector<br>&nbsp;&nbsp;std::vector&lt;T&gt; vertex_bag();
###&nbsp;&nbsp;// return vertexes of digraph with std::unordered_set<br>&nbsp;&nbsp;std::unordered_set&lt;T&gt; vertex_set();
###&nbsp;&nbsp;// out: `tpls`<br>&nbsp;&nbsp;void dump2triples(std::vector&lt;std::tuple&lt;T, T, double&gt; &gt; & tpls);
###&nbsp;&nbsp;// out: `dict`<br>&nbsp;&nbsp;void dump2dict(std::unordered_map&lt;T, std::unordered_map&lt;T, double&gt; &gt; & dict);
###&nbsp;&nbsp;// reverse digraph with edge direction<br>&nbsp;&nbsp;digraph reverse();
###&nbsp;&nbsp;// return total number of vertexes<br>&nbsp;&nbsp;inline size_t v();
###&nbsp;&nbsp;// return total number of edges<br>&nbsp;&nbsp;inline size_t e();
###&nbsp;&nbsp;// return adjacent info of vertex `v`<br>&nbsp;&nbsp;std::unordered_map&lt;T, double&gt;<br>&nbsp;&nbsp;adjacent(const T & v);
###&nbsp;&nbsp;unordered_map&lt;T, double&gt;<br>&nbsp;&nbsp;reverse_adjacent(const T & v);
###&nbsp;&nbsp;// return outdegree of vertex `v`<br>&nbsp;&nbsp;inline size_t outdegree(const T & v);
###&nbsp;&nbsp;// return indegree of vertex `v`<br>&nbsp;&nbsp;inline size_t indegree(const T & v);
###&nbsp;&nbsp;// return average degree of digraph<br>&nbsp;&nbsp;inline double avg_degree();
###&nbsp;&nbsp;// return total number of self-loops of digraph<br>&nbsp;&nbsp;inline int selfloops();

## Undirected_graph

```cpp
#include "graph.hpp" // paracel::undirected_graph
#include "paracel_types.hpp"

/*
 * G:
 *   0, 1
 *   0, 2
 *   0, 5
 *   0, 6
 *   3, 4
 *   3, 5
 *   4, 5
 *   4, 6
 *   9, 10
 *   9, 11
 *   9, 12
 *   11, 12
 */
void foo(paracel::undirected_graph<size_t> & G) {
  auto v_sz = G.v(); // 11
  auto e_sz = G.e(); // 12
  auto adj = G.adjacent(5); // {0: 1.0, 4:1.0}
  auto ad = G.avg_degree(); // 24. / 11.
  auto md = G.max_degree(); // 4
  auto sl = G.selfloops(); // 0
  auto print_lambda = [] (size_t a, size_t b, double c) {
    std::cout << a << " | " << b << " | " << c << std::endl;
  };  
  G.traverse(print_lambda);
  G.traverse(0, print_lambda); // 0|1|1.0\n0|2|1.0\n0|5|1.0\n0|6|1.0, order does not matter
}
```

`paracel::undirected_graph` can be used to represent a weighted undirected graph. You can use it to develop graph algorithms. Interfaces of `undirected_graph` are quite similar with [paracel::digraph](#digraph).

###template &lt;class T = size_t&gt;<br>class undirected_graph {<br><br>&nbsp;public:<br>&nbsp;&nbsp;undirected_graph();
###&nbsp;&nbsp;undirected_graph(std::unordered_map&lt;T, std::unordered_map&lt;T, double&gt; &gt; edge_info);
###&nbsp;&nbsp;undirected_graph(std::vector&lt;std::tuple&lt;T, T&gt; &gt; tpls);
###&nbsp;&nbsp;undirected_graph(std::vector&lt;std::tuple&lt;T, T, double&gt; &gt; tpls);
###&nbsp;&nbsp;void add_edge(const T & v, const T & w);
###&nbsp;&nbsp;void add_edge(const T & v, const T & w, double wgt);
###&nbsp;&nbsp;// return undirected_graph data<br>&nbsp;&nbsp;unordered_map<T, std::unordered_map<T, double> > get_data();
###&nbsp;&nbsp;// traverse undirected_graph edge using functor `func`<br>&nbsp;&nbsp;template &lt;class F&gt;<br>&nbsp;&nbsp;void traverse(F & func);
###&nbsp;&nbsp;// traverse vertex `v`’s related edges using functor `func`<br>&nbsp;&nbsp;template &lt;class F&gt;<br>&nbsp;&nbsp;oid traverse(const T & v, F & func);
###&nbsp;&nbsp;template &lt;class F&gt;<br>&nbsp;&nbsp;void traverse_by_vertex(F & func);
###&nbsp;&nbsp;// return vertexes of undirected_graph with std::vector<br>&nbsp;&nbsp;std::vector<T> vertex_bag();
###&nbsp;&nbsp;// return vertexes of undirected_graph with std::unordered_set<br>&nbsp;&nbsp;std::unordered_set<T> vertex_set();
###&nbsp;&nbsp;// return total number of vertexes<br>&nbsp;&nbsp;inline size_t v();
###&nbsp;&nbsp;// return total number of edges<br>&nbsp;&nbsp;inline size_t e();
###&nbsp;&nbsp;// return adjacent info of vertex `v`<br>&nbsp;&nbsp;std::unordered_map&lt;T, double&gt;<br>&nbsp;&nbsp;adjacent(const T & v);
###&nbsp;&nbsp;// return outdegree of vertex `v`<br>&nbsp;&nbsp;inline size_t outdegree(const T & v);
###&nbsp;&nbsp;// return indegree of vertex `v`<br>&nbsp;&nbsp;inline size_t indegree(const T & v);
###&nbsp;&nbsp;// return average degree of digraph<br>&nbsp;&nbsp;inline double avg_degree();
###&nbsp;&nbsp;// return total number of self-loops of digraph<br>&nbsp;&nbsp;inline int selfloops();

## More Graph Operation

## SparseMatrix

```cpp
#include <vector>
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Sparse>
#include <eigen3/Eigen/QR>
#include <eigen3/Eigen/Cholesky>
#include "utils.hpp"

int main(int argc, char *argv[])
{
  Eigen::MatrixXd mtx(4, 3);
  std::cout << mtx << std::endl;
  mtx << 1., 2., 3.,
        4., 5., 6.,
        7., 8., 9.,
        10., 11., 12.;
  Eigen::MatrixXd clusters_mtx(2, 3);
  std::vector<double> c1 = {2.5, 3.5, 4.5}, c2 = {8.5, 9.5, 10.5};
  clusters_mtx.row(0) = paracel::vec2evec(c1);
  clusters_mtx.row(1) = paracel::vec2evec(c2);

  for(size_t i = 0; i < (size_t)mtx.rows(); ++i) {
    Eigen::MatrixXd::Index indx;
    (clusters_mtx.rowwise() - mtx.row(i))
      .rowwise()
      .squaredNorm()
      .minCoeff(&indx);
    std::cout << "| " << indx << " |" << std::endl;
  }
  
  typedef Eigen::Triplet<double> eigen_triple;
  std::vector<eigen_triple> tpls;
  tpls.push_back(eigen_triple(0, 0, 0.6));
  tpls.push_back(eigen_triple(0, 2, 0.7));
  tpls.push_back(eigen_triple(0, 4, 0.4));
  tpls.push_back(eigen_triple(1, 2, 0.6));
  tpls.push_back(eigen_triple(1, 3, 0.5));
  tpls.push_back(eigen_triple(1, 4, 0.3));
  tpls.push_back(eigen_triple(2, 0, 0.3));
  tpls.push_back(eigen_triple(2, 1, 0.1));
  tpls.push_back(eigen_triple(3, 3, 0.1));
  tpls.push_back(eigen_triple(3, 4, 0.7));
  tpls.push_back(eigen_triple(4, 1, 0.3));
  tpls.push_back(eigen_triple(5, 0, 0.1));
  tpls.push_back(eigen_triple(5, 4, 0.7));
  tpls.push_back(eigen_triple(6, 0, 0.2));
  tpls.push_back(eigen_triple(6, 2, 0.8));
  tpls.push_back(eigen_triple(7, 0, 0.3));
  tpls.push_back(eigen_triple(8, 1, 0.1));
  tpls.push_back(eigen_triple(8, 2, 0.2));
  tpls.push_back(eigen_triple(8, 3, 0.3));
  tpls.push_back(eigen_triple(8, 4, 0.4));
  tpls.push_back(eigen_triple(9, 0, 0.9));
  tpls.push_back(eigen_triple(9, 3, 0.1));
  tpls.push_back(eigen_triple(9, 4, 0.2));
  Eigen::SparseMatrix<double, Eigen::RowMajor> A;
  A.resize(10, 5);
  A.setFromTriplets(tpls.begin(), tpls.end());
  Eigen::MatrixXd H(5, 3); // 5 * 3
  H << 1., 2., 3.,
      4., 5., 6.,
      7., 8., 9.,
      10., 11., 12.,
      13., 14., 15.;
  Eigen::MatrixXd W(1,1);
  W.resize(10, 3);
  W = A * H;
  std::cout << W << std::endl;
  
  return 0;
}
```

Paracel import `Eigen::SparseMatrix` as sparse matrix representation. Check out more details [here](http://eigen.tuxfamily.org/index.php?title=SparseMatrix). You can directly use it to develop your sparse matrix related application. In the code area, we show a brief usage of `Eigen::SparseMatrix`. `Eigen::SparseVector` is similar usage.

###// std::vector to Eigen::VectorXd<br>Eigen::VectorXd vec2evec(const std::vector<double> & v);
###// Eigen::VectorXd to std::vector<br>std::vector<double> evec2vec(const Eigen::VectorXd & ev);
###// traverse Eigen::SparseVector `v` with functor `func`<br>template &lt;class F&gt;<br>void traverse_vector(Eigen::SparseVector&lt;double&gt; & v, F & func);
###// traverse Eigen::SparseMatrix `m` with functor `func`<br>template &lt;class F&gt;<br>void traverse_matrix(Eigen::SparseMatrix&lt;double, Eigen::RowMajor&gt; & m, F & func);

## Dense Matrix

```cpp
#include <eigen3/Eigen/Dense>
#include <iostream>
#include "utils.hpp"

void foo() {
  std::cout.precision(20);
  Eigen::MatrixXd H_blk(4, 3);
  Eigen::MatrixXd stone = Eigen::MatrixXd::Random(2, 3);
  H_blk.row(0) = stone.row(0);
  H_blk.row(1) = stone.row(1);
  H_blk.row(2) = stone.row(0);
  H_blk.row(3) = stone.row(1);
  std::cout << stone.transpose() * stone << std::endl;
  auto kk = H_blk.block(0, 0, 2, 3).transpose() * H_blk.block(0,0,2,3);
  auto kk2 = H_blk.block(2, 0, 2, 3).transpose() * H_blk.block(2,0,2,3);
  Eigen::MatrixXd tt(3, 3);
  tt << 
    kk.row(0)[0] + kk2.row(0)[0],
    kk.row(0)[1] + kk2.row(0)[1],
    kk.row(0)[2] + kk2.row(0)[2],
    kk.row(1)[0] + kk2.row(1)[0],
    kk.row(1)[1] + kk2.row(1)[1],
    kk.row(1)[2] + kk2.row(1)[2],
    kk.row(2)[0] + kk2.row(2)[0],
    kk.row(2)[1] + kk2.row(2)[1],
    kk.row(2)[2] + kk2.row(2)[2];
  
  std::cout << tt << std::endl;
  std::cout << tt.inverse() << std::endl;
  std::cout << (H_blk.transpose() * H_blk).inverse() << std::endl;
}

void goo() {
  
  Eigen::MatrixXd m(2, 3);
  m << 1., 2., 3.,
      4., 5., 6.;
  
  auto v = paracel::mat2vec(m);
  for(auto & vv : v) std::cout << vv << std::endl; // 1\n4\n2\n5\n3\n6
  
  Eigen::MatrixXd mat = paracel::vec2mat(v, 3); // 3 is columns
  // 1 2 3
  // 4 5 6
  std::cout << mat << std::endl;
}

int main(int argc, char *argv[])
{
  foo();
  goo();
  return 0; 
}
```

Paracel import `Eigen::MatrixXd` as dense matrix represestation. Check out mpre details [here](http://eigen.tuxfamily.org/dox/group__DenseMatrixManipulation__chapter.html). You can directly use it to develop your matrix related application. In the right code area, we show a brief usage of `Eigen::MatrixXd`.

###// Eigen::MatrixXd is column-major and return col seq<br>std::vector<double> mat2vec(const Eigen::MatrixXd & m);
###// return row seq<br>Eigen::MatrixXd vec2mat(const std::vector<double> & v, size_t rows);

<aside class="notice">
Since Paracel do not support Eigen::MatrixXd for communication between worker and parameter server, we provide mat2vec and vec2mat interface above. You can also do it yourself converting Eigen::SparseMatrix and Eigen::MatrixXd to stl containers.
</aside>

# Paralg

`Paralg` is the basic class providing a marjority of functionalities of Paracel. Writing a Paracel program involves subclassing the `paralg` baseclass and you have to override the virtual `solve` method. Some of them are [SPMD](http://en.wikipedia.org/wiki/SPMD) iterfaces, we will call them parallel interfaces in the following.

## Initialize

```cpp
#include <gflags/gflags.h>
#include "ps.hpp"
#include "utils.hpp"

class foo : public paracel::paralg {
 public:
  foo(paracel::Comm comm,
      std::string hosts_dct_str,
      std::string _output,
      int k) : paracel::paralg(hosts_dct_str, comm, _output), topk(k) {}
  virtual void solve() { /* ... */ }
 private:
  int topk;
};

DEFINE_string(server_info,
              "host1:7777PARACELhost2:8888",
              "hosts name string of paracel-servers.\n");

int main(int argc, char *argv[])
{
  paracel::main_env comm_main_env(argc, argv);
  paracel::Comm comm(MPI_COMM_WORLD);
  google::SetUsageMessage("[options]\n\t--server_info\n\t--cfg_file\n");
  google::ParseCommandLineFlags(&argc, &argv, true);
  std::string output = "/nfs/tmp/demo/";
  {
    paracel::paralg *pt = new paracel::paralg(comm);
    delete pt;
  }
  {
    foo solver(comm, FLAGS_server_info, output, 10);
  }
  return 0;
}
```

###paralg(paracel::Comm comm, std::string output = "", int rounds = 1);

*Constructor: for direct use. `comm` refer to the worker communicator, `output` refer to the output folder for dumping results, `rounds` refer to the traverse count of the total dataset.*

###paralg(std::string hosts_dct_str,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;paracel::Comm comm,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;std::string output = "",<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;int rounds = 1,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;int limit_s = 0,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;bool ssp_switch = false);

*Constructor: for subclassing `paralg` use. `hosts_dct_str` is internal connect information which you will get from the main function, `comm` refer to the worker communicator, `output` refer to the output folder for dumping results, `rounds` refer to the traverse count of the total dataset, `limit_s` and `ssp_switch` are used for [Asynchronous Controlling](#asynchronous-controlling) in iterative tasks. `output`, `rounds`, `limit_s` and `ssp_switch` are default parameters, you do not need to define them.*

<aside class="success">
Remember
<p>
<p> - With passing into `output` argument, Paracel will create a new folder if not exist.
<p> - You do not need to know what the variable `hosts_dct_str` really means, you just need to get it from the main function and use it to initialize the constructor.
</aside>

## Load

```cpp
#include <gflags/gflags.h>
#include "ps.hpp"
#include "utils.hpp"
#include "graph.hpp"

class foo : public paracel::paralg {
 public:
  foo(paracel::Comm comm,
      std::string hosts_dct_str) : paracel::paralg(hosts_dct_str, comm) {}

  void load_demo1() {
    size_t total_sz = 0;
    auto line_list = paracel_loadall("./data/*.csv");
    for(auto & line : line_lst) {
      total_sz += line.size();
    }
  }

  void load_demo2() {
    size_t total_sz = 0;
    auto handle_lambda = [&] (std::vector<std::string> & lines) {
      for(auto & line : lines) {
        total_sz += line.size();
      }
    };
    paracel_sequential_loadall("./data/*.csv", handle_lambda);
  }

  void load_demo3() {
    std::vector<std::string> files = {data1.csv, data2.csv};
    size_t local_sz = 0;
    auto local_line_list = paracel_load(files);
    for(auto & line : local_line_list) {
      local_sz += line.size();
    }
  }

  void load_demo4() {
    std::vector<std::string> files = {data1.csv, data2.csv};
    size_t local_sz = 0;
    paracel_load_handle(files, [&] (string & line) { local_sz += line.size(); });
  }
  
  void load_demo5() {
    paracel::digraph<std::string> G1;
    paracel::bigraph<paracel::default_id_type> G2;
    paracel::bigraph_continuous G3;
    
    /*
     * graph1.txt     graph2.txt
     * 100,1,1          a,b
     * 100,3,2          a,c
     * 101,4,3          b,d
     * 102,1,2          c,a
     * ...            ...
     * ...            ...
     */
    auto local_parser = [] (const std::string & line) {
      return paracel::str_split(line, ',');
    };
    // you have to wrap your pre-defined functor with paracel::gen_parser
    auto f_parser = paracel::gen_parser(local_parser);

    paracel_load_as_graph(G1, "graph2.txt", f_parser, "fmap", false);
    paracel_load_as_graph(G2, "graph1.txt", f_parser, "smap", false);

    std::unordered_map<paracel::default_id_type,
    			paracel::default_id_type> row_map, col_map;
    paracel_load_as_graph(G3, row_map, col_map, "graph1.txt", f_parser);
  }

  void load_demo6() {
    Eigen::SparseMatrix<double, Eigen::RowMajor> blk_mtx1, blk_mtx2, blk_mtx3;
    std::unordered_map<paracel::default_id_type,
    			std::string> row_map, col_map;
    /*
     * matrix1.txt  	matrix2.txt
     * a,b,1		a 1.1,2.1,3.1,4.1
     * a,c,2		b 1.2,2.2,3.2,4.2
     * b,d,3		c 1.3,2.3,3.3,4.3
     * c,a,2		d 1.4,2.4,3.4,4.4
     * ...
     * ...
     */
    auto local_parser = [] (const std::string & line) {
      return paracel::str_split(line, ',');
    };
    // you have to wrap your pre-defined functor with paracel::gen_parser
    auto f_parser = paracel::gen_parser(local_parser);

    paracel_load_as_matrix(blk_mtx1, row_map, col_map, 
    						"matrix1.txt", f_parser, "fmap", false);
    paracel_load_as_matrix(blk_mtx2, row_map, "matrix1.txt", f_parser, "fmap");
    paracel_load_as_matrix(blk_mtx3, "matrix1.txt", f_parser, "fmap");

    Eigen::MatrixXd blk_dense_mtx1, blk_dense_mtx2;
    paracel_load_as_matrix(blk_dense_mtx1, row_map,
    						"matrix2.txt", f_parser);
    paracel_load_as_matrix(blk_dense_mtx2, "matrix2.txt", f_parser);
  }

  virtual void solve() {
    // paracel_loadall interface usage
    load_demo1();
    // paracel_sequential_loadall interface usage
    load_demo2();
    // paracel_load interface usage
    load_demo3();
    // paracel_load_handle interface usage
    load_demo4();
    // paracel_load_as_graph interface usage
    load_demo5();
    // paracel_load_as_matrix interface usage
    load_demo6();
  }

};

DEFINE_string(server_info,
              "host1:7777PARACELhost2:8888",
              "hosts name string of paracel-servers.\n");

int main(int argc, char *argv[])
{
  paracel::main_env comm_main_env(argc, argv);
  paracel::Comm comm(MPI_COMM_WORLD);
  google::SetUsageMessage("[options]\n\t--server_info\n\t--cfg_file\n");
  google::ParseCommandLineFlags(&argc, &argv, true);
  foo solver(comm, FLAGS_server_info);
  solver.solve();
  return 0;
}
```

Paracel provides various interfaces for loading input files. In this version, all load related interfaces only support text format files so that it will cost a little more memory. You can either read a partition lines of data parallelly then construct customized data structure or directly load input data as the Paracel&#39;s `graph` or `matrix` types. In the latter case, you have to use `pattern` and `mix_flag` variables to describe the structure of the input files. `Pattern` also decide the partition method for input data.

Paracel defines several patterns with variable `pattern`:

pattern | structure | line example
--- | --- | -------------
linesplit(default) | partition with lines | all structures
fmap | first-second case(value set to 1.0)<br>first-second-value case<br>partition with the first field | a,b<br>a,b,0.2
smap | second-first case(value set to 1.0)<br>second-first-value case<br>partition with the second field | a,b<br>a,b,0.2
fsmap | support the same structure as fmap and smap<br>2D partition | a,b or a,b,0.2
fvec | id,feature1,...,featurek case<br>partition with id | 1001 0.1&#124;0.2&#124;0.3&#124;0.4
fset | attr1,attr2,attr3,...<br>attr1,attr2&#124;value2,attr3&#124;value3,...<br>partition with the first field | a,b,c or a,b&#124;0.2,c&#124;0.4

Variable `mix_flag` represents whether linking relation of a graph/matrix is defined in a single line. As you can see the example below, when `mix_flag` is set to false, all the linking relation of node 'a' is expanded in three single lines. If `pattern` equals to `fvec` and `fset`, `mix_flag` is always `true`.

mix_flag | example
--- | ---
true | a,b,c,d<br>b,c,d<br>...
true | a,b<br>a,c,d<br>b,c<br>b,d<br>...
false(default) | a,b<br>a,c<br>a,d<br>b,c<br>b,d<br>...

As you can see above, `pattern` do not only decide data format but also decide partition strategy while `mix_flag` tell Paracel if a link relation is mixed in a single line.

###template&lt;class T&gt;<br>vector&lt;string&gt; paracel_loadall(const T & fn);
*Each worker load all lines in `fn`. `T` can be either string type to represent one file or vector&lt;string&gt; to represent a set of files. See more details on the right.*

###template&lt;class T, class F&gt;<br>void paracel_sequential_loadall(const T & fn, F & func);
*Each worker load all lines in `fn` and handle with functor `func`. To avoid memory exceed, we recommend you to use this interface. Paracel will load part of lines in `fn` and call `func` to handle them, then do it again and again until the end of `fn`. `T` can be either string type to represent one file or vector&lt;string&gt; to represent a set of files. See more details on the right.*

###template&lt;class T&gt;<br>vector<string> paracel_load(const T & fn);
*SPMD interface: each worker load `fn`&#39;s lines parallelly. `T` can be either string type to represent one file or vector&lt;string&gt; to represent a set of files. See more details on the right.*

###template&lt;class T, class F&gt;<br>void paracel_load_handle(const T & fn,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;F & func,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;const string & pattern = "linesplit",<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;bool mix_flag = false);
*SPMD interface: parallelly load `fn` and handle with `func` line by line. `func` takes string type as argument and Paracel will pass each line of `fn` as input parameter. You can also specify `pattern` and `mix_flag` to describe the structure of `fn`. `T` can be either string type to represent one file or vector&lt;string&gt; to represent a set of files. See more details on the right.*

###template&lt;class T, class G&gt;<br>void paracel_load_as_graph(paracel::digraph&lt;G&gt; & grp,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;const T & fn,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;parser_type & parser,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;const std::string pattern = "fmap",<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;bool mix_flag = false);
*SPMD interface: parallelly load `fn` using `parser` and then generate local [digraph](#digraph) `grp` for each worker. Input argument of `parser` must be string type and it must return vector&lt;string&gt; type which refer to the fields of this digraph. You can also specify `pattern` and `mix_flag` to describe the structure of `fn`. `T` can be either string type to represent one file or vector&lt;string&gt; to represent a set of files. See more details on the right.*

###template&lt;class T, class G&gt;<br>void paracel_load_as_graph(paracel::bigraph&lt;G&gt; & grp,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;const T & fn,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;parser_type & parser,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;const std::string pattern = "fmap",<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;bool mix_flag = false);
*SPMD interface: parallelly load `fn` using `parser` and then generate local [bigraph](#bigraph) `grp` for each worker. Input argument of `parser` must be string type and it must return vector&lt;string&gt; type which refer to the fields of this bigraph. You can also specify `pattern` and `mix_flag` to describe the structure of `fn`. `T` can be either string type to represent one file or vector&lt;string&gt; to represent a set of files. See more details on the right.*

###template&lt;class T&gt;<br>void paracel_load_as_graph(paracel::bigraph_continuous & grp,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;unordered_map&lt;default_id_type, default_id_type&gt; & row_map,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;unordered_map&lt;default_id_type, default_id_type&gt; & col_map,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;const T & fn,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;parser_type & parser,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;const string & pattern = "fmap",<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;bool mix_flag = false);
*SPMD interface: parallelly load `fn` using `parser` and then generate local [bigraph_continuous](#bigraph_continuous) `grp` for each worker. Since Paracel will map original ids into continuous indexes, this interface has two more output arguments to tell the mapping infomation: `row_map` and `col_map`. `row_map` or `col_map` store the corresponding relation between `paracel::default_id_type` and generic type `G` of graph node. Input argument of `parser` must be string type and it must return vector&lt;string&gt; type which refer to the fields of this bigraph_continuous. You can also specify `pattern` and `mix_flag` to describe the structure of `fn`. `T` can be either string type to represent one file or vector&lt;string&gt; to represent a set of files. See more details on the right.*

###template&lt;class T, class G&gt;<br>void paracel_load_as_matrix(Eigen::SparseMatrix&lt;double, Eigen::RowMajor&gt; & blk_mtx,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;std::unordered_map&lt;default_id_type, G&gt; & row_map,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;std::unordered_map&lt;default_id_type, G&gt; & col_map,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;const T & fn,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;parser_type & parser,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;const std::string & pattern = "fsmap",<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;bool mix_flag = false);
*SPMD interface: parallelly load `fn` using `parser` and then generate block [Eigen::SparseMatrix](#sparsematrix) `blk_mtx` for each worker. Since indexes of a matrix must be continuous, Paracel will map original ids into continuous indexes. There are two more output arguments to store the mapping infomation: `row_map` and `col_map`. `row_map` or `col_map` store the corresponding relation between `paracel::default_id_type` and generic type `G` of Eigen::SparseMatrix. Input argument of `parser` must be string type and it must return vector&lt;string&gt; type. You can also specify `pattern` and `mix_flag` to describe the structure of `fn`. `T` can be either string type to represent one file or vector&lt;string&gt; to represent a set of files. See more details on the right.*

###template&lt;class T, class G&gt;<br>void paracel_load_as_matrix(Eigen::SparseMatrix&lt;double, Eigen::RowMajor&gt; & blk_mtx,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;std::unordered_map&lt;default_id_type, G&gt; & row_map,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;const T & fn,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;parser_type & parser,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;const std::string & pattern = "fsmap",<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;bool mix_flag = false);
*Similar interface as the previous one without `col_map`. See more details on the right.*

###template&lt;class T, class G&gt;<br>void paracel_load_as_matrix(Eigen::SparseMatrix&lt;double, Eigen::RowMajor&gt; & blk_mtx,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;const T & fn,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;parser_type & parser,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;const std::string & pattern = "fsmap",<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;bool mix_flag = false);
*Similar interface as the previous one without `row_map` and `col_map`. See more details on the right.*

###template&lt;class T, class G&gt;<br>void paracel_load_as_matrix(Eigen::MatrixXd & blk_dense_mtx,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;std::unordered_map&lt;default_id_type, G&gt; & row_map,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;const T & fn,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;parser_type & parser);
*SPMD interface: parallelly load `fn` using `parser` and then generate block [Eigen::MatrixXD](#dense-matrix) `blk_dense_mtx` for each worker. Since indexes of a matrix must be continuous, Paracel will map original ids into continuous indexes. The output arguments `row_map` is used to store the corresponding relation between `paracel::default_id_type` and generic type `G` of Eigen::MatrixXd. Input argument of `parser` must be string type and it must return vector&lt;string&gt; type. File structure in this case can be only `fvec`. `T` can be either string type to represent one file or vector&lt;string&gt; to represent a set of files. See more details on the right.*

###template&lt;class T, class G&gt;<br>void paracel_load_as_matrix(Eigen::MatrixXd & blk_dense_mtx,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;const T & fn,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;parser_type & parser);
*Similar interface as the previous one without `row_map`. See more details on the right.*

<aside class="success">
Remember
<p>
<p> - <b>fn</b> above can be a regular expression.
</aside>

## Communication

```cpp

#include <string>
#include <unordered_map>
#include <gflags/gflags.h>
#include "ps.hpp"
#include "utils.hpp"

using std::string;
using std::unordered_map;
using std::vector;

class foo : public paracel::pralg {
 public:
  foo(paracel::Comm comm, string hosts_dct_str) 
  	: paracel::paralg(hosts_dct_str, comm) {}
  
  void test_register() {
    paracel_register_update(
    			"/nfs/lib/paracel/libupdate.so",
    			"sum_updater");
    paracel_register_bupdate(
    			"/nfs/lib/paracel/libupdate.so",
    			"sum_updater");
    paracel_register_read_special(
    			"/nfs/lib/paracel/lib/libfilter.so",
    			"key_filter");
    
  }

  void test_push() {
    if(get_worker_id() == 0) {
      paracel_write("key1", "value");
      unordered_map<std::string, double> d;
      d["key2"] = 3.; d["key3"] = 4.;
      d["doc_key4"] = 5.;
      d["key5"] = 6.; d["key6"] = 7.;
      paracel_write_multi(d);
    }
  }

  void test_pull() {
    string r1 = paracel_read<string>("key1"); // "value"
    double r2 = paracel_read<double>("key2"); // 3.
    double r3;
    bool f1 = paracel_read<double>("key3", r3); // r3 = 4., f1 = true
    bool f2 = paracel_read<double>("key10", r3); // f2 = false
    vector<string> keys_list = {"key5", "key6"};
    
    // { 6., 7. }
    auto val_list = paracel_read_multi<double>(keys_list);
    
    // compile error since there contains string-type-value
    auto rr = paracel_readall<double>(); 
    
    double result = 0.;
    auto special_read_handle_lambda = 
    [&] (unordered_map<string, V> & d) {
      for(auto & kv : d) {
        result += kv.second;
      }
    };
    
    // compile error since there contains string-type-value
    paracel_readall_handle(special_read_handle_lambda);

    // compile error since there contains stirng-type-value
    auto rrr = paracel_read_special<double>(
    					"/nfs/lib/libfilter.so",
    					"key_filter");

    // result = 5.
    paracel_read_special_handle<double>(
    				"/nfs/lib/libfilter.so",
    				"key_filter",
					special_read_handle_lambda);
    
  }

  void test_update() {
    paracel_update("key5", 4.); // update with registered "sum_updater"
    paracel_update("key5", 3.); 
    // key doesn't exist, following code equals to paracel_write
    paracel_update("new_key1", 1.);
  }

  void test_bupdate() {
    if(get_worker_id() == 0) {
      paracel_bupdate("key2", 7.); // bupdate with registered "sum_updater"
      // specify another update function instead of the registered one
      paracel_bupdate("key3", 6.,
      		"/nfs/lib/paracel/libupdate.so",
			"sum_updater2");
      
      // key doesn't exist, following code equals to paracel_write
      paracel_bupdate("new_key2", 1.);
    
      vector<double> keys = {"key2", "key3", "key5"};
      paracel_bupdate_multi(keys, deltas,
				"/nfs/lib/paracel/libupdate.so",
				"sum_updater2");
    
      unordered_map<string, double> d;
      d["key2"] = 1.; d["key3"] = 1.; d["key5"] = 1.;
      paracel_bupdate_multi(d,
      				"/nfs/lib/paracel/libupdate.so",
					"sum_updater2");
    }
  }

  virtual void solve() {
    test_register();
    paracel_sync();
    
    test_push();
    paracel_sync();
    
    test_pull();
    
    test_update();
    
    test_bupdate();
    paracel_sync();
  }

};

DEFINE_string(server_info,
	"host1:7777PARACELhost2:8888",
	"hosts name string of paracel-servers.\n");

int main(int argc, char *argv[])
{
  paracel::main_env comm_main_env(argc, argv);
  paracel::Comm comm(MPI_COMM_WORLD);
  google::SetUsageMessage("[options]\n\t--server_info\n\t--cfg_file\n");
  google::ParseCommandLineFlags(&argc, &argv, true);
  foo solver(comm, FLAGS_server_info);
  solver.solve();
  return 0;
}

```

Paracel provides a distributed, global key-value store called parameter server. Parameter server is a novel paradigm, which will help you do information exchange much more easily. Messages in Paracel(also defined as parameters) must have a key-value structure. When doing communication, workers only need to interact with servers to read/write/update messages. Communication interfaces in Paracel are really simple and flexible.

###bool paracel_register_update(const std::string & file_name,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;const std::string & func_name);
*Register [update function](#update-function) into Paracel with specified `file_name` and `func_name`. Corresponding interface with `paracel_update` and so on.*

###bool paracel_register_bupdate(const std::string & file_name,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;const std::string & func_name);
*Register [update function](#update-function) into Paracel with specified `file_name` and `func_name`. Corresponding interface with `paracel_bupdate` and so on.*

###bool paracel_register_read_special(const std::string & file_name,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;const std::string & func_name);
*Register [filter function](#filter-function) into Paracel with specified `file_name` and `func_name`. Corresponding interface with `paracel_read_special` and so on.*

###template&lt;class V&gt;<br>bool paracel_read(const std::string & key, V & val);
*Pull `val` from parameter server with specified `key`. If `key` does not exist, return false. Otherwise return true.*

###template&lt;class V&gt;<br>V paracel_read(const std::string & key);
*Pull `val` from parameter server with specified `key`. If `key` does not exist, `ERROR_ABORT` will be invoked.*

###template&lt;class V&gt;<br>std::vector&lt;V&gt;<br>paracel_read_multi(const std::vector&lt;std::string&gt; & keys);
*Pull multiply values from parameter server in one-time. Input argument is a list of keys and return value is the corresponding value with these keys. Every key in `keys` must exist.*

###template&lt;class V&gt;<br>unordered_map&lt;string, V&gt; paracel_readall();
*Pull all key-value pairs from parameter server into an unordered_map.*

###template&lt;class V, class F&gt;<br>void paracel_readall_handle(F & func);
*Pull all key-value pairs from parameter server in batches and handle with functor `func`. It is similar interface with `paracel_readall` while this one can avoid memory exceed problem in large parameter case.*

###template&lt;class V&gt;<br>unordered_map&lt;string, V&gt;<br>paracel_read_special(const std::string & file_name,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;const std::string & func_name);
*Pull key-value pairs from parameter server into an unordered_map with [filter function](#filter-function). The filter function is specified with `file_name` and `func_name`.*

###template&lt;class V, class F&gt;<br>void paracel_read_special_handle(const std::string & file_name,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;const std::string & func_name,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;F & func);
*Pull key-value pairs from parameter server into an unordered_map in batches with handler `func` and [filter function](#filter-function). The filter function is specified with `file_name` and `func_name`.*

###template&lt;class T&gt;<br>void paracel_read_topk(int k, vector&lt;pair&lt;string, T&gt; &gt; & result);
*Read topk key-value pairs(compare by value) from parameter server into a `result` list.*

###template&lt;class T&gt;<br>void paracel_read_topk_with_key_filter(int k,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;vector&lt;pair&lt;string, T&gt; &gt; & result,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;const string & file_name,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;const string & func_name);
*Read topk key-value pairs(compare by value) from parameter server with [filter function](#filter-function) into a `result` list. The filter function is specified with `file_name` and `func_name`.*

###template&lt;class V&gt;<br>bool paracel_write(const std::string & key, const V & val);
*Push key-value pair into parameter server. `paracel_write` is idempotent.*

###template&lt;class V&gt;<br>bool paracel_write_multi(const unordered_map<string, V> & dct);
*Push key-value pairs into parameter server. `paracel_write_multi` is idempotent.*

###template&lt;class V&gt;<br>void paracel_update(const std::string & key, const V & delta);
*Update key&#39;s value with delta in parameter server. The function will be invoked for `delta` with the update function specified in `paracel_register_update`. It is a non-blocking interface which means after returning, the updating operation in the server end may or may not be finished. We recommend you to use `paracel_bupdate` interface in most case to ensure the correctness of your algorithms.*

###template&lt;class V&gt;<br>bool paracel_bupdate(const std::string & key, const V & delta);
*Block update key&#39;s value with delta in parameter server. The function will be invoked for `delta` with the update function specified in `paracel_register_bupdate`. It is a blocking interface which means after returning, the updating operation in the server end must be finished.*

###template &lt;class V&gt;<br>bool paracel_bupdate(const std::string & key, const V & delta,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;const std::string & file_name,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;const std::string & func_name);
*Block update key&#39;s value with delta in parameter server. The function will be invoked for `delta` with the update function specified by `file_name` and `func_name`. It is a blocking interface which means after returning, the updating operation in the server end must be finished.*

###template &lt;class V&gt;<br>bool paracel_bupdate_multi(const std::vector&lt;std::string&gt; & keys,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;const std::vector&lt;V&gt; & deltas<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;const std::string & file_name,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;const std::string & func_name);
*Block update key-value pairs in batches. You must specify the [update function](#update-function) with `file_name` and `func_name`. `deltas` size must be equal with `keys` size.*

###template &lt;class V&gt;<br>bool paracel_bupdate_multi(const unordered_map<string, V> & dct,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;const std::string & file_name, <br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;const std::string & func_name);
*Block update key-value pairs in batches. You must specify the [update function](#update-function) with `file_name` and `func_name`. `dct` refer to the unordered_map with key to delta.*

###bool paracel_contains(const std::string & key);
*Check if `key` is existed in parameter server.*

###bool paracel_remove(const std::string & key);
*Remove key-value pair in parameter server.*

###void paracel_sync();
*Block until all workers have reached this line of code.*

###void set_decomp_info(const std::string & pattern);
*Set decomposition infomation by `pattern`(fmap, smap, fsmap, fvec...).*

###void get_decomp_info(int & x, int & y);
*Get decomposition infomation from `x` and `y`. `x` refers to the number of partition in the horizontal direction while `y` refers to the number of partition in the vertical direction.*

###size_t get_worker_id();
*Get the worker id.*

###size_t get_worker_size();
*Get the size of workers.*

###size_t get_server_size();
*Get the size of servers.*

###paracel::Comm get_comm();
*Get workers&#39; communication channel, you can use the returned channel to do low-level communication between workers. See more interfaces in the [communicator](#communicator) section.*

<aside class="success">
Remember
<p>
<p> - If key do not exist, <b>paracel_update</b>/<b>paracel_bupdate</b> equals to <b>paracel_write</b>.
<p> - If you need more than one registry functions, we suggest to use interfaces that containing the arguments <b>file_name</b> and <b>func_name</b> instead of register ones.
<p> - <b>Paracel_write</b> is idempotent while <b>paracel_bupdate</b> is not. 
<p> - In most cases, we suggest to choose <b>paracel_bupdate</b> to ensure the correctness.
</aside>


## Dump

```cpp

#include <string>
#include <unordered_map>
#include <gflags/gflags.h>
#include "ps.hpp"
#include "utils.hpp"

using std::string;
using std::unordered_map;
using std::vector;
using std::pair;

class foo : public paracel::pralg {
 public:
  foo(paracel::Comm comm, string hosts_dct_str) 
  	: paracel::paralg(hosts_dct_str, comm, "/nfs/tmp/paracel_output/") {}
  
  void save() {
    vector<double> data1 = {1., 2., 3.};
    unordered_map<string, int> data2;
    data2["a"] = 1; data2["b"] = 2;
    unordered_map<string, double> data3;
    data3["c"] = 3.; data3["d"] = 4.;
    unordered_map<string, vector<double> > data4;
    data4["e"] = {1.23, 2.34, 3.45};
    data4["f"] = {5.43, 4.32, 3.21};
    unordered_map<string,
    			vector<pair<string, double> > > data5;
    data5["g"] = { make_pair("i", 3.1415926),
    			make_pair("j", 3.14) };
    data5["h"] = { make_pair("x", 3.1415),
    			make_pair("y", 3.141) };
    paracel_dump_vector(data1, "data1_");
    paracel_dump_dict(data2, "data2_");
    paracel_dump_dict(data3, "data3_");
    paracel_dump_dict(data4, "data4_");
    paracel_dump_dict(data5, "data4_", true);
  }
 
};

DEFINE_string(server_info,
	"host1:7777PARACELhost2:8888",
	"hosts name string of paracel-servers.\n");

int main(int argc, char *argv[])
{
  paracel::main_env comm_main_env(argc, argv);
  paracel::Comm comm(MPI_COMM_WORLD);
  google::SetUsageMessage("[options]\n\t--server_info\n\t--cfg_file\n");
  google::ParseCommandLineFlags(&argc, &argv, true);
  foo solver(comm, FLAGS_server_info);
  solver.save();
  return 0;
}
```

Paracel implements several dumping interfaces below. They are all defined in the `paralg` baseclass. Since there are very limited data types supported, you must write your own dumper in most cases.

###template&lt;class V&gt;<br>void paracel_dump_vector(const std::vector&lt;V&gt; & data,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;const string & file_prefix = "result_",<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;const string & sep = ",",<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;bool append_flag = false,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;bool merge = false);

###void paracel_dump_dict(const unordered_map&lt;string, int&gt;& data,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;const string & file_prefix = "result_",<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;bool append_flag = false,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;bool merge = false);

###void paracel_dump_dict(const unordered_map&lt;string, double&gt;& data,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;const string & file_prefix = "result_",<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;bool append_flag = false,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;bool merge = false);

###template&lt;class T, class P&gt;<br>void paracel_dump_dict(<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;const unordered_map&lt;T, vector&lt;P&gt; &gt; & data,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;const string & file_prefix = "result_",<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;bool append_flag = false,<br>&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;bool merge = false);

###void paracel_dump_dict(<br>&ensp;&ensp;const unordered_map&lt;string, vector&lt;pair&lt;string, double&gt; &gt; &gt; & data,<br>&ensp;&ensp;const string & file_prefix = "result_",<br>&ensp;&ensp;bool append_flag = false,<br>&ensp;&ensp;bool merge = false);

<aside class="notice">
<b>File_prefix</b> refer to the prefix of the output file name. For example, if you use two workers, Paracel will dump results into the output folder with file result_0 and result_1.
</aside>

## Asynchronous Controlling

```cpp
class logistic_regression: public paracel::paralg {

 public:
  logistic_regression(paracel::Comm comm,
  			std::string hosts_dct_str,
			std::string _output,
			int _rounds,
			int _limit_s,
			bool _ssp_switch) : 
		paracel::paralg(hosts_dct_str,
						comm,
		    			_output,
		    			_rounds,
		    			_limit_s,
		    			_ssp_switch) {}

  void training() {
    theta = paracel::random_double_list(data_dim); 
    paracel_write("theta", theta); // init push
    for(int iter = 0; iter < rounds; ++iter) {
      for(int i = 0; i < data_dim; ++i) { 
        delta[i] = 0.; 
      }
      random_shuffle(idx.begin(), idx.end()); 

      // pull theta
      theta = paracel_read<vector<double> >("theta");
      
      for(auto sample_id : idx) {
        for(int i = 0; i < data_dim; ++i) {
	  	  delta[i] += coff1 * 
		  	samples[sample_id][i] - coff2 * theta[i];
		}
      } // traverse
      
      // update theta with delta
      paracel_bupdate("theta",
      				delta,
					"update.so",
					"lg_theta_update");

      // commit to server at the end of each iteration
      iter_commit();
    }

    // last pull
    theta = paracel_read<vector<double> >("theta");
  }

  void solve() {
    // init training data
    auto parser = [](const std::vector<std::string>) {
      /* ... */
    };
    auto lines = paracel_load(input);
    parser(lines);
    paracel_sync(); 
    
    // set total iterations of your training process
    set_total_iters(rounds); 

    // training
    training();
  }
  
}; // class logistic regression 
```

Many machine learning problems can be converted to an iterative task, the traditional way to do that is using [BSP](http://en.wikipedia.org/wiki/Bulk_synchronous_parallel) model which means we must synchronize at the end of every iterator. This leads to the [last reducer problem](http://stackoverflow.com/questions/16836770/last-reducer-tasks-are-taking-long-time-to-finish). The straggled worker came out because of some hareware related reasons such as network congestion, CPU interrupts, produrement of machines in different years and some software related reasons such as garbage collection, virtualization and so on.   

There are two ways to solve this problem: firstly, we have to write some tricky code to make load imbalance so we could make a fast worker training more data. Secondly, we can do some asynchronous controlling to relax the synchronization condition.    

Paracel use the second solution, we relax the synchronization condition with an assumption that the fastest worker can lead no more than a bounded parameter with the slowest worker which is a trade-off between convergence of every iteration and total time of convergence. We have some similar ideas with the [SSP](https://www.usenix.org/conference/hotos13/session/cipar) data structure. The attractive point is that you only have to add few lines of code to transform a BSP process to an asynchronous process. The asynchronous iterfaces are listed below and we will show you a very simple example in the right.

###void iter_commit();
*Commit to parameter servers at the end of each iteration.*

###void set_total_iters(int n);
*Set total number of iterations. You have to use this function to tell Paracel the total number of iterations beforehand.*

###template&lt;class V&gt;<br>V get_cache(const std::string & key);
*Get cached value of specified `key` locally.*

###bool is_cached(const std::string & key);
*Check whether the value of specified `key` is cached locally.*

Except above functions, there are two important parameters you have to use: `ssp_switch` and `limit_s`. `ssp_switch` is the switch and `limit_s` is the bound parameter, all of them must be set in the [constructor](#initialize) of paralg baseclass.     

The logistic regression example on the right describe the usage in detail. As you can see, you only need to add four lines of code in original version.

# Registry Function

A registry function is a user-define function that interacts with Paracel framework.

You must follow the predefined interface below and can do anything you want inside a registry function. Then you need to register your function into Paracel. After compiling and installing your code, you can specify the registry function with the shared library file name and function name you defined.

###// substitute `name` with your update function name<br>template &lt;class T&gt;<br>T name(T & a, T & b);
###// substitute `name` with your filter function name<br>bool name(const std::string & key);

## Update Function

```cpp
/* update.cpp */
#include "proxy.hpp"
#include "paracel_types.hpp"

// wrap c++ function here
extern "C" {
  extern paracel::update_result sum_updater;
}

// define your update function here
// return type must be the same with parameter a and parameter b
double foo(double a, double b) { return a + b; }

// register into Paracel framework
paracel::update_result sum_updater = paracel::update_proxy(foo);
```

A update registry function can be used together with `paracel_update`, `paracel_bupdate` and so on. It is something like the reduce function in map-reduce paradigm which will be dynamicly executed in the server side. For example, in the code area we define a simple sum updater in file `update.cpp` and in the corresponding `CMakeLists.txt` file we add:
###add_library(update SHARED update.cpp)
###target_link_libraries(update ${CMAKE_DL_LIBS})
###install(TARGETS update LIBRARY DESTINATION lib)

<p>After compiling and installing the code, you can specify your update function with the shared library file name:`xxx/lib/libupdate.so` and the function name inside: `sum_updater`(here xxx refer to your path for Paracel installation).

## Filter Function

```cpp
/* filter.cpp */
#include "proxy.hpp"
#include "paracel_types.hpp"

// wrap c++ function here
extern "C" {
  extern paracel::filter_with_key_result key_filter;
}

// define your filter function here
// return bool, parameter must be std::string
bool foo(const std::string & key) {
  if(paracel::startswith(key, "doc")) return true;
  return false;
}

// register into Paracel framework
paracel::filter_with_key_result key_filter = paracel::filter_with_key_proxy(foo);
```

A filter registry function can be used together with `paracel_read_special` and `paracel_read_special_handle` which will read special key-value pairs from all paramter servers. Workers can only pull key-value pairs that the filter function return true.

###add_library(filter SHARED filter.cpp)
###target_link_libraries(filter ${CMAKE_DL_LIBS})
###install(TARGETS filter LIBRARY DESTINATION lib)

<p>Similarly, you must add compile information in the corresponding `CMakeLists.txt` and after compiling and installing the code, you can specify your filter function with the shared library file name:`xxx/lib/libfilter.so` and the function name inside: `key_filter`(here xxx refer to your path for Paracel installation).

In the right example, say we have `{"doc_key1" : 1.23, "key2" : "world"}` stored in parameter server, and invoking<p>`paracel_read_special("xxx/lib/libfilter.so", "key_filter")`<p>will only return `{"doc_key1" : 1.23}`.

# Utility

Paracel provide some scattered utility for common use functionality and we are planning to encapsulate them with high-level abstraction in next release.

## Communicator

```cpp

// worker number = 2
#include <vector>
#include <tuple>
#include <string>
#include <set>
#include <unordered_map>
#include <mpi.h>
#include "utils/comm.hpp"

int main(int argc, char *argv[])
{
  paracel::main_env comm_main_env(argc, argv);
  paracel::Comm comm(MPI_COMM_WORLD); 
  int rk = comm.get_rank();
  int sz = comm.get_size();
  comm.get_rank();

  { // builtin send + recv
    if(rk == 0) {
      int a = 7; 
      comm.send(a, 1, 2014);
    } else if(rk == 1){
      int b = 0;
      comm.recv(b, 0, 2014);
    }
  }
  
  { // isend + recv
    if(rk == 0) {
      int a = 7; 
      paracel::vrequest req;
      req = comm.isend(a, 1, 2014);
    } else if(rk == 1){
      int b = 0;
      comm.recv(b, 0, 2014);
    }
  }

  { // container send + recv
    if(rk == 0) {
      std::vector<int> aa {77, 88};
      comm.send(aa, 1, 2014);
    } else if(rk == 1) {
      std::vector<int> bb;
      comm.recv(bb, 0, 2014);
    }
  }

  { // container isend + recv
    if(rk == 0) {
      std::vector<int> aa {77, 88};
      paracel::vrequest req;
      req = comm.isend(aa, 1, 2014);
    } else if(rk == 1) {
      std::vector<int> bb;
      comm.recv(bb, 0, 2014);
    }
  }

  { // paracel triple send + recv
    if(rk == 0) {
      std::tuple<std::string, std::string, double> aa;
      std::get<0>(aa) = "abc";
      std::get<1>(aa) = "def";
      std::get<2>(aa) = 3.14;
      paracel::vrequest req;
      req = comm.isend(aa, 1, 2014);
    } else if(rk == 1) {
      std::tuple<std::string, std::string, double> bb;
      comm.recv(bb, 0, 2014);
    }
  }

  { // paracel list of triple send + recv
    if(rk == 0) {
      std::vector<std::tuple<std::string,
      				std::string, double> > aa;
      std::tuple<std::string, std::string, double> tmp1;
      std::get<0>(tmp1) = "abc"; std::get<1>(tmp1) = "def";
      std::get<2>(tmp1) = 4.15; aa.push_back(tmp1);
      std::tuple<std::string, std::string, double> tmp2;
      std::get<0>(tmp2) = "cba"; std::get<1>(tmp2) = "fed";
      std::get<2>(tmp2) = 5.16; aa.push_back(tmp2);
      paracel::vrequest req;
      req = comm.isend(aa, 1, 2014);
    } else if(rk == 1) {
      std::vector<std::tuple<std::string,
      				std::string, double> > bb;
      comm.recv(bb, 0, 2014);
    }
  }

  { // another paracel list of triple send + recv
    if(rk == 0) {
      std::vector<std::tuple<std::string,
      				std::string, double> > aa;
      std::tuple<std::string, std::string, double> tmp1;
      std::get<0>(tmp1) = "abc"; std::get<1>(tmp1) = "def";
      std::get<2>(tmp1) = 4.15; aa.push_back(tmp1);
      std::tuple<std::string, std::string, double> tmp2;
      aa.push_back(tmp2);
      paracel::vrequest req;
      req = comm.isend(aa, 1, 2014);
    } else if(rk == 1) {
      std::vector<std::tuple<std::string,
      				std::string, double> > bb;
      comm.recv(bb, 0, 2014);
    }
  }
  
  { // builtin sendrecv
    int a = 8;
    int b;
    int left, right;
    right = (rk + 1) % sz;
    left = rk - 1;
    if(left < 0) left = sz - 1;
    comm.sendrecv(a, b, left, 2014, right, 2014);
  }

  { // container sendrecv
    std::vector<int> aaa{1,2,3};
    std::vector<int> bbb;
    int left, right;
    right = (rk + 1) % sz;
    left = rk - 1;
    if(left < 0) left = sz - 1;
    comm.sendrecv(aaa, bbb, left, 2014, right, 2014);
  }

  { // paracel triple sendrecv
    std::tuple<std::string, std::string, double> aa;
    std::tuple<std::string, std::string, double> bb;
    std::get<0>(aa) = "abc"; std::get<1>(aa) = "def";
    std::get<2>(aa) = 3.14; int left, right;
    right = (rk + 1) % sz;
    left = rk - 1;
    if(left < 0) left = sz - 1;
    comm.sendrecv(aa, bb, left, 2014, right, 2014);
  }

  { // paracel list of triple sendrecv
    std::vector<std::tuple<std::string,
    				std::string, double> > bb;
    std::vector<std::tuple<std::string,
    				std::string, double> > aa;
    std::tuple<std::string, std::string, double> tmp1;
    std::get<0>(tmp1) = "abc"; std::get<1>(tmp1) = "def";
    std::get<2>(tmp1) = 4.15; aa.push_back(tmp1);
    std::tuple<std::string, std::string, double> tmp2;
    std::get<0>(tmp2) = "cba"; std::get<1>(tmp2) = "fed";
    std::get<2>(tmp2) = 5.16; aa.push_back(tmp2);
    int left, right;
    right = (rk + 1) % sz;
    left = rk - 1;
    if(left < 0) left = sz - 1;
    comm.sendrecv(aa, bb, left, 2014, right, 2014);
  }
  
  { // another paracel list of triple sendrecv
    std::vector<std::tuple<std::string,
    				std::string, double> > bb;
    std::vector<std::tuple<std::string,
    				std::string, double> > aa;
    std::tuple<std::string, std::string, double> tmp1;
    std::get<0>(tmp1) = "abc"; std::get<1>(tmp1) = "def";
    std::get<2>(tmp1) = 4.15;
    std::tuple<std::string, std::string, double> tmp2;
    aa.push_back(tmp2); aa.push_back(tmp1);
    int left, right;
    right = (rk + 1) % sz;
    left = rk - 1;
    if(left < 0) left = sz - 1;
    comm.sendrecv(aa, bb, left, 2014, right, 2014);
  }

  { // debug for list of triple sendrecv
    std::vector<std::vector<std::tuple<std::string,
    					std::string, double> > > aa(2);
    std::vector<std::tuple<std::string,
    				std::string, double> > bb;
    int t = -1, f = -1;
    if(rk == 0) {
      std::vector<std::tuple<std::string,
      				std::string, double> > aaa;
      std::tuple<std::string, std::string, double> tmp1;
      std::get<0>(tmp1) = "a"; std::get<1>(tmp1) = "b";
      std::get<2>(tmp1) = 1.; aaa.push_back(tmp1);
      std::get<0>(tmp1) = "a"; std::get<1>(tmp1) = "c";
      std::get<2>(tmp1) = 1.; aaa.push_back(tmp1);
      std::get<0>(tmp1) = "a"; std::get<1>(tmp1) = "d";
      std::get<2>(tmp1) = 1.; aaa.push_back(tmp1);
      std::get<0>(tmp1) = "b"; std::get<1>(tmp1) = "a";
      std::get<2>(tmp1) = 1.; aaa.push_back(tmp1);
      aa[1] = aaa; t = 1; f = 1;
    } else if(rk == 1) {
      std::vector<std::tuple<std::string,
      				std::string, double> > aaa;
      std::tuple<std::string, std::string, double> tmp1;
      std::get<0>(tmp1) = "e"; std::get<1>(tmp1) = "a";
      std::get<2>(tmp1) = 1.; aaa.push_back(tmp1);
      std::get<0>(tmp1) = "e"; std::get<1>(tmp1) = "d";
      std::get<2>(tmp1) = 1.; aaa.push_back(tmp1);
      aa[0] = aaa;

      std::vector<std::tuple<std::string,
      				std::string, double> > aaaa;
      std::tuple<std::string, std::string, double> tmp2;
      std::get<0>(tmp2) = "b"; std::get<1>(tmp2) = "d";
      std::get<2>(tmp2) = 1.; aaaa.push_back(tmp2);
      std::get<0>(tmp2) = "d"; std::get<1>(tmp2) = "c";
      std::get<2>(tmp2) = 1.; aaaa.push_back(tmp2);
      aa[1] = aaaa; f = 0; t = 0;
    }
    comm.sendrecv(aa[t], bb, t, 2014, f, 2014);
  }

  { // builtin bcast
    int a;
    if(rk == 0) a = 7;
    comm.bcast(a, 0);
  }

  { // container bcast
    std::vector<int> aa(2);
    if(rk == 0) {
      aa[0] = 3; aa[1] = 4;
    }
    comm.bcast(aa, 0);
  }
  
  { // builtin alltoall
    std::vector<int> a(2), b(2);
    if(rk == 0) {
      a[0] = 1; a[1] = 3;
    }
    if(rk == 1) {
      a[0] = 2; a[1] = 4;
    }
    comm.alltoall(a, b);
  }

  { // container alltoall
    std::vector< std::vector<int> > a(2), b;
    if(rk == 0) {
      std::vector<int> tmp1{1, 5};
      std::vector<int> tmp2{3};
      a[0] = tmp1; a[1] = tmp2;
    }
    if(rk == 1) {
      std::vector<int> tmp1{2};
      std::vector<int> tmp2{4, 7};
      a[0] = tmp1; a[1] = tmp2;
    }
    comm.alltoall(a, b);
  }

  { // builtin allreduce
    int aaa;
    if(rk == 0) { aaa = 1; }
    if(rk == 1) { aaa = 2; }
    comm.allreduce(aaa);
  }
  
  { // container allreduce
    std::vector<int> aaa(3);
    if(rk == 0) {
      aaa[0] = 1; aaa[1] = 2; aaa[2] = 3;
    }
    if(rk == 1) {
      aaa[0] = 3; aaa[1] = 2; aaa[2] = 1;
    }
    comm.allreduce(aaa);
  }

  { // bcastring
    std::vector<int> a;
    if(rk == 0) {
      a.push_back(6); a.push_back(42);
    }
    if(rk == 1) {
      a.push_back(28); a.push_back(42);
      a.push_back(42); a.push_back(28); a.push_back(6);
    }
    std::set<int> result;
    auto func = [&](std::vector<int> tmp){
      for(auto & stf : tmp)
        result.insert(stf);
    };
    comm.bcastring(a, func);
  }

  { // dict_type<size_t, int> isend
    if(rk == 0) {
      std::unordered_map<size_t, int> aa;
      aa[0] = 1; aa[1] = 2;
      paracel::vrequest req;
      req = comm.isend(aa, 1, 2014);
    } else if(rk == 1) {
      std::unordered_map<size_t, int> bb;
      comm.recv(bb, 0, 2014);
    }
  }

  return 0;
}
```

Communicator stands for the collection of workers. Paracel&#39;s communicator has the same meaning as [MPI communicator](http://static.msi.umn.edu/tutorial/scicomp/general/MPI/communicator.html). We provide a C++ interface, but we are still suggesting you to do communication between parameter servers unless very special cases. Do not use low-level interfaces given in this section unless you know what you&#39;re doing.

###class Comm {<br><br>&nbsp;public:<br>&nbsp;&nbsp;Comm(MPI_Comm comm = MPI_COMM_WORLD);
*&nbsp;&nbsp;Constructor*
###&nbsp;&nbsp;inline size_t get_size() const;
*&nbsp;&nbsp;Get the number of workers.*
###&nbsp;&nbsp;inline size_t get_rank() const;
*&nbsp;&nbsp;Get the id of worker.*
###&nbsp;&nbsp;inline MPI_Comm get_comm() const;
*&nbsp;&nbsp;Get communicator field.*
###&nbsp;&nbsp;inline void synchronize();
*&nbsp;&nbsp;Block until all workers in this communicator have reached this routine.*
###&nbsp;&nbsp;int get_source(MPI_Status & stat);
*&nbsp;&nbsp;Get source id.*
###&nbsp;&nbsp;Comm split(int color);
*&nbsp;&nbsp;Split the communicator into sub-communicators using `color`.*
###&nbsp;&nbsp;void wait(MPI_Request & req);
*&nbsp;&nbsp;Wait for an MPI request to complete.*
###&nbsp;&nbsp;void wait(vrequest & v_req);
*&nbsp;&nbsp;Wait for an MPI request to complete*
###&nbsp;&nbsp;template&lt;class T&gt;<br>&nbsp;&nbsp;void send(const T & data, int dest, int tag);
*&nbsp;&nbsp;Block send messages.*
###&nbsp;&nbsp;template&lt;class T&gt;<br>&nbsp;&nbsp;void isend(const T & data, int dest, int tag);
*&nbsp;&nbsp;Non-blocking send messages.*
###&nbsp;&nbsp;template&lt;class T&gt;<br>&nbsp;&nbsp;void recv(T & data, int src, int tag);
*&nbsp;&nbsp;Receive messages.*
###&nbsp;&nbsp;template&lt;class T&gt;<br>&nbsp;&nbsp;void sendrecv(const T & sdata, T & rdata,<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;int sto, int stag,<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;int rfrom, int rtag);
*&nbsp;&nbsp;Receive `rdata` from `rfrom`, and send `sdata` to `sto`.*
###&nbsp;&nbsp;template&lt;class T&gt;<br>&nbsp;&nbsp;void bcast(T & data, int master);
*&nbsp;&nbsp;Broadcast master&#39;s `data`.*
###&nbsp;&nbsp;template&lt;class T, class F&gt;<br>&nbsp;&nbsp;void bcastring(const T & data, F & func);
*&nbsp;&nbsp;Broadcast every worker&#39;s `data` and handle with functor `func`.*
###&nbsp;&nbsp;template&lt;class T&gt;<br>&nbsp;&nbsp;void alltoall(const T & sbuf, T & rbuf);
*&nbsp;&nbsp;Send data from all to all workers.*
###&nbsp;&nbsp;template&lt;class T, class F&gt;<br>&nbsp;&nbsp;void alltoallring(const T & sbuf, T & rbuf, F & func);
*&nbsp;&nbsp;Send data from all to all workers and handle with functor `func`.*
###&nbsp;&nbsp;template&lt;class T&gt;<br>&nbsp;&nbsp;void allreduce(T & data);
*&nbsp;&nbsp;Combine values from all workers and distributes the result back to all workers. Only supported summation in current version.*
###&nbsp;&nbsp;template&lt;class T, class F&gt;<br>&nbsp;&nbsp;T treereduce(T & data, F & func, int rank = 0);
*&nbsp;&nbsp;Treereduce using functor `func`.*

###}; // class Comm

<br><br>
<aside class = "notice">
Actually, the return type of above iterfaces are very flexiable because there are so many override versions. Here we just all use <b>void</b> for convenience while it does not affect the use.
</aside>

## Json_parser

```cpp
#include "utils.hpp"

void json_parser_usage() {
  paracel::json_parser pt("demo.json");
  std::string r1 = pt.parse<std::string>("wu"); // "hong"
  int r2 = pt.parse<int>("hong"); // 7
  bool r3 = pt.parse<bool>("changsheng"); // true
  double r4 = pt.parse<double>("jiang"); // 3.141592653
  
  // {"hong", "xun", "zhang"}
  std::vector<std::string> r5 = pt.parse_v<std::string>("wul");
  
  // {1, 2, 3, 4, 5, 6, 7}
  std::vector<int> r6 = pt.parse_v<int>("hongl");
  
  // {true, false, false, true, true}
  std::vector<bool> r7 = pt.parse_v<bool>("changshengl");
  
  // {1.23, 2.34, 3.45, 4.56, 5.67, 6.78, 7.89}
  std::vector<double> r8 = pt.parse_v<double>("jiangl");
}
```

```json
{
  "wu" : "hong",
  "hong" : 7,
  "changsheng" : true,
  "jiang" : 3.141592653,
  "wul" : ["hong", "xun", "zhang"],
  "hongl" : [1, 2, 3, 4, 5, 6, 7],
  "changshengl" : [true, false, false, true, true],
  "jiangl" : [1.23, 2.34, 3.45, 4.56, 5.67, 6.78, 7.89]
}
```

For some design reasons, the configuration information of algorithms/applications building upon paracel must be read from a config file in stead of command line arguments. We highly recommend [JSON](http://json.org/). Paracel provide a rough json parser which do not support comment and must be parsered in sequence. The simple parser may be not that flexible but can avoid unpredictable mistakes. The `check_parser` and `check_parser_v` interface below will check if the value is a file or a directory.

###struct json_parser {<br><br>&nbsp;public:<br>&nbsp;&nbsp;json_parser(paracel::str_type fn);
###&nbsp;&nbsp;template &lt;class T&gt;<br>&nbsp;&nbsp;T parse(const paracel::str_type & key);
###&nbsp;&nbsp;template &lt;class T&gt;<br>&nbsp;&nbsp;T check_parse(const paracel::str_type & key);
###&nbsp;&nbsp;template &lt;class T&gt;<br>&nbsp;&nbsp;std::vector&lt;T&gt; parse_v(const paracel::str_type & key);
###&nbsp;&nbsp;template &lt;class T&gt;<br>&nbsp;&nbsp;std::vector&lt;T&gt; check_parse_v(const paracel::str_type & key);
###};

## Random

```cpp
#include "utils.hpp"

double pi() {

  int incycle_cnt = 0;
  for(int i = 0; i < 10000000; ++i) {
    double x = paracel::random_double();
    double y = paracel::random_double();
    if(x * x + y * y < 1.)  incycle_cnt += 1;
  }
  return 4 * static_cast<double>(incycle_cnt) / 10000000.;

}
```

Random is the basic assumption of many fields of algorithms. Below we provide two simple interface to randomly generate double value(s) for some initialization usage of machine learning algorithms. More details and interfaces can be found [here](http://www.cplusplus.com/reference/random/).

###// return a uniform random double value in range(0, 1.)<br>double random_double();
###// return a list of random double value with upper bound `upper_bnd`<br>std::vector&lt;double&gt; random_double_list(size_t len, double upper_bnd = 1.);

## String

```cpp
#include "utils.hpp"

void foo() {
  {
    std::vector<std::string> init_lst 
		= {"hello", "world", "happy", "new", "year", "2015"};
    std::string seps = "orz";
    
    // helloorzworldorzhappyorzneworzyearorz2015
    auto together = paracel::str_join(init_lst, seps);
    auto res1 = paracel::str_split_by_word(together, seps); // init_lst
    
    std::string tmp = together;
    // init_lst
    auto res2 = paracel::str_split_by_word(std::move(tmp), seps);
    
    paracel::startswith(together, "hello"); // true
    paracel::startswith(together, "helo"); // false
    paracel::startswith(together, ""); // true
    paracel::endswith(together, "2015"); // true
    paracel::endswith(together, "2014"); // false
    paracel::endswith(together, ""); // true
  }
  {
    std::string tmp = "a|b|c|d|e";
    std::vector<std::string> r = {"a", "b", "c", "d", "e"};
    auto r1 = paracel::str_split(tmp, '|'); // r
    auto r2 = paracel::str_split(tmp, "|"); // r
    auto r3 = paracel::str_split(tmp, "?|?"); // r
  }
}
```

Paracel provide several extra operations which are included in `std::string`.

###std::vector&lt;std::string&gt;<br>str_split(const std::string & str, const char sep);
###std::vector&lt;std::string&gt;<br>str_split(const std::string & str, const std::string & seps);
###std::vector&lt;std::string&gt;<br>str_split_by_word(const std::string & str, const std::string & seps);
###std::vector&lt;std::string&gt;<br>str_split_by_word(std::string && str, const std::string & seps);
###std::string str_join(const std::vector&lt;std::string&gt; & strlst, const std::string & seps);
###bool startswith(const std::string & str, const std::string & key);
###bool endswith(const std::string & str, const std::string & key);

## Misc

```cpp
#include "ps.hpp"
#include "utils.hpp"
#include "paracel_types.hpp"

void foo() {
  pt = new paralg(comm);
  paracel::default_id_type id = 7;
  pt->paracel_write(cvt(id), 3.14);
  double val = pt->paracel_read<double>(cvt(id));
  delete pt;
}

void goo() {
  paracel::hash_type<paracel::default_id_type> hfunc;
  paracel::default_id_type a = 0, b = 1, c = 2, d = 3;
  hfunc(a); // 0
  hfunc(b); // 1
  hfunc(c); // 2
  hfunc(d); // 3
  paracel::hash_type<std::string> hfunc2;
  std::string x = "0", y = "1", z = "2", t = "3";
  a = 2297668033614959926ULL;
  b = 10159970873491820195ULL;
  c = 4551451650890805270ULL;
  d = 8248777770799913213ULL;
  hfunc2(x); // a
  hfunc2(y); // b
  hfunc2(z); // c
  hfunc2(t); // d
}
```

###template&lt;class T&gt; struct hash { &nbsp;&nbsp;size_t operator()(const T &t) const; };
###double dot_product(const std::vector&lt;double&gt; & a, const std::vector&lt;double&gt; & b);
###paracel::default_id_type cvt(std::string id);
###std::string cvt(paracel::default_id_type id);

<aside class="success">
Remember - the key of communication messages must be `std::string`, so you need to convert other type of keys before communication with parameter server.
</aside>

# DataGen 

```shell
python ./tool/datagen.py -m wc -o data.txt

python datagen.py -o sample1.dat -m classification -n 100000 -k 800 

python datagen.py -o sample2.dat -m regression -n 100000 -k 100 

python ./tool/datagen.py -m pagerank -o pr.dat

python datagen.py -o sample3.dat -m similarity -n 100000 -k 800 

python datagen.py -o sample4.dat -m kmeans -n 1000 --ncenters 20 -k 80

# for lda, n refer to the number of documents while k refer to the number of topics
python datagen.py -o sample5.dat -m lda -n 100000 -k 800 -s '|' 

python ./tool/datagen.py -m svd -o svd.dat
```

To generate training data for machine learning algorithms and ensure repeatability, we provide a python script named `datagen.py` in the `tool` folder. Click right `shell` tab to see some examples.

Options | Description
--------- | -----------
-h, --help | show this help message and exit
-m METHOD, --method=METHOD | wc, classification, regression, pagerank, similarity, kmeans, lda, svd...
  -o OUTPUT, --out=OUTPUT | output file name 
  -s SEP, --sep=SEP | seperator, default: `,` 
  -n SIZE, --datasize=SIZE |  number of training samples
  -k K, --nfeatures=K | number of features 
  --ncenters=NCENTERS | number of centers for kmeans method

# Driver 

```shell
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:your_paracel_install_path/lib

./prun.py -p 1 -w 16 -m local -c demo_config.json ./a.out

./prun.py -p 10 -w 100 -m mesos --ppn 10 -c word_count_config.json ./local/bin/wc

./prun.py -p 10 --m_server mesos --ppn_server 2 --mem_limit_server 1000 -w 100 -m mesos --ppn 10 --mem_limit 500 -c pagerank_config.xml ./local/bin/pagerank
```

We provide a python script named `prun.py` to run a Paracel programs. Firstly, you have to modify the `PARACEL_INSTALL_PREFIX` variable in `prun.py`, by default it is set to the local directory in the Paracel&#39; home directory. Secondly, you have to specify the path of the installed shared libraries in the environment variable `LD_LIBRARY_PATH`. Then you can run your program using `prun.py` of the Paracel&#39;s home directory. Click right `shell` tab to see some examples.

Options | Description
------- | -----------
-h, --help | show this help message and exit
-p PARASRV_NUM, <br>--snum<br>&nbsp;&nbsp;=PARASRV_NUM | number of parameter servers
--m_server <br>&nbsp;&nbsp;=local &#124; mesos &#124; mpi | running method for parameter servers. <br>If not given, set with the same value of -m or <br>--method
--ppn_server <br>&nbsp;&nbsp;=PPN_SERVER | mesos case: procs number per node of parameter servers. <br>If not given, set with the same value of <br>--ppn
--mem_limit_server<br>&nbsp;&nbsp;=MEM_LIMIT_SERVER | mesos case: memory size of each task in parameter servers. <br>If not given, set with the same value of <br>--mem_limit
--hostfile_server <br>&nbsp;&nbsp;=HOSTFILE_SERVER | mpi case: hostfile for mpirun of parameter servers.<br>If not given, set with the same value of <br>--hostfile
-w WORKER_NUM, <br>--wnum<br>&nbsp;&nbsp;=WORKER_NUM | number of workers for learning
-m local &#124; mesos &#124; mpi,<br>--method<br>&nbsp;&nbsp;=local &#124; mesos &#124; mpi | running method for workers
--ppn=PPN | mesos case: procs number per node for workers
--mem_limit<br>&nbsp;&nbsp;=MEM_LIMIT | mesos case: memory size of each task of workers
--hostfile=HOSTFILE | mpi case: hostfile for mpirun for workers
-c CONFIG,<br>--cfg_file=CONFIG | config file in json fmt, for alg usage

<aside class="notice">
<br>
1. <b>-c</b> option is a must, all algorithm related parameters have to be written in a config file.
<br>
2. The <b>--m_server</b> and <b>-m</b> options above refer to what type of cluster you use. Paracel support <a href="http://mesos.apache.org/">mesos</a> clusters, mpi clusters and multiprocessers in a single machine. 
<br>
3. Executable files have to be specified at the end.
</aside>

# Toolkits
In current version, [Paracel Toolkits](https://github.com/douban/paracel/tree/master/alg/) contains the following algorithms:

* [alg/regression/ridge](https://github.com/douban/paracel/tree/master/alg/regression/ridge)
* [alg/classification/logistic_regression](https://github.com/douban/paracel/tree/master/alg/classification/logistic_regression)
* [alg/clusterting/kmeans](https://github.com/douban/paracel/tree/master/alg/clustering/kmeans)
* [alg/graph_alg/pagerank](https://github.com/douban/paracel/tree/master/alg/graph_alg/pagerank)
* [alg/recommendation/matrix_factorization](https://github.com/douban/paracel/tree/master/alg/recommendation/matrix_factorization)
* [alg/recommendation/similarity_sparse](https://github.com/douban/paracel/tree/master/alg/recommendation/similarity_sparse)
* [alg/recommendation/simialrity_dense](https://github.com/douban/paracel/tree/master/alg/recommendation/similarity_dense)
* [alg/recommendation/decision_tree_rec](https://github.com/douban/paracel/tree/master/alg/recommendation/decision_tree_rec)
* [alg/topic_model/gLDA](https://github.com/douban/paracel/tree/master/alg/topic_model)
* [tool/lasso_serial](https://github.com/douban/paracel/tree/master/tool#lasso)
* [tool/lr_l1_serial](https://github.com/douban/paracel/tree/master/tool#logistic_regression_l1)
* [tool/svd_serial](https://github.com/douban/paracel/tree/master/tool#svd)

Refer to the link for more description and usage examples.

# Deployment

Paracel needs some libraries pre-installed in the environment of your cluster.<br>Just follow instructions below step by step:

## Prerequisites
There are a few prerequisites which must be manually satisfied including:

* [g++](https://gcc.gnu.org/) (>= 4.7) [Required]
  *  Required for compiling Paracel.
<br>
* [CMake](www.cmake.org) (>= 2.8.9) [Required]
  *  Should come with most Mac/Linux systems by default. Recent Ubuntu version will require to install the build-essential package.
<br>
* Any version of MPI([Open MPI](http://www.open-mpi.org/) or [MPICH2](http://www.mpich.org/)) [Required]
  *  Required for running Paracel code distributed.

## Dependencies

We give different installation guide on different platforms:

### Gentoo

We provide the needed ebuild files for you in the `ebuild` directory and you can install all this libraries in any order.

### Ubuntu(12.04 LTS) / Debian(Wheezy)
1. ```sudo apt-get install libboost-dev```
<br><br>
2. ```sudo apt-get install libzmq-dev```
<br>*Make sure the version is greater than 3.2.4 by typing `apt-cache show libzmq-dev`, if no related version found, you must manually download and install it at [here](http://zeromq.org/intro:get-the-software).*
<br><br>
3. ```sudo apt-get install libeigen3-dev```
<br><br>
4. *Install a increment version of [Msgpack-C](https://github.com/xunzhang/msgpack-c) which supports some C++11 types. Here you must manually clone and install the library:*
<br>```git clone https://github.com/xunzhang/msgpack-c.git```
<br>```cd msgpack-c;```
<br>```./bootstrap; ./configure; make; sudo make install```
<br><br>
5. ```sudo apt-get install libgflags2```<br> *Make sure version of gflags is 2.0, if no corresponding version could be found, you have to install [it](https://github.com/schuhschuh/gflags/archive/v2.0.tar.gz) manually:*
<br>```git clone https://github.com/schuhschuh/gflags.git```
<br>```cd gflags```
<br>```git checkout v2.0;```
<br>```./configure; make; sudo make install```

### OpenSUSE(13.2)
1. ```sudo zypper in boost-devel```
<br><br>
2. ```sudo zypper in zeromq-devel```
<br>*Make sure the version is greater than 3.2.4 by typing `zypper info zeromq_devel`, if no related version found, you must manually download and install it at [here](http://zeromq.org/intro:get-the-software).*
<br><br>
3. ```sudo zypper in eigen3-devel```
<br><br>
4. *Install a increment version of [Msgpack-C](https://github.com/xunzhang/msgpack-c) which supports some C++11 types. Here you must manually clone and install the library:*
<br>```git clone https://github.com/xunzhang/msgpack-c.git```
<br>```cd msgpack-c;```
<br>```./bootstrap; ./configure; make; sudo make install```
<br><br>
5. ```sudo apt-get install libgflags2```<br> *Make sure version of gflags is 2.0, if no corresponding version could be found, you have to install [it](https://github.com/schuhschuh/gflags/archive/v2.0.tar.gz) manually:*
<br>```git clone https://github.com/schuhschuh/gflags.git```
<br>```cd gflags```
<br>```git checkout v2.0;```
<br>```./configure; make; sudo make install```

### Centos(7)
1. ```sudo yum install boost-devel```
<br><br>
2. ```sudo yum install zeromq-devel```
<br>*Make sure the version is greater than 3.2.4 by typing `yum info zeromq-devel`, if no related version found, you must manually download and install it at [here](http://zeromq.org/intro:get-the-software).*
<br><br>
3. ```sudo yum install  eigen3-devel```
<br><br>
4. *Install a increment version of [Msgpack-C](https://github.com/xunzhang/msgpack-c) which supports some C++11 types. Here you must manually clone and install the library:*
<br>```git clone https://github.com/xunzhang/msgpack-c.git```
<br>```cd msgpack-c;```
<br>```./bootstrap; ./configure; make; sudo make install```
<br><br>
5. ```sudo yum install gflags2.0```<br> *Make sure version of gflags is 2.0, if no corresponding version could be found, you have to install [it](https://github.com/schuhschuh/gflags/archive/v2.0.tar.gz) manually:*
<br>```git clone https://github.com/schuhschuh/gflags.git```
<br>```cd gflags```
<br>```git checkout v2.0;```
<br>```./configure; make; sudo make install```

### ArchLinux
1. ```sudo pacman -S boost```
<br><br>
2. ```sudo pacman -S zeromq```
<br>*Make sure the version is greater than 3.2.4.*
<br><br>
3. ```sudo pacman -S eigen3```
<br><br>
4. *Install a increment version of [Msgpack-C](https://github.com/xunzhang/msgpack-c) which supports some C++11 types. Here you must manually clone and install the library:*
<br>```git clone https://github.com/xunzhang/msgpack-c.git```
<br>```cd msgpack-c;```
<br>```./bootstrap; ./configure; make; sudo make install```
<br><br>
5. ```sudo pacman -S gflags2.0```<br> *Make sure version of gflags is 2.0, if no corresponding version could be found, you have to install [it](https://github.com/schuhschuh/gflags/archive/v2.0.tar.gz) manually:*
<br>```git clone https://github.com/schuhschuh/gflags.git```
<br>```cd gflags```
<br>```git checkout v2.0;```
<br>```./configure; make; sudo make install```

### Mac OS X
Ditto

## Downloading Paracel

You can download Paracel directly from the github repository. The git command line for cloning the reposotory is:
	
	```git clone https://github.com/douban/paracel.git```
	<br><br>
	```cd paracel```

## Compiling Paracel

```mkdir build; cd build```

```cmake -DCMAKE_BUILD_TYPE=Release ..```
<br>*You can also specify `-DCMAKE_INSTALL_PREFIX` with the path where you want Paracel to be installed. For example, type <br>```cmake -DCMAKE_INSTALL_PREFIX=your_paracel_install_path ..``` instead.*

```make -j 4```
<br>*The above command will perform up to 4 build tasks in parallel, you can specify the number of tasks you want according to your machine.*

```make install```

# Paracel FAQ

<br>
**How does Paracel relate to MPI?**
<br>
Paracel use MPI to create worker processers, then parallelly load input data(it will do some communication in this step). Communication between workers and servers is nothing to do with MPI. And we provide a C++ wrapper for MPI communication, you can directly do message passing between workers in some cases. See more details in the [Communicator](#communicator) section.
<br><br>

**What is parameter server?**
<br>
Parameter server is a global and distributed key-value store brings to a novel way for communication. In this paradigm, if worker `W1` needs to talk to worker `W2`, `W1` has to push his words to server `S` for `W2` to pull. It is kind of indirect, but you can get more flexibility and simplicity from that. 
<br><br>

**What is the difference between Paracel and Spark/GraphLab framework?**
<br>
The question is hard to answer in brief, Spark and GraphLab are both outstanding distributed computational framework with many advantages, but there are also some limitations of them:
<br>Spark project follows the mapreduce paradigm which is not that straightforward and flexible for algorithms in some specialized domains such as machine learning, graph and so on. If you think the transformation of RDD as a set of basis functions, they are not fit for all the applications. For example, Spark implements Bagel for graph processing which follows Google&#39;s Pregel graph processing model(not mapreduce at all). Spark is more suitable and easily used for data statistics problems and problems with narrow dependency.
<br>Graphlab framework is designed for graph algorithms which focus more on efficiency. In the other end, your application code is so tricky that it will look very different from its original logic. Also you have to transform your machine learning problems to GraphLab&#39;s model and it may be too low-level for application developers.
<br>Paracel is a framework in between, it is more high-level and easily to use than GraphLab and more low-level/flexible than Spark, users can write communication code in Paracel. Developers may focus more on their application logic to build distributed algorithms. Paracel is original designed for machine learning problems.
<br><br>

**What programming languages does Paracel support?**
<br>
C++ only, for the consideration of computational efficiency.
<br><br>

**What is the largest data size Paracel can scale to?**
<br>
It depends on the number of processers you have. At Douban, the usual data size is about 100GB.
<br><br>

**Is Paracel only fit for machine learning problems?**
<br>
Not exactly, Paracel is designed for many machine problems as well as graph algorithms and scientific problems. For example, pagerank algorithm in Paracel is really straightfoward compare to a two-phase mapreduce implementation.
<br><br>

**How can I write a distributed algorithm upon Paracel framework?**
<br>
Follow [the third section](http://paracel.io/docs/quick_tutorial.html#WriteaParacelApplication) in quick tutorial page step by step.
<br><br>

**How large a cluster can Paracel scale to?**
<br>
In the current version, we have tested the cluster scale to 60 plus nodes with 1000 plus processers.
<br><br>

**How can I run Paracel on a cluster?**
<br>
Firstly, set up a cluster environment with either mpi or mesos. Secondly, make sure Paracel is successfully installed on your clusters. More details can be found [here](#deployment). Then use `-m` option with `mpi` or `mesos` in your `prun.py` script.
<br><br>

**Can I run Paracel programs with docker?**
<br>
<br><br>

**How can I contribute to Paracel?**
<br>
Just fork the [repository](https://github.com/douban/paracel/fork) and send a pull request on [github](https://github.com/douban/paracel).
<br><br>

**What new features will Paracel import in the future?**
<br>More data source formats such as gzip and bz2.
<br>Streaming Paracel: fault tolerance to ensure service-like application reliable.
<br>Data flow interfaces with which you can process data like a pipeline.
<br>ParacelSQL.
<br><br>

> 1.Deadlock mistake

```cpp
void deadlock_mistake() {
  if(get_worker_id() == 0) {
    paracel_sync();
  }
}
```

> 2.Wrong type mistake

```cpp
void wrong_type_mistake() {
  paracel_write("key1", 1.2);
  paracel_write("key2", 3.4);
  vector<string> keys = {"key1", "key2"};
  // use double in stead of vector<double>
  auto vals = paracel_read_multi<vector<double> >(keys);
}
```

**This document is terrible and I have a lot of questions, where can I get more help?**
<br>
You can initiate a discussion at this [group](https://groups.google.com/forum/#!forum/paracel) or write an email to <xunzhangthu@gmail.com>, <algorithm@douban.com>.
<br><br>

**Can I write a Paracel application outside the Paracel directory?**
<br>
Yes, of course. In this case you must modify cmake files and be responsible to the linking relations of your code. If you are not professional, we strongly do not suggest you to do that.
<br><br>

**Common mistakes.**
<br>
1. Deadlock mistake: in the example on the right, only worker 0 can access the `if` clause while `paracel_sync` requires all workers to execute this line.<br>
2. Wrong type specified mistake: see the interface at [Paralg](#communication) section, the return type is `std::vector<V>`, so in the right example, you have to only use `double`.
<br><br>
