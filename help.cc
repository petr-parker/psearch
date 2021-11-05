vector<string> split(string line, char splitter) {
	int i = 0;
	vector<string> ret;
	string s;
	while (i < line.size()) {
    	while (i < line.size() && line[i] != splitter) {
        	s = s + line[i];
			i++;
		}
    	if (s != "") {
			ret.push_back(s);
        	s = "";
    	}
		i++;
	}
	return ret;
}

string join(vector<string> v, string joiner) {
	string ret = v[0];
	for (int i = 1; i < v.size(); i++) {
		ret = ret + joiner + v[i];
	}
	return ret;
}


vector<string> input() {
	vector<string> ret;
	string s;
	int i = 0;
	char c = ' ';
	while (c != '\n') {
		scanf("%c", &c);
		while (c != ' ' && c != '\n') {
			s = s + c;
			scanf("%c", &c);
		}
		if (s != "") {
			ret.push_back(s);
			s = "";
		}
	}
	return ret;
}

vector<char *> v_c_str(vector<string> &v) {
	vector<char *> ret;
	for (int i = 0; i < v.size(); i++) {
		ret.push_back((char *)(v[i].c_str())); 
    }
	ret.push_back(nullptr);
	return ret;
}

vector<string> v_str(int argc, char ** argv) {
    vector<string> ret;
    for (int i = 1; i < argc; i++) {
        ret.push_back(string(argv[i]));
    }
    return ret;
}

void walk_recursive(string const &dirname, vector<string> &ret) {
    DIR *dir = opendir(dirname.c_str());
    if (dir == nullptr) {
        // perror(dirname.c_str());
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
        // perror(dirname.c_str());
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

