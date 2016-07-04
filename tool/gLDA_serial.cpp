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


#include<cstdio>
#include<cstdlib>
#include<ctime>
#include<cmath>
#include<string>
#include<vector>
#include<unordered_map>
#include<algorithm>
#include <gflags/gflags.h>
#include "ps.hpp"
#include "utils.hpp"

namespace paracel {
namespace tool {

class LDAmodel{

public:

	LDAmodel(Comm comm,
           std::string input,
           std::string output,
           double alpha,
           double beta,
           int k_topics,
           int iters,
           int top_words) : input(input),
                            output(output),
                            alpha(alpha),
                            beta(beta),
                            K(k_topics),
                            max_iter(iters),
                            top(top_words) {
		pt = new paralg(comm);
		load(input);
		printf("Total docs:%d, dicts:%d, words:%d\n", M, V, T);
		top = std::min(top, V);
		printf("Top %d\n", top);
		Kalpha = alpha * K;
		Vbeta = beta * V;
		para_init();
	}

	~LDAmodel(){
		if(pt) delete pt;
		if(k_prob) delete[] k_prob;
		if(sum_doc2topic) delete[] sum_doc2topic;
		if(sum_topic2word) delete[] sum_topic2word;
		if(doc2topic) {
			for(int i = 0; i < M; i++) {
				if(doc2topic[i]) delete[] doc2topic[i];
			}
			delete[] doc2topic;
		}
		if(topic2word) {
			for(int i = 0; i < K; i++) {
				if(topic2word[i]) delete[] topic2word[i];
			}
			delete[] topic2word;
		}
		if(z_index) {
			for(int i = 0; i < M; i++) {
				if(z_index[i]) delete[] z_index[i];
			}
			delete[] z_index;
		}

		if(theta) {
			for(int i = 0; i < M; i++) {
				if(theta[i]) delete[] theta[i];
			}
			delete[] theta;
		}

		if(phi) {
			for(int i = 0; i < K; i++) {
				if(phi[i]) delete[] phi[i];
			}
			delete[] phi;
		}
	}

	void train() {
		for(int i = 0; i < max_iter; i++) {
			single_iter();
			printf("ITER:%d, LOGLIKELIHOOD:%f, PERPLEXITY:%f\n",
             i + 1,
             likelihood(),
             exp(-1 * likelihood() / T));
		}
	}

	void save(){
		theta_phi();
		FILE* f = fopen(output.c_str(), "w");

	  auto sort_comp = [] (std::pair<int, double> a,
                         std::pair<int, double> b) {
		  return a.second > b.second;
	  };

		for(int i = 0; i < K; i++) {
			std::vector<std::pair<int, double> > words_probs;
			std::pair<int, double> word_prob;
			for (int j = 0; j < V; j++) {
				word_prob.first = j;
				word_prob.second = phi[i][j];
				words_probs.push_back(word_prob);
			}
			std::sort(words_probs.begin(), words_probs.end(), sort_comp);

			fprintf(f, "%dtopic\t", i);
			for(int j = 0; j < top; j ++) {
				auto word_id = words_probs[j].first;
				auto word = id2word[word_id];
				fprintf(f, "%s:%f|", word.c_str(), words_probs[j].second);
			}
			fprintf(f, "\n");
		}
		fclose(f);
	}

 private:
	void load(std::string file_name) {
		word2id.clear();
		id2word.clear();
		std::string sep(" \t\r\n");
		std::vector<std::string> res;
		auto parser = [&] (const std::string line){
			std::vector<int> tmp;
			if(split(line, sep, res)) {
				for(auto it = res.begin(); it != res.end(); it++) {
					if(!word2id.count(*it)) {
						word2id[*it] = V; 
						id2word[V] = *it;
						V++;
          }
					tmp.push_back(word2id[*it]);
					T++;
				}
			}
			docs.push_back(tmp);
			M++;
		};
		pt->paracel_load_handle(file_name, parser);
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

	void para_init() {
		k_prob = new double[K];
		for(int i = 0; i < K; i++) k_prob[i] = 1.0 / K;

		doc2topic = new int*[M];
		for(int i = 0; i < M; i++) {
			doc2topic[i] = new int[K];
			for(int j = 0; j < K; j++) {
				doc2topic[i][j] = 0;
      }
		}

		topic2word = new int*[K];
		for(int i = 0; i < K; i++) {
			topic2word[i] = new int[V];
			for(int j = 0; j < V; j++) {
				topic2word[i][j] = 0;
      }
		}

		sum_doc2topic = new int[M];
		for(int i = 0; i < M; i++) sum_doc2topic[i] = 0;
		sum_topic2word = new int[K];
		for(int i = 0; i < K; i++) sum_topic2word[i] = 0;

		srand(time(NULL));
		z_index = new int*[M];
		for(int doc_id = 0; doc_id < M; doc_id++) {
			auto tmp_doc = docs[doc_id];
			int N = tmp_doc.size();
			z_index[doc_id] = new int[N];
			for(int word_index = 0; word_index < N; word_index++) {
				int topic_id = (int) (ran_uniform() * K);
				z_index[doc_id][word_index] = topic_id;
				int word_id = tmp_doc[word_index];
				add(doc_id, word_id, topic_id);
			}
		}

		theta = new double*[M];
		for(int i = 0; i < M; i++) theta[i] = new double[K];
		phi = new double*[K];
		for(int i = 0; i < K; i++) phi[i] = new double[V];
	}

	void show_info() {
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
		for(int i = 0; i < K; i++) {
			for(int j = 0; j < V; j++) {
				printf("t2w:%d ", topic2word[i][j]);
      }
			printf("\n");
		}
	}

	void add(int doc_id, int word_id, int topic_id, int num = 1) {
		doc2topic[doc_id][topic_id] += num;
		sum_doc2topic[doc_id] += num;
		topic2word[topic_id][word_id] += num;
		sum_topic2word[topic_id] += num;
	}

	void remove(int doc_id, int word_id, int topic_id) {
		add(doc_id, word_id, topic_id, -1);
	}

	double ran_uniform() {
		return rand() / (RAND_MAX + 1.0);
	}

	int ran_multinomial(double* prob, int k) {
		int i;
		for(i = 1; i < k; i++) {
			prob[i] += prob[i - 1];
    }
		double p = ran_uniform() * prob[k - 1];
		for(i = 0; i < k; i++) {
			if(prob[i] > p) break;
		}
		return i;
	}

	void single_iter() {
		for(int doc_id = 0; doc_id < M; doc_id++) {
			auto tmp_doc = docs[doc_id];
			int N = tmp_doc.size();
			for(int word_index = 0; word_index < N; word_index++) {
				int topic_id = z_index[doc_id][word_index];
				int word_id = tmp_doc[word_index];
				remove(doc_id, word_id, topic_id);
				for(int k = 0; k < K; k++) {
					k_prob[k] = (doc2topic[doc_id][k] + alpha) / (sum_doc2topic[doc_id] + Kalpha) *
								(topic2word[k][word_id] + beta) / (sum_topic2word[k] + Vbeta);
				}
				topic_id = ran_multinomial(k_prob, K);
				z_index[doc_id][word_index] = topic_id;
				add(doc_id, word_id, topic_id);
			}
		}
	}

	double likelihood() {
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

		for(int i = 0; i < K; i++) {
			for(int j = 0 ; j < V; j++) {
				phi[i][j] = (topic2word[i][j] + beta) / (sum_topic2word[i] + Vbeta);
      }
    }
	}
	
 private:
	std::string input, output;
	double alpha, beta, Kalpha, Vbeta;
	int K, max_iter, top;
	int M = 0, V = 0, T = 0;
	paralg* pt;
	std::vector<std::vector<int>> docs;
	std::unordered_map<std::string, int> word2id;
	std::unordered_map<int, std::string> id2word;
	double* k_prob;
	int** doc2topic;
	int** topic2word;
	int** z_index;
	int* sum_doc2topic;
	int* sum_topic2word;
	double** theta;
	double** phi;
}; // class LDA

} // namespace tool
} // namespace paracel

DEFINE_string(cfg_file, "", "config json file with absolute path.\n");
int main(int argc, char *argv[])
{
	paracel::main_env comm_main_env(argc, argv);
	paracel::Comm comm(MPI_COMM_WORLD);
	google::SetUsageMessage("[options]\n\t--cfg_file\n");
	google::ParseCommandLineFlags(&argc, &argv, true);
  
	paracel::json_parser pt(FLAGS_cfg_file);
	std::string input = pt.check_parse<std::string>("input");
	std::string output = pt.parse<std::string>("output");
	double alpha = pt.parse<double>("alpha");
	double beta = pt.parse<double>("beta");
	int k_topics = pt.parse<int>("k_topics");
	int iters = pt.parse<int>("iters");
	int top_words = pt.parse<int>("top_words");
	paracel::tool::LDAmodel solver(comm,
                                 input,
                                 output,
                                 alpha,
                                 beta,
                                 k_topics,
                                 iters,
                                 top_words);
	solver.train();
	solver.save();
	return 0;
}
