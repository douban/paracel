/*
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

#ifndef ALG_TOPIC_MODEL_GLDA_HPP
#define ALG_TOPIC_MODEL_GLDA_HPP

#include<cstdio>
#include<cstdlib>
#include<ctime>
#include<cmath>

#include<string>
#include<vector>
#include <chrono>
#include <random>
#include<algorithm>
#include<unordered_map>

#include "ps.hpp"
#include "utils.hpp"

namespace paracel {
namespace alg {

class LDAmodel: public paracel::paralg {

 public:
	LDAmodel(Comm comm,
           std::string hosts_dct_str,
           std::string input,
           std::string output,
           double alpha,
           double beta,
           int k_topics,
           int iters,
           int top_words,
           std::string handle_fn,
           int debug) : paralg(hosts_dct_str, comm),
                        input(input),
                        output(output),
                        alpha(alpha),
                        beta(beta),
                        K(k_topics),
                        max_iter(iters),
                        top(top_words),
                        handle_fn(handle_fn),
                        debug(debug) {
		worker_id = (int) get_worker_id();
		worker_size = (int) get_worker_size();
		load(input);
		printf("worker: %d, docs: %d, dicts: %lu, words: %d, global dicts: %d\n",
           worker_id,
           M,
           local_dict_list.size(),
           T,
           V);
		top = std::min(top, V);
		Kalpha = alpha * K;
		Vbeta = beta * V;
		para_init();
	}

	~LDAmodel(){
		if(k_prob) delete[] k_prob;
		if(sum_doc2topic) delete[] sum_doc2topic;
		if(doc2topic){
			for(int i = 0; i < M; i++){
				if(doc2topic[i]) delete[] doc2topic[i];
			}
			delete[] doc2topic;
		}

		if(z_index){
			for(int i = 0; i < M; i++){
				if(z_index[i]) delete[] z_index[i];
			}
			delete[] z_index;
		}

		if(debug){
			if(theta){
				for(int i = 0; i < M; i++){
					if(theta[i]) delete[] theta[i];
				}
				delete[] theta;
			}

			if(phi){
				for(int i = 0; i < K; i++){
					if(phi[i]) delete[] phi[i];
				}
				delete[] phi;
			}
		}
	}

	void train(){
		for(int i = 0; i < max_iter; i++) {
			single_iter();
			if(worker_id == 0) {
				if(debug) 
					printf("ITER:%d, LOGLIKELIHOOD:%f, PERPLEXITY:%f, COMPUTE:%f, UPDATE:%f\n",
                 i + 1,
                 likelihood(),
                 exp(-1 * likelihood() / T),
                 compute,
                 update);
				else
					printf("ITER:%d, COMPUTE:%f, UPDATE:%f\n",
                 i + 1,
                 compute,
                 update);
			}
		}
	}

	void save(){
		paracel_sync();
		time_t start = time(NULL);
		std::unordered_map<std::string, double> dump_phi;
		for(int j = 0; j < K; j++){
			for(auto it = local_dict_list.begin(); it != local_dict_list.end(); it++) {
				auto value = word2topic[*it];
				if(value[j] == 0) continue;
				dump_phi["key_" + std::to_string(j) + "_" + *it] = (value[j] + beta) / (sum_topic2word[j] + Vbeta);
			}
			if(j % 5 == 0 || j == K - 1){
				paracel_write_multi(dump_phi);
				dump_phi.clear();
			}
		}
		paracel_sync();
		printf("shuffle wirte done, %f\n", difftime(time(NULL), start));

		start = time(NULL);
		FILE* f = fopen((output + std::to_string(worker_id)).c_str(), "w");
		std::unordered_map<int, std::vector<std::pair<int, double> > > dump_data;
		auto parser_key = [&] (std::unordered_map<std::string, double> tmp_value) {
			for(auto it = tmp_value.begin(); it != tmp_value.end(); it++) {
				auto key = it->first;
				auto p1 = key.find("_") + 1;
				auto p2 = key.rfind("_");
				int tid = std::stoi(key.substr(p1, p2 - p1));
				if(tid % worker_size != worker_id) continue;
				int wid = std::stoi(key.substr(p2 + 1, key.size() - p2 - 1));
				dump_data[tid].push_back(std::make_pair(wid, it->second));
			}
		};
		paracel_read_special_handle<double>(handle_fn,
                                        "gLDA_dump_filter",
                                        parser_key);
		printf("shuffle read done, %f\n", difftime(time(NULL), start));
	
		for(auto it = dump_data.begin(); it != dump_data.end(); it++){
			auto tid = it->first;
			auto value = it->second;
			std::sort(value.begin(), value.end(), [] (std::pair<int, double> a,
                                                std::pair<int, double> b) {
                                              return a.second > b.second;
                                            });

			fprintf(f, "%dtopic\t", tid);
			for(int j = 0; j < std::min(top, (int)value.size()); j ++){
				auto word_id = value[j].first;
				auto word = id2word[word_id];
				fprintf(f, "%s:%f|", word.c_str(), value[j].second);
			}
			fprintf(f, "\n");
		}

		fclose(f);
	}

private:
	void load(std::string file_name){
		word2id.clear();
		id2word.clear();
		std::string sep(" \t\r\n");
		std::vector<std::string> res;
		auto parser_dict = [&] (const std::string line){
			std::vector<int> tmp;
			if(split(line, sep, res)){
				for(auto it = res.begin(); it != res.end(); it++){
          auto finder = word2id.find(*it);
          if(finder == word2id.end()) {
            word2id[*it] = V;
						V++;
          }
				}
			}
		};

		paracel_load_handle(file_name, parser_dict);
		paracel_bupdate("global_dict",
                    word2id,
                    handle_fn,
                    "gLDA_dict_update");
		paracel_sync();
		word2id = paracel_read<std::unordered_map<std::string, int>>("global_dict");
		V = word2id.size();
		for(auto it = word2id.begin(); it != word2id.end(); it++) {
      id2word[it->second] = it->first;
    }

		std::unordered_map<int, int> local_dict;
		auto parser_document = [&] (const std::string line) {
			std::vector<int> tmp;
			if(split(line, sep, res)) {
				for(auto it = res.begin(); it != res.end(); it++) {
					auto idx = word2id[*it];
					tmp.push_back(idx);
          auto finder = local_dict.find(idx);
          if(finder == local_dict.end()) local_dict[idx] = 1;
					T++;
				}
			}
			docs.push_back(tmp);
			M++;
		};

		paracel_load_handle(file_name, parser_document);
		for(auto it = local_dict.begin(); it != local_dict.end(); it++) {
      local_dict_list.push_back(std::to_string(it->first));
    }
  }

	bool split(const std::string & src,
             std::string sep,
             std::vector<std::string>& res) {
		int N = src.size();
		if (N == 0  || sep.size() == 0) {
			return false;
		}
		res.clear();
		int start = src.find_first_not_of(sep);
		int stop = 0;
		while(start >= 0 && start < N) {
			stop = src.find_first_of(sep, start);
			if(stop < 0 || stop > N) {
				stop = N;
      }
			res.push_back(src.substr(start, stop - start));
			start = src.find_first_not_of(sep, stop + 1);
		}
		return true;
	}

	void para_init(){
		k_prob = new double[K];
		for(int i = 0; i < K; i++) {
      k_prob[i] = 1.0 / K;
    }
		doc2topic = new int*[M];
		for(int i = 0; i < M; i++){
			doc2topic[i] = new int[K];
			for(int j = 0; j < K; j++)
				doc2topic[i][j] = 0;
		}

		for(auto it = local_dict_list.begin(); it != local_dict_list.end(); it++){
			word2topic[*it] = std::vector<int>(K, 0);
		}

		sum_doc2topic = new int[M];
		for(int i = 0; i < M; i++) sum_doc2topic[i] = 0;
		for(int i = 0; i < K; i++) {
      global_topic_key.push_back("sum_topic_" + std::to_string(i));
    }
		sum_topic2word = std::vector<int>(K, 0);

		time_t cur_time = time(NULL);
		cur_time += (10 * worker_id);
		srand(cur_time);
		last_word2topic = word2topic;
		last_sum_topic2word = sum_topic2word;
		z_index = new int*[M];
		for(int doc_id = 0; doc_id < M; doc_id++){
			auto tmp_doc = docs[doc_id];
			int N = tmp_doc.size();
			z_index[doc_id] = new int[N];
			for(int word_index = 0; word_index < N; word_index++){
				int topic_id = (int) (ran_uniform() * K);
				z_index[doc_id][word_index] = topic_id;
				int word_id = tmp_doc[word_index];
				add(doc_id, word_id, topic_id);
			}
		}
		last_word2topic = delta_new(word2topic, last_word2topic);
		last_sum_topic2word = delta_sum(sum_topic2word, last_sum_topic2word);
		paracel_bupdate_multi(last_word2topic,
                          handle_fn,
                          "gLDA_word_update");
		paracel_bupdate_multi(global_topic_key,
                          last_sum_topic2word,
                          handle_fn,
                          "gLDA_sum_topic_update");
		paracel_sync();
		auto new_word2topic = paracel_read_multi<std::vector<int> >(local_dict_list);
		update_word_info(new_word2topic);
		sum_topic2word = paracel_read_multi<int>(global_topic_key);
		paracel_sync();
		last_word2topic = word2topic;
		last_sum_topic2word = sum_topic2word;
		printf("init done, worker: %d\n", worker_id);

		if(debug){
			theta = new double*[M];
			for(int i = 0; i < M; i++) theta[i] = new double[K];
			phi = new double*[K];
			for(int i = 0; i < K; i++) phi[i] = new double[V];
		}
	}

	std::unordered_map<std::string, std::vector<int> >
  delta_new(std::unordered_map<std::string, std::vector<int> > & now,
            std::unordered_map<std::string, std::vector<int>> & last) {
		for(auto it = local_dict_list.begin(); it != local_dict_list.end(); it++) {
			auto now_vec = now[*it];
			auto last_vec = last[*it];
			for(auto j = 0; j < K; j++) {
				last_vec[j] = now_vec[j] - last_vec[j];
      }
			last[*it] = last_vec;
		}
		return last;
	}

	std::vector<int> delta_sum(std::vector<int>& now, std::vector<int>& last) {
		for(auto i = 0; i < K; i++) {
			last[i] = now[i] - last[i];
    }
		return last;
	}

	void update_word_info(std::vector<std::vector<int> > & new_word) {
		for(size_t i = 0; i < local_dict_list.size(); i++) {
			auto key = local_dict_list[i];
			auto value = new_word[i];
			word2topic[key] = value;
		}
	}

	void show_info(){
		for(int doc_id = 0; doc_id < M; doc_id++) {
			auto tmp_doc = docs[doc_id];
			int N = tmp_doc.size();
			for(int word_index = 0; word_index < N; word_index++) {
				printf("Z:%d ", z_index[doc_id][word_index]);
      }
			printf("\n");
		}
		for(int i = 0; i < M; i++) printf("sumDOC:%d\n", sum_doc2topic[i]);
		for(int i = 0; i < K; i++) printf("sumTOP:%d\n", sum_topic2word[i]);
		for(int doc_id = 0; doc_id < M; doc_id++) {
			for(int i = 0; i < K; i++) {
				printf("d2t:%d ", doc2topic[doc_id][i]);
      }
			printf("\n");
		}
		for(auto it = local_dict_list.begin(); it < local_dict_list.end(); it++){
			auto value = word2topic[*it];
			for(int j = K - 2; j < K; j++) {
				printf("w2t:%d ", value[j]);
      }
			printf("%s\n", id2word[std::stoi(*it)].c_str());
		}
	}

	void add(int doc_id, int word_id, int topic_id, int num = 1){
		doc2topic[doc_id][topic_id] += num;
		sum_doc2topic[doc_id] += num;
		word2topic[std::to_string(word_id)][topic_id] += num;
		sum_topic2word[topic_id] += num;
	}

	void remove(int doc_id, int word_id, int topic_id){
		add(doc_id, word_id, topic_id, -1);
	}

	double ran_uniform(){
		return rand() / (RAND_MAX + 1.0);
	}

	int ran_multinomial(double* prob, int k){
		int i;
		for(i = 1; i < k; i++) {
			prob[i] += prob[i - 1];
    }
		double p = ran_uniform() * prob[k - 1];
		for(i = 0; i < k; i++){
			if(prob[i] > p) break;
		}
		return i;
	}

	void single_iter(){
		time_t start = time(NULL);
		for(int doc_id = 0; doc_id < M; doc_id++) {
			auto tmp_doc = docs[doc_id];
			int N = tmp_doc.size();
			for(int word_index = 0; word_index < N; word_index++) {
				int topic_id = z_index[doc_id][word_index];
				int word_id = tmp_doc[word_index];
				remove(doc_id, word_id, topic_id);
				auto value = word2topic[std::to_string(word_id)];
				for(int k = 0; k < K; k++) {
					k_prob[k] = (doc2topic[doc_id][k] + alpha) / (sum_doc2topic[doc_id] + Kalpha) *
								(value[k] + beta) / (sum_topic2word[k] + Vbeta);
				}
				topic_id = ran_multinomial(k_prob, K);
				z_index[doc_id][word_index] = topic_id;
				add(doc_id, word_id, topic_id);
			}
		}
		compute += difftime(time(NULL), start);
		start = time(NULL);
		last_word2topic = delta_new(word2topic, last_word2topic);
		last_sum_topic2word = delta_sum(sum_topic2word, last_sum_topic2word);
		paracel_bupdate_multi(last_word2topic,
                          handle_fn,
                          "gLDA_word_update");
		paracel_bupdate_multi(global_topic_key,
                          last_sum_topic2word,
                          handle_fn,
                          "gLDA_sum_topic_update");
		paracel_sync();
		auto new_word2topic = paracel_read_multi<std::vector<int> >(local_dict_list);
		update_word_info(new_word2topic);
		sum_topic2word = paracel_read_multi<int>(global_topic_key);
		last_word2topic = word2topic;
		last_sum_topic2word = sum_topic2word;
		update += difftime(time(NULL), start);
	}

	double likelihood(){
		theta_phi();
		double sum = 0.0;
		for(int doc_id = 0; doc_id < M; doc_id++) {
			auto tmp_doc = docs[doc_id];
			int N = tmp_doc.size();
			double tmp_log = 0.00001;
			for(int word_index = 0; word_index < N; word_index++) {
				int word_id = tmp_doc[word_index];
				for(int k = 0; k < K; k++) {
					tmp_log += theta[doc_id][k] * phi[k][word_id];
				}
			sum += log(tmp_log);
			}
		}
		return sum;
	}

	void theta_phi(){
		for(int i = 0; i < M; i++) {
			for(int j = 0 ; j < K; j++) {
				theta[i][j] = (doc2topic[i][j] + alpha) / (sum_doc2topic[i] + Kalpha);
      }
    }
		for(auto it = local_dict_list.begin(); it != local_dict_list.end(); it++) {
			int i = std::stoi(*it);
			auto value = word2topic[*it];
			for(int j = 0; j < K; j++) {
				phi[j][i] = (value[j] + beta) / (sum_topic2word[j] + Vbeta);
			}
		}
	}

 private:
	std::string input, output;
	double alpha, beta, Kalpha, Vbeta;
	int K, max_iter, top;
	int M = 0, V = 0, T = 0;
	std::string handle_fn;
	int debug = 0;
	
  std::vector<std::vector<int> > docs;
	std::unordered_map<std::string, int> word2id;
	std::unordered_map<int, std::string> id2word;
	std::vector<std::string> local_dict_list;
	double* k_prob;
	int** doc2topic;
	std::unordered_map<std::string, std::vector<int> > word2topic;
	std::unordered_map<std::string, std::vector<int> > last_word2topic;
	int** z_index;
	int* sum_doc2topic;
	std::vector<int> sum_topic2word;
	std::vector<int> last_sum_topic2word;
	std::vector<std::string> global_topic_key;
	double** theta;
	double** phi;
	double compute = 0, update = 0;
	int worker_id, worker_size;
 
}; // class LDA

} // namespace alg
} // namespace paracel

#endif
