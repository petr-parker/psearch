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

using namespace std;

#include "help.cc"
#include "KMP.cc"

void walk_recursive(string const &dirname, vector<string> &ret) {
    DIR *dir = opendir(dirname.c_str());
    if (dir == nullptr) {
        perror(dirname.c_str());
        return;
    }
    for (dirent *de = readdir(dir); de != NULL; de = readdir(dir)) {
        if (strcmp(".", de->d_name) == 0 || strcmp("..", de->d_name) == 0) continue; // не берём . и ..
        if (de->d_type == DT_DIR) {
            walk_recursive(dirname + "/" + de->d_name, ret);
        } else {
			ret.push_back(dirname + "/" + de->d_name);
		}
    }
    closedir(dir);
}

vector<string> walk(string const &dirname) {
    vector<string> ret;
    walk_recursive(dirname, ret);
    return ret;
}

vector<string> this_walk(string const &dirname) {
	vector<string> ret;
    DIR *dir = opendir(dirname.c_str());
    if (dir == nullptr) {
        perror(dirname.c_str());
        return ret;
    }
    for (dirent *de = readdir(dir); de != NULL; de = readdir(dir)) {
        if (de->d_type != DT_DIR) {
        	ret.push_back(dirname + "/" + de->d_name);
		}
    }
    closedir(dir);
	return ret;
}

vector<string> v_str(int argc, char ** argv) {
	vector<string> ret;
	for (int i = 1; i < argc; i++) {
		ret.push_back(string(argv[i]));
	}
	return ret;
}


struct args {
	string file_name;
	string sample;
};

void * searcher(void *arg) {
	args * a = (args *) arg;
	string sample = a->sample;
	string file_name = a->file_name;

	KMP A(sample);
	
	string line;
    ifstream file(file_name); // файл из которого читаем (для линукс путь будет выглядеть по другому)
	int line_num = 0;

    while(getline(file, line)) {
		line_num++;
		if (A.find(line)) {
			printf("Образец %s найден в файле %s в строке %d:\n", sample.c_str(), file_name.c_str(), line_num);
			printf("%s\n\n", line.c_str());
		}
    }

    file.close();

	/*
	int fd = open(file_name.c_str(), O_RDWR | O_CREAT, 0666);

	char buf[100000] = {0};
	Vertex * current = A.vertexes; // указатель на первый
	while (true) {
		ssize_t rd = read(fd, buf, sizeof buf-1);
		if (rd <= 0) break;
		current = A.steps(current, string(buf));
	}

	if (current->next == NULL) {
		printf("sample found in file %s\n", file_name.c_str());
	}
	
	close(fd);*/

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

