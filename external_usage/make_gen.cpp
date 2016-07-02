#include <unordered_map>
#include <string>
#include "utils.hpp"

int main(int argc, char *argv[])
{
  std::unordered_map<std::string, std::string> T;
  T["@PARACEL_INSTALL_PREFIX@"] = "/opt/paracel";
  T["@CXX@"] = "mpic++";
  T["@REGISTERY_SOURCE@"] = "registery.cpp";
  T["@DRIVER_SOURCE@"] = "driver.cpp";
  T["@REGISTERY_TARGET@"] = "registery.so";
  T["@DRIVER@"] = "demo";
  paracel::file_replace("Makefile.in", "Makefile", T);
  return 0;
}
