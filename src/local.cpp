#include <functional>
#include <vector>
#include <string>

#include "paracel_types.hpp"
#include "proxy.hpp"
#include "utils.hpp"

using update_result = std::function<std::string(std::string, std::string)>;
using filter_result = std::function<bool(std::string, std::string)>;

extern "C" {
  extern update_result local_update;
  extern filter_result local_filter;
  extern filter_result local_filter2;
  extern filter_result local_filter_remove;
}

double foo(double a, double b) { 
  return a * b; 
}

bool goo0(std::string k, double v) {
  if(v >= 12.) return true;
  return false;
}

bool goo1(std::string k, double v) {
  paracel::str_type s = "_3";
  if(paracel::endswith(k, s)) { 
    return true; 
  }
  return false;
}

bool too0(std::string k, double v) {
  if(v >= 14.) return true;
  return false;
}

bool too1(std::string k, double v) {}

update_result local_update = paracel::update_proxy(foo);
filter_result local_filter = paracel::filter_proxy(goo0);
filter_result local_filter2 = paracel::filter_proxy(goo1);
filter_result local_filter_remove = paracel::filter_proxy(goo0);
