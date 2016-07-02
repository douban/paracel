#include <string>

#include "proxy.hpp"
#include "paracel_types.hpp"
#include "utils.hpp"

extern "C" {
  extern paracel::filter_with_key_result demo_filter;
}

bool local_filter(const std::string & key) {
  std::string s = "external_0";
  if(paracel::startswith(key, s)) {
    return true;
  }
  return false;
}

paracel::filter_with_key_result demo_filter = paracel::filter_with_key_proxy(local_filter);
