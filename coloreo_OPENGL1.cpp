// OpenGL + C++ Graph Interaction mejorado con alerta de conflicto de color
#include <GL/glut.h>
#include <bits/stdc++.h>
using namespace std;

struct Graph {
	int size = 0;
	vector<vector<int>> adj;
	vector<int> colors;
	map<int, pair<int, int>> nodePositions;
	
	void tie(int a, int b) {
		if (find(adj[a].begin(), adj[a].end(), b) != adj[a].end()) return;
		adj[a].push_back(b);
		adj[b].push_back(a);
	}
	
	void untie(int a, int b) {
		auto remove_edge = [](vector<int>& v, int target) {
			for (int i = 0; i < v.size(); ++i) {
				if (v[i] == target) {
					swap(v[i], v.back());
					v.pop_back();
					break;
				}
			}
		};
		remove_edge(adj[a], b);
		remove_edge(adj[b], a);
	}
	
	void paint() {
		vector<int> visited(size, 0);
		vector<int> orderedIndexes(size);
		iota(orderedIndexes.begin(), orderedIndexes.end(), 0);
		sort(orderedIndexes.begin(), orderedIndexes.end(), [&](int a, int b) {
			return adj[a].size() > adj[b].size();
		});
		
		for (int i = 0; i < size; i++) {
			int current = orderedIndexes[i];
			if (visited[current]) continue;
			visited[current] = 1;
			vector<int> available(3, 1);
			for (int neighbor : adj[current]) {
				if (visited[neighbor]) {
					if (colors[neighbor] < 3)
						available[colors[neighbor]] = 0;
				}
			}
			bool assigned = false;
			for (int j = 0; j < 3; j++) {
				if (available[j]) {
					colors[current] = j;
					assigned = true;
					break;
				}
			}
			if (!assigned) {
				cout << "??  Nodo " << current << " no pudo ser coloreado. Conflicto de colores.\n";
				colors[current] = 3; // sin color válido
			}
		}
	}
	
	void addNode(int x, int y) {
		nodePositions[size] = {x, y};
		adj.push_back({});
		colors.push_back(3); // sin color inicial
		size++;
	}
	
	int getColor(int node) const {
		return node < (int)colors.size() ? colors[node] : 3;
	}
};

class OpenGLGraph {
private:
	static OpenGLGraph* instance;
	Graph* graph;
	int width, height;
	vector<pair<int, int>> clickBuffer;
	
	OpenGLGraph(Graph* g, int w, int h) : graph(g), width(w), height(h) {}
	
	static void displayCallback() { if (instance) instance->draw(); }
	static void mouseCallback(int button, int state, int x, int y) {
		if (instance && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
			instance->handleClick(x, y);
	}
	
	void draw() {
		glClear(GL_COLOR_BUFFER_BIT);
		
		// Dibuja las aristas
		for (int from = 0; from < graph->size; ++from) {
			for (int to : graph->adj[from]) {
				if (from < to) {
					pair<int, int> coord1 = graph->nodePositions[from];
					pair<int, int> coord2 = graph->nodePositions[to];
					glColor3f(1, 1, 1);
					glBegin(GL_LINES);
					glVertex2f(coord1.first, coord1.second);
					glVertex2f(coord2.first, coord2.second);
					glEnd();
				}
			}
		}
		
		// Dibuja los nodos
		for (int node = 0; node < graph->size; ++node) {
			pair<int, int> coord = graph->nodePositions[node];
			float x = coord.first, y = coord.second;
			int color = graph->getColor(node);
			switch (color) {
			case 0: glColor3f(1, 0, 0); break;
			case 1: glColor3f(0, 1, 0); break;
			case 2: glColor3f(0, 0, 1); break;
			default: glColor3f(1, 1, 0); break; // amarillo si hay error de color
			}
			glBegin(GL_POLYGON);
			for (int i = 0; i < 20; ++i) {
				float theta = 2 * 3.14159f * i / 20;
				glVertex2f(x + 10 * cos(theta), y + 10 * sin(theta));
			}
			glEnd();
		}
		
		glutSwapBuffers();
	}
	
	void handleClick(int x, int y) {
		if (graph->size < 6) {
			graph->addNode(x, y);
		} else {
			for (int i = 0; i < graph->size; ++i) {
				pair<int, int> pos = graph->nodePositions[i];
				float dx = x - pos.first;
				float dy = y - pos.second;
				if (sqrt(dx * dx + dy * dy) <= 10) {
					clickBuffer.push_back({i, graph->getColor(i)});
					if (clickBuffer.size() == 2) {
						int a = clickBuffer[0].first, b = clickBuffer[1].first;
						if (a != b) {
							auto& neighbors = graph->adj[a];
							if (find(neighbors.begin(), neighbors.end(), b) != neighbors.end()) {
								graph->untie(a, b);
								cout << "?? Desconectados " << a << " y " << b << endl;
							} else {
								graph->tie(a, b);
								cout << "?? Conectados " << a << " y " << b << endl;
							}
							graph->paint();
						}
						clickBuffer.clear();
					}
					break;
				}
			}
		}
		glutPostRedisplay();
	}
	
public:
		static void run(Graph* g, int w, int h, int argc, char** argv) {
			instance = new OpenGLGraph(g, w, h);
			glutInit(&argc, argv);
			glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
			glutInitWindowSize(w, h);
			glutCreateWindow("Click para crear, conectar y desconectar nodos");
			glClearColor(0, 0, 0, 1);
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluOrtho2D(0, w, h, 0);
			glutDisplayFunc(displayCallback);
			glutMouseFunc(mouseCallback);
			glutMainLoop();
		}
};

OpenGLGraph* OpenGLGraph::instance = nullptr;

int main(int argc, char** argv) {
	Graph g;
	OpenGLGraph::run(&g, 800, 600, argc, argv);
	return 0;
}
