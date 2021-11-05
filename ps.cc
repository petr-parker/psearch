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

using namespace std;

#include "help.cc"
#include "KMP.cc"

struct args {
	string file_name;
	string sample;
};

void * searcher(void *arg) {
	args * a = (args *) arg;
	string sample = a->sample;
	string file_name = a->file_name;

	KMP A(sample);

	int fd = open(file_name.c_str(), O_RDWR | O_CREAT, 0666);
	if (fd < 0) {
		//printf("Файл \"%s\" недоступен(.\n\n", file_name.c_str());
		return nullptr;
	}
	int n = lseek(fd, 0, SEEK_END);

	void * m = mmap(NULL, n, PROT_READ, MAP_SHARED, fd, 0);
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
			printf("Образец \"%s\" найден в файле \"%s\" в строке %d:\n", sample.c_str(), file_name.c_str(), line_num);
			for (int j = line_start; all[j] != 0 && all[j] != '\n'; j++) {
				printf("%c", all[j]);
			}
			printf("\n\n");
		}
		
		if (all[i] != 0) i++;
	}

	munmap(all, file_name.size());
	close(fd);
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

	vector<string> all_files;
	if (this_dir) {
		all_files = this_walk(dir_name);
	} else {
		all_files = walk(dir_name);
	}
	

	vector<pthread_t> threads(N);
	vector<args> argums(N);
	
	long int n = 0;
	int thread_num;
	int size = all_files.size();
	
	while (size - n >= N) {
		for (int i = 0; i < N; i++) {
			argums[i].file_name = all_files[n];
			argums[i].sample = sample;
			pthread_create(&threads[i], nullptr, searcher, &argums[i]);
			n++;
		}
		for (int i = 0; i < N; i++) {
        	pthread_join(threads[i], nullptr);
	    }
	}
	
	int N_rest = size - n;
	for (int i = 0; i < N_rest; i++) {
		argums[i].file_name = all_files[n];
		argums[i].sample = sample;
   		pthread_create(&threads[i], nullptr, searcher, &argums[i]);
        n++;
    }   
    for (int i = 0; i < N_rest; i++) {
        pthread_join(threads[i], nullptr);
    }
}

