/**
 * Copyright (c) 2014, Douban Inc. 
 *   All rights reserved. 
 *
 * Distributed under the BSD License. Check out the LICENSE file for full text.
 *
 * Paracel - A distributed optimization framework with parameter server.
 *
 * Downloading
 *   git clone https://github.com/douban/paracel.git
 *
 * Authors: Hong Wu <xunzhangthu@gmail.com>
 *
 */

#ifndef FILE_607caade_25c4_da56_7a7b_c0cd1aa35b74_HPP
#define FILE_607caade_25c4_da56_7a7b_c0cd1aa35b74_HPP

#include <functional>
#include "paracel_types.hpp"
#include "utils/ext_utility.hpp"

namespace paracel {

/**
 * example: 1001|1|2|4
 *   std::bind(parser_a, std::placeholders::_1, '|')
 *
 * example: 1001 1 2 4
 *   std::bind(parser_a, std::placeholders::_1)
 *
 */
auto parser_a_int = 
  [] (const paracel::str_type & line, char sep = ' ') {
    auto l = paracel::str_split(line, sep);
    paracel::list_type<int> r;
    for(auto & item : l) {
      r.push_back(std::stoi(item));
    }
    return r;
  }; 

/**
 * example: a b c d 
 *          ...
 *   f = std::bind(parser_a, std::placeholders::_1) 
 *   f(l) -> ['a', 'b', 'c', 'd']
 *   a.txt
 *
 * example: a b
 *          a c
 *          a d
 *          ...
 *   f = std::bind(parser_a, std::placeholders::_1)
 *   f(l) -> ['a', 'b']
 *   b.txt
 *
 * example: a b
 *          a c d
 *          ...
 *   f = std::bind(parser_a, std::placeholders::_1)
 *   f(l) -> ['a', 'b']
 *   c.txt
 *
 */
auto parser_a = 
  [] (const paracel::str_type & line, char sep = ' ') {
    return paracel::str_split(line, sep);
  };

/**
 * example: a b|c|d
 *          b a|d
 *          ...
 *   f = std::bind(parser_b, std::placeholders::_1, ' ', '|') 
 *   f(l) -> ['a', 'b', 'c', 'd']
 *   a2.txt
 *
 * example: a b:0.1|c:0.2|d:0.4
 *   f = std::bind(parser_b, std::placeholders::_1, ' ', '|')
 *   f(l) -> ['a', 'b:0.1', 'c:0.2', 'd:0.4']
 *   f.txt
 *
 * example: a\tb:0.1
 *          a\tc:0.2 d:0.4
 *   f = std::bind(parser_b, std::placeholders::_1, '\t')
 *   f(l) -> ['a', 'b:0.1']
 *   h.txt
 *
 */
auto parser_b = 
  [] (const paracel::str_type & line, 
     char sep1 = ' ', 
     char sep2 = ' ') {
    auto tmp = paracel::str_split(line, sep1);
    auto r = paracel::str_split(tmp[1], sep2);
    r.insert(r.begin(), tmp[0]);
    return r;
  };

using parser_type = std::function<paracel::list_type<paracel::str_type>(paracel::str_type)>;

template <class T>
parser_type gen_parser(T & parser) {
  return std::bind(parser, std::placeholders::_1);
}

template <class T>
parser_type gen_parser(T & parser, char sep1) {
  return std::bind(parser, std::placeholders::_1, sep1);
}

template <class T>
parser_type gen_parser(T & parser, char sep1, char sep2) {
  return std::bind(parser, std::placeholders::_1, sep1, sep2);
}

} // namespace paracel

#endif
