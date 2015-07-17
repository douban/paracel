#include <iostream>
#include <vector>
#include "balltree.hpp"

int main(int argc, char *argv[])
{
  {
    std::vector<std::vector<double> > items = 
    {
      {2.8, 3.9}, {2.1, 2.7}, {2.8, 3.1}, {3., 2.8},
      {3.1, 3.}, {2.6, 9.1}, {3.5, 9.2}, {3.1, 8.6},
      {3.6, 8.8}, {8.2, 7.6}, {9.2, 8.5}, {9.3, 7.5},
      {8.3, 6.3}, {8., 6.}, {8.4, 6.1}, {9., 6.4},
      {9.4, 6.8}, {9.2, 6.6}, {9.1, 6.1}, {7.9, 3.7},
      {8.8, 3.2}, {9.1, 2.7}, {8.7, 1.8}, {8.9, 1.5}
    };
    paracel::balltree<double> stree(items);
    paracel::balltree<double> stree2(items);
    stree.build();
    std::cout << "stree built" << std::endl;

    stree.pickle("tmp.bt");
    std::cout << "pickle done" << std::endl;
    stree2.build_from_file("tmp.bt");
    std::cout << "load done" << std::endl;

    std::vector<double> tmp = {1., 2.};
    paracel::query q(tmp, 7);
    std::vector<long> r;
    paracel::search(q, stree, r);
    for(auto & i : r) {
      std::cout << i << std::endl;
    }
    std::cout << "-------------------------------------" << std::endl;
    paracel::search(q, stree2, r);
    for(auto & i : r) {
      std::cout << i << std::endl;
    }
    std::cout << "-------------------------------------" << std::endl;
    // linear search
    paracel::search(q, items, r);
    for(auto & i : r) {
      std::cout << i << std::endl;
    }
  }
  return 0;
}
