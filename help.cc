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

