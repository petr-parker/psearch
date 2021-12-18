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

struct args {
	vector<string> * files_to_search;
	string sample;
	bool * finish;
	mutex * files_mut;
	mutex * console;
};

void * searcher(void *arg) {
	args * a = (args *) arg;
	string sample = a->sample;
	vector<string> * files_to_search = a->files_to_search;
	mutex * files_mut = a->files_mut;
	bool * finish = a->finish;
	mutex * console = a->console;

	int fd, n;
	int i, line_start, line_num;
	Vertex * current;

	KMP A(sample);
	string file = "";
	char * all;

	bool exit = false;
	while (true) {
		while (file == "" && !exit) {
			files_mut->lock();
			if (!files_to_search->empty()) {
				file = files_to_search->back();
				files_to_search->pop_back();
			} else if (*finish) {
				exit = true;
			}
			files_mut->unlock();
			if (file == "" && !exit) {
				usleep(100);
			}
		}
		if (exit) { break; }

		fd = open(file.c_str(), O_RDWR | O_CREAT, 0666);
		if (fd < 0) { file = ""; continue; }
		n = lseek(fd, 0, SEEK_END);
		void * m = mmap(NULL, n, PROT_READ, MAP_SHARED, fd, 0);
		close(fd);
		if (m == MAP_FAILED) { file = ""; continue; }
		all = (char *) m;

		i = 0;
		line_start = 0;
		line_num = 0;
		
		while (all[i] != 0) {
			line_num++;
			current = A.vertexes;
			line_start = i;
			for (int j = line_start; all[j] != 0 && all[j] != '\n'; j++) {
				current = A.step(current, all[j]);
				i++;
			}
			if (current->next == NULL) {
				console->lock();
				printf("Образец \"%s\" найден в файле \"%s\" в строке %d:\n", sample.c_str(), file.c_str(), line_num);
				for (int j = line_start; all[j] != 0 && all[j] != '\n'; j++) {
					printf("%c", all[j]);
				}
				printf("\n\n");
				console->unlock();
			}
			if (all[i] != 0) i++;
		}
		munmap(m, n);
		file = "";
	}
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
	
	vector<string> files_to_search;
	mutex files_mut;
	bool finish = false;
	
	int thread_num = 0;
	Walker w(dir_name);
	mutex console;

    for (int i = 0; i < N; i++) {
        argums[i].files_to_search = &files_to_search;
        argums[i].sample = sample;
        argums[i].finish = &finish;
		argums[i].files_mut = &files_mut;
		argums[i].console = &console;
        pthread_create(&threads[i], nullptr, searcher, &argums[i]);
    }

	if (this_dir) {
        string file = w.this_step();
        while (file != "") {
            files_mut.lock();
            files_to_search.push_back(file);
            files_mut.unlock();

            file = w.this_step();
        }
	} else {
		string file = w.step();
		while (file != "") {
			files_mut.lock();
			files_to_search.push_back(file);
			files_mut.unlock();

			file = w.step();
		}
	}

	files_mut.lock();
	finish = true;
	files_mut.unlock();

	for (int i = 0; i < N; i++) {
        pthread_join(threads[i], nullptr);
    }
}

