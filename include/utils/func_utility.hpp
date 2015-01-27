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

#ifndef FILE_5301a774_a9c4_df8d_d8e4_62d952066bb5_HPP
#define FILE_5301a774_a9c4_df8d_d8e4_62d952066bb5_HPP

#include <tuple>

namespace paracel {

// lambda & functor
template <class F>
struct f_traits {
public:

  using call_type = f_traits<decltype(&F::operator())>;

public:

  using result_type = typename call_type::result_type;
  
  static constexpr std::size_t arity = call_type::arity - 1;
  
  template <std::size_t N>
  
  struct args {
    static_assert(N < arity, "Error: invalid parameter index.");
    using type = typename call_type::template args<N+1>::type;
  };
};

// func ref
template <class F>
struct f_traits<F&> : public f_traits<F> {};

// func rval ref
template <class F>
struct f_traits<F&&> : public f_traits<F> {};

template <class R, class ...Args>
struct f_traits< R(Args...) > {

public:
  using result_type = R;  
  
  static constexpr std::size_t arity = sizeof...(Args);
  
  template <std::size_t N>
  struct args {
    static_assert(N < arity, "Error: invalid parameter index.");
    using type = typename std::tuple_element<N, std::tuple<Args...> >::type;
  };
};

// func pt
template <class R, class ...Args>
struct f_traits< R(*)(Args...) > : public f_traits< R(Args...) > {};

// const mem func pt
template <class C, class R, class ...Args>
struct f_traits< R(C::*)(Args...) const > : public f_traits< R(C&, Args...) > {};

// mem func pt
template <class C, class R, class ...Args>
struct f_traits< R(C::*)(Args...) > : public f_traits< R(C&, Args...)> {};

// mem obj pt
template <class C, class R>
struct f_traits< R(C::*) > : public f_traits< R(C&) > {};

} // namespace paracel

#endif
