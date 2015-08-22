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
 * Authors: Ping Qin <qinping@douban.com>
 *
 */

#include <vector>
#include "proxy.hpp"
#include "paracel_types.hpp"
#include "utils.hpp"

using std::vector;

extern "C" {
  extern paracel::update_result gLDA_dict_update;
  extern paracel::update_result gLDA_word_update;
  extern paracel::update_result gLDA_sum_topic_update;
  extern paracel::filter_with_key_result gLDA_dump_filter;
}

std::unordered_map<std::string, int>
dict_update(std::unordered_map<std::string, int> & a,
            std::unordered_map<std::string, int> & b) {
	int V = a.size();
	for(auto it = b.begin(); it != b.end(); it++) {
		auto word = it -> first;
    auto finder = a.find(word);
    if(finder == a.end()) {
			finder->second = V;
			V++;
		}
	}
	return a;
}

std::vector<std::vector<int> > 
topic_update(std::vector<std::vector<int>> & a,
             std::vector<std::vector<int>> & b){
	for(size_t i = 0; i < b.size(); i++) {
		for(size_t j = 0; j < b[0].size(); j++) {
			a[i][j] = a[i][j] + b[i][j];
    }
	}
	return a;
}

std::vector<int> word_update(std::vector<int> & a,
                             std::vector<int> & b) {
	for(size_t i = 0; i < b.size(); i++) {
		a[i] = a[i] + b[i];
	}
	return a;
}

int sum_topic_update(int a, int b){
	return a + b;
}

bool key_filter(const std::string & key) {
	std::string s = "key_";
	if(paracel::startswith(key, s)) {
		return true;
		}
	return false;
}

paracel::update_result gLDA_dict_update = paracel::update_proxy(dict_update);
paracel::update_result gLDA_topic_update = paracel::update_proxy(topic_update);
paracel::update_result gLDA_word_update = paracel::update_proxy(word_update);
paracel::update_result gLDA_sum_topic_update = paracel::update_proxy(sum_topic_update);
paracel::filter_with_key_result gLDA_dump_filter = paracel::filter_with_key_proxy(key_filter);
