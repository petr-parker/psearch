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

