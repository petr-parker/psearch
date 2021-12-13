#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <dirent.h>
#include <vector>
#include <string>
#include <iostream>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/time.h>
#include <algorithm>
#include <signal.h>
#include <sys/times.h>
#include <pthread.h>
#include <fstream>
#include <sys/mman.h>
#include <mutex>

using namespace std;

#include "help.cc"
#include "KMP.cc"

class Walker {
public:
	DIR * dir;
	dirent * curr;
	string curr_dirname;
	vector<string> dirs_to_search;

	Walker(string dirname) {
		dir = NULL;
		curr = NULL;
		dirs_to_search.push_back(dirname);
	}
	bool new_dir() {
		if (!dirs_to_search.empty()) {
			curr_dirname = dirs_to_search.back();
			dirs_to_search.pop_back();
			dir = opendir(curr_dirname.c_str());
			curr = readdir(dir);
			return true;
		}
		return false;
	}
	void skip() {
        while (curr != NULL && curr->d_type == DT_DIR) {
            if (strcmp(".", curr->d_name) == 0 || strcmp("..", curr->d_name) == 0) { 
                curr = readdir(dir);
            } else {
				dirs_to_search.push_back(curr_dirname + "/" + curr->d_name);
				curr = readdir(dir);
			}
        }
	}
	string step() {
		skip();
		while (curr == NULL) {
			if (!new_dir()) { break; }
			skip();
		}

        if (curr == NULL) { return ""; }

		string ret = curr_dirname + "/" + curr->d_name;
		curr = readdir(dir);
		return ret;
	}
	string this_step() {
        if (!dirs_to_search.empty()) {
			curr_dirname = dirs_to_search.back();
			dirs_to_search.pop_back();
			dir = opendir(curr_dirname.c_str());
			curr = readdir(dir);
        }
		while (curr != NULL && curr->d_type == DT_DIR) {
			curr = readdir(dir);
		}
		if (curr == NULL) { return ""; }
		string ret = curr->d_name;
		curr = readdir(dir);
		return ret;
	}
};

struct args {
	vector<string> * files_to_search;
	string sample;
	bool * finish;
	mutex * mutex;
};

void * searcher(void *arg) {
	args * a = (args *) arg;
	string sample = a->sample;
	vector<string> * files_to_search = a->files_to_search;
	mutex * mutex = a->mutex;
	bool * finish = a->finish;
	
	int fd, n;
	KMP A(sample);
	string file = "";







	bool exit = false;
	while (true) {
		while (file == "" && !exit) {
			mutex->lock();
			if (!files_to_search->empty()) {
				file = files_to_search->back();
				files_to_search->pop_back();
			} else if (*finish) {
				exit = true;
			}
			mutex->unlock();
			if (file == "" && !exit) {
				usleep(100);
			}
			printf("exit:%d, file:%s\n", exit, file.c_str());
		}
		if (exit) { break; }

		
		n = lseek(fd, 0, SEEK_END);




		file = "";
	}

	


/*
	KMP A(sample);

	int fd, n;
	char * file_name;
	while (!finish) {
		n = lseek(fd, 0, SEEK_END);
		void * m = mmap(NULL, n, PROT_READ, MAP_SHARED, fd, 0);
		close(fd);
 		if (m == MAP_FAILED) {
    	    return nullptr;
    	}
    	char * all = (char *) m;

		int i = 0;
		int line_start = 0;
		int line_num = 0;
    	Vertex * current;
    	while (all[i] != 0) {
        	line_num++;
        	current = A.vertexes;
        	line_start = i;
        	for (int j = line_start; all[j] != 0 && all[j] != '\n'; j++) {
            	current = A.step(current, all[j]);
            	i++;
        	}

        	if (current->next == NULL) {
            	printf("Образец \"%s\" найден в файле \"%s\" в строке %d:\n", sample.c_str(), file_names[k].c_str(), line_num);
            	for (int j = line_start; all[j] != 0 && all[j] != '\n'; j++) {
                	printf("%c", all[j]);
            	}
            	printf("\n\n");
        	}

        	if (all[i] != 0) i++;
    	}

    	munmap(m, n);
	}*/
	return nullptr;
}

int main(int argc, char ** argv) {
	vector<string> com = v_str(argc, argv);
	string dir_name = ".";
	string sample = "";
	int N = 1;
	bool this_dir = false;

	for (int i = 0; i < com.size(); i++) {
		if (com[i].size() > 1 && com[i][0] == '-') {
			if (com[i][1] == 't') { N = stoi(com[i].substr(2)); }
            if (com[i] == "-n") { this_dir = true; }
		} else if (sample == "") {
			sample = com[i];
		} else {
			dir_name = com[i];
		}
	}

	if (sample == "") {
		printf("Для поиска укажите образец.\n");
		return 0;
	}
	if (N == 0) {
		printf("Нельзя искать в 0 потоков.\n");
		return 0;
	}

	vector<pthread_t> threads(N);
	vector<args> argums(N);
	
	vector<mutex> mutexes(N);
	vector<vector<string> > files_to_search(N);
	bool finish = false;
	
	int thread_num = 0;
	Walker w(dir_name);

    for (int i = 0; i < N; i++) {
        argums[i].files_to_search = &files_to_search[i];
        argums[i].sample = sample;
        argums[i].finish = &finish;
		argums[i].mutex = &mutexes[i];
        pthread_create(&threads[i], nullptr, searcher, &argums[i]);
    }

	if (this_dir) {
        string file = w.this_step();
        while (file != "") {
            mutexes[thread_num].lock();
            files_to_search[thread_num].push_back(file);
            mutexes[thread_num].unlock();

			thread_num = (thread_num + 1) % N;
            file = w.this_step();
        }
	} else {
		string file = w.step();
		while (file != "") {
			mutexes[thread_num].lock();
			files_to_search[thread_num].push_back(file);
			mutexes[thread_num].unlock();

			thread_num = (thread_num + 1) % N;
			file = w.step();
		}
	}

	for (int i = 0; i < N; i++) { mutexes[i].lock(); }
	finish = true;
    for (int i = 0; i < N; i++) { mutexes[i].unlock(); }   

	for (int i = 0; i < N; i++) {
        pthread_join(threads[i], nullptr);
    }


/*
	for (int i = 0; i < size; i++) {
		mutexes[thread_num].lock();
		argums[i].files_to_search.push_back();
		argums[i].sample = sample;
		mutexes[thread_num].unlock();
	}

	for (int i = 0; i < N; i++) {
        argums[i].file_names = file_names[i];
		argums[i].fds = fds[i];
        argums[i].sample = sample;
        pthread_create(&threads[i], nullptr, searcher, &argums[thread_num]);
	}

	for (int i = 0; i < N; i++) {
		pthread_join(threads[i], nullptr);
	}*/
}

