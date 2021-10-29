struct Vertex {
	Vertex * exc;
	Vertex * next;
	char sigma;
};

class KMP {
public:
	string sample;
	Vertex * vertexes;
	KMP(string sample) {
		this->sample = sample;
		int N = sample.size();
		vertexes = new Vertex[N + 1];
	
		vertexes[0].exc = NULL;
		vertexes[0].next = vertexes + 1; 
		vertexes[0].sigma = sample[0];


		for	(int i = 1; i < N; i++) {
			vertexes[i].exc = NULL;
			vertexes[i].next = vertexes + i + 1;
			vertexes[i].sigma = sample[i];
		}		

		vertexes[N].exc = NULL;
		vertexes[N].next = NULL; 
		vertexes[N].sigma = 0;

		Vertex * v;
		for (int i = 1; i < N + 1; i++) {
			v = vertexes + i - 1;
    		while (v->exc != NULL) {
				if (v->exc->sigma == sample[i - 1]) {
					vertexes[i].exc = v->exc->next;
					break;
				}
				v = v->exc;
			}
			if (v->exc == NULL) {
				vertexes[i].exc = vertexes;
			}
		}       
	}
	int find(string text) {
		Vertex * current = vertexes;
		for (int i = 0; i < text.size(); i++) {
			if (current->next == NULL) {
				break;
			}
			current = step(current, text[i]);
		}
		if (current->next == NULL) {
			return 1;
		} else {
			return 0;
		}
	}
	Vertex * steps(Vertex * current, string word) {
		Vertex * v = current;
		for (int i = 0; i < word.size(); i++) {
			v = step(v, word[i]);
		}
		return v;
	}
	Vertex * step(Vertex * current, char sigma) {
		if (current->next == NULL) {
			return current;
		}
		Vertex * v = current;
		while (v->sigma != sigma && v->exc != NULL) {
			v = v->exc;
		}
		if (v->exc == NULL) {
			return v->sigma == sigma ? v->next : v;
		} 
		return v->next;
	}

	~KMP() {
		delete[] vertexes;
	}
};

