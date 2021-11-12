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
	int fd;
};

void * searcher(void *arg) {
	args * a = (args *) arg;
	string sample = a->sample;
	string file_name = a->file_name;

	KMP A(sample);

	int fd = a->fd; // open(file_name.c_str(), O_RDWR | O_CREAT, 0666);
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

class Queue {
public:
	int N;
	long long int * A;
	Queue(int N) {
		A = new long long int[N];
		this->N = N;
		for (int i = 0; i < N; i++) {
			A[i] = 0;
		}
	}
    ~Queue() {
		delete[] A;
	}
	void add(int n, long long int len) {
		A[n] = A[n] + len;
	}
	long long min () {
		long long int ret = A[0];
		for (int i = 0; i < N; i++) {
			if (ret > A[i]) {
				ret = A[i];
			}
		}
		return ret;
	}
	int i_min() {
		long long m = min();
		for (int i = 0; i < N; i++) {
			if (A[i] == m) return i;
		}
		return 0;
	}
	void reset() {
		long long m = min();
		for (int i = 0; i < N; i++) {
			A[i] = A[i] - m;
		}
	}
	int step(int len) {
		int n = i_min();
		add(n, len);
		reset();
		return n;
	}
};

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

	vector<string> all_files;
	if (this_dir) {
		all_files = this_walk(dir_name);
	} else {
		all_files = walk(dir_name);
	}

	vector<pthread_t> threads(N);
	vector<args> argums(N);
	
	long int n;
	long long int len;
	int size = all_files.size();

	Queue Q = Queue(N);

	int thread_num;
	int fd;
	
	int min_size_N = (size < N ? size : N);
	for (n = 0; n < min_size_N; n++) {
		fd = open(all_files[n].c_str(), O_RDWR | O_CREAT, 0666);
		len = lseek(fd, 0, SEEK_END);
		thread_num = Q.step(len);
       
		argums[thread_num].file_name = all_files[n];
		argums[thread_num].sample = sample;
		argums[thread_num].fd = fd; 
		pthread_create(&threads[n], nullptr, searcher, &argums[thread_num]);
	}

	while (n < size) {
		fd = open(all_files[n].c_str(), O_RDWR | O_CREAT, 0666);
		len = lseek(fd, 0, SEEK_END);
		thread_num = Q.step(len); // выбираем, какому потоку дать файл на обработку
		
		pthread_join(threads[thread_num], nullptr);

     	argums[thread_num].file_name = all_files[n];
    	argums[thread_num].sample = sample;
		argums[thread_num].fd = fd;
	    pthread_create(&threads[thread_num], nullptr, searcher, &argums[thread_num]);

		n++;
	}

    for (int i = 0; i < min_size_N; i++) {
        pthread_join(threads[i], nullptr);
    }
}

