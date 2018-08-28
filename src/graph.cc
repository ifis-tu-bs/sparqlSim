#include "graph.h"

#include <cstdint>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <fstream>
#include <queue>
#include <bitset>

using namespace std;

#include "edge.h"
#include "node.h"
#include "label.h"
#include "reporter.h"

extern Reporter Karla;
// external graph database

// Standard constructur & destructor
Graph::Graph() {
}

Graph::Graph(unsigned nodesize, unsigned labelsize) :
  Graph()
{
	_rnodes.rehash(nodesize);
	_rSigma.rehash(labelsize);
}

Graph::~Graph() {
	// nothing is deleted due to the reuse of the stored pointers
}

// This method creates the ball around w with distance dq
// Graph::Graph(Graph &g, const unsigned int w, const unsigned int dq) {
// 	// 1. Init bfs
// 	queue<unsigned int> q;
// 	q.push(w);

// 	unsigned int d = dq;
// 	// unordered_set<Edge *> edges;

// 	// 2. As long as distance not exceeded, add neighbors
// 	while (d-- > 0) {
// 		queue<unsigned int> newq;
// 		while (!q.empty()) {
// 			unsigned int current = q.front();
// 			addNode(g(current));
// 			string n = g(current).getName();
// 			q.pop();
// 			for (auto &e : g._pree[current]) {
// 				for (auto &nnid : e.second) {
// 					string nn = g.getNode(nnid).getName();
// 					if (!existsNode(nn)) {
// 						newq.push(g._rnodes[nn]);
// 					}
// 					else {
// 						addTriple(nn, _Sigma[e.first]->str(), n);
// 					}
// 				}
// 			}
// 			for (auto &e : g._postt[current]) {
// 				for (auto &nnid : e.second) {
// 					string nn = g.getNode(nnid).getName();
// 					if (!existsNode(nn)) {
// 						newq.push(g._rnodes[nn]);
// 					}
// 					else {
// 						addTriple(n, _Sigma[e.first]->str(), nn);
// 					}
// 				}
// 			}
// 		}
// 		q = newq;
// 	}
// }

// Extract Graph from sim
// Graph::Graph(Graph &g, Graph &q, Simulation &sim) {
// 	// add all sim-relevant nodes to the graph
// 	for (int u = 0; u < q.size(); ++u) {
// 		for (unsigned int v = 0; v < g.size(); ++v) {
// 			// check whether v simulates u
// 			if (!sim(u,v))
// 				continue;
// 			// Add node v if necessary
// 			string n = g(v).getName();
// 			addNode(n);
// 			// add relevant edges to temporary set
// 			for (auto &eu : q._pree[u]) {
// 				string l = q._Sigma[eu.first]->str(); // label of eu
// 				unsigned int lid = g._rSigma[l];
// 				for (unsigned int uu = 0; uu < eu.second.size(); ++uu)
// 					for (unsigned int vv = 0; vv < g._pree[v][lid].size(); ++vv) {
// 						string nn = g.getNode(g._pree[v][lid][vv]).getName();
// 						if (existsNode(nn)
// 							&& sim(eu.second[uu], g._pree[v][lid][vv])) {
// 							addTriple(nn, l, n);
// 						}
// 					}
// 			}
// 			for (auto &eu : q._postt[u]) {
// 				string l = q._Sigma[eu.first]->str(); // label of eu
// 				unsigned int lid = g._rSigma[l]; // id of label in g
// 				for (unsigned int uu = 0; uu < eu.second.size(); ++uu)
// 					for (unsigned int vv = 0; vv < g._postt[v][lid].size(); ++vv) {
// 						string nn = g.getNode(g._postt[v][lid][vv]).getName();
// 						if (existsNode(nn)
// 							&& sim(eu.second[uu], g._postt[v][lid][vv])) {
// 							addTriple(n, l, nn);
// 						}
// 					}
// 			}
// 		}
// 	}
// }

// pair<unsigned int, unsigned int> Graph::pruning(ofstream &os, Graph &q, Simulation &sim) {
// 	pair<unsigned int, unsigned int> result(0,0);

// 		// add all sim-relevant nodes to the graph
// 	for (int u = 0; u < q.size(); ++u) {
// 		for (unsigned int v = 0; v < size(); ++v) {
// 			// check whether v simulates u
// 			if (!sim(u,v))
// 				continue;
// 			// Add node v if necessary
// 			string n = _nodes[v]->getName();
// 			result.first++;
// 			// addNode(n);
// 			// add relevant edges to temporary set
// 			for (auto &eu : q._pree[u]) {
// 				string l = q._Sigma[eu.first]->str(); // label of eu
// 				unsigned int lid = _rSigma[l];
// 				for (unsigned int uu = 0; uu < eu.second.size(); ++uu)
// 					for (unsigned int vv = 0; vv < _pree[v][lid].size(); ++vv) {
// 						string nn = getNode(_pree[v][lid][vv]).getName();
// 						if (sim(eu.second[uu], _pree[v][lid][vv])) {
// 							os << nn << " " << l << " " << n << " ." << endl;
// 							result.second++;
// 						}
// 					}
// 			}
// 			// for (auto &eu : q._postt[u]) {
// 			// 	string l = q._Sigma[eu.first]->str(); // label of eu
// 			// 	unsigned int lid = g._rSigma[l]; // id of label in g
// 			// 	for (unsigned int uu = 0; uu < eu.second.size(); ++uu)
// 			// 		for (unsigned int vv = 0; vv < g._postt[v][lid].size(); ++vv) {
// 			// 			string nn = g.getNode(g._postt[v][lid][vv]).getName();
// 			// 			if (existsNode(nn)
// 			// 				&& sim(eu.second[uu], g._postt[v][lid][vv])) {
// 			// 				addTriple(n, l, nn);
// 			// 			}
// 			// 		}
// 			// }
// 		}
// 	}
// 	return result;
// }

// pair<unsigned int, unsigned int> Graph::pruning(Graph &q, Simulation &sim) {
// 	pair<unsigned int, unsigned int> result(0,0);

// 		// add all sim-relevant nodes to the graph
// 	for (int u = 0; u < q.size(); ++u) {
// 		for (unsigned int v = 0; v < size(); ++v) {
// 			// check whether v simulates u
// 			if (!sim(u,v))
// 				continue;
// 			// Add node v if necessary
// 			string n = _nodes[v]->getName();
// 			result.first++;
// 			// addNode(n);
// 			// add relevant edges to temporary set
// 			for (auto &eu : q._pree[u]) {
// 				string l = q._Sigma[eu.first]->str(); // label of eu
// 				unsigned int lid = _rSigma[l];
// 				for (unsigned int uu = 0; uu < eu.second.size(); ++uu)
// 					for (unsigned int vv = 0; vv < _pree[v][lid].size(); ++vv) {
// 						string nn = getNode(_pree[v][lid][vv]).getName();
// 						if (sim(eu.second[uu], _pree[v][lid][vv])) {
// 							// os << nn << " " << l << " " << n << " ." << endl;
// 							result.second++;
// 						}
// 					}
// 			}
// 			// for (auto &eu : q._postt[u]) {
// 			// 	string l = q._Sigma[eu.first]->str(); // label of eu
// 			// 	unsigned int lid = g._rSigma[l]; // id of label in g
// 			// 	for (unsigned int uu = 0; uu < eu.second.size(); ++uu)
// 			// 		for (unsigned int vv = 0; vv < g._postt[v][lid].size(); ++vv) {
// 			// 			string nn = g.getNode(g._postt[v][lid][vv]).getName();
// 			// 			if (existsNode(nn)
// 			// 				&& sim(eu.second[uu], g._postt[v][lid][vv])) {
// 			// 				addTriple(n, l, nn);
// 			// 			}
// 			// 		}
// 			// }
// 		}
// 	}
// 	return result;
// }

Graph::Graph(Graph &g, const unsigned int w) {
	cerr << "BFS Graph Constructor not implemented" << endl;
	// // 1. Init bfs
	// queue<unsigned int> q;
	// q.push(w);

	// unordered_set<Edge *> edges;

	// // 2. As long as distance not exceeded, add neighbors
	// while (!q.empty()) {
	// 	unsigned int current = q.front();
	// 	addNode(g(current));
	// 	q.pop();
	// 	for (Edge *e : g._inc[current]) {
	// 		edges.insert(e);
	// 		Node &neighbor = (*e)(g(current));
	// 		if (!existsNode(neighbor.getName()))
	// 			q.push(g._rnodes[neighbor.getName()]);
	// 	}
	// }

	// for (Edge *e : edges) {
	// 	addTriple(e->source().getName(), e->label(), e->target().getName());
	// }
}

// initializing sim to only relevant nodes
void Graph::reduce(Graph &g, Simulation &sim) {
	cerr << "Graph::reduce is currently not implemented" << endl;
	// Graph &sub = *(new Graph());
	// sim.initialize0(g.size(), size());
	// for (Label *l : g._Sigma) {
	// 	if (!isLabel(l->str()))
	// 		continue;
	// 	for (Edge *e : edgesByLabel(*l)) {
	// 		// sub.addNode(e->source().getName());
	// 		// sub.addNode(e->target().getName());
	// 		// sub.addLabel(e->label());

	// 		// sub.addTriple(e);
	// 		for (unsigned int u = 0; u < sim.size(); ++u) {
	// 			sim.setAll(getIndex(e->source()));
	// 			sim.setAll(getIndex(e->target()));
	// 		}
	// 	}
	// }
	// return sub;
}

// returns a minimal graph wrt. sim
// must be called with a self-simulation
Graph & Graph::operator[](Simulation &sim) {
	assert(sim.size()==size());
	Graph &min = *(new Graph());
	cerr << "Graph::min() not implemented" << endl;
	// unsigned int idx;
	// for (unsigned int i = 0; i < size(); ++i) {
	// 	if (min.existsNode(_nodes[i]->getName()))
	// 		continue;
	// 	idx = min.addNode(*_nodes[i]);
	// 	for (unsigned int j = sim.init(i); j < sim.max(); j = sim.next(i)) {
	// 		if (sim(j,i)) {
	// 			min._rnodes[_nodes[j]->getName()] = idx;
	// 		}
	// 	}
	// }
	// for (unsigned int i = 0; i < Vsize(); ++i) {
	// 	string sub = getNode(i).getName();
	// 	for (auto &p : _postt[i]) {
	// 		min.addTriple(sub, _Sigma[p.first]->str(), getNode(p.second).getName());
	// 	}
	// }

	return min;
}

unsigned int Graph::addNode(const string &name) {
	if (!existsNode(name)) {
		// cout << "add node '" << name << "'" << endl;
		_rnodes[name] = _nodes.size();
		_nodes.push_back(new Node(name));
		// edgesInit();
	}

	return _rnodes[name];
}

unsigned int Graph::addNode(Node &n) {
	if (!existsNode(n.getName())) {
		_rnodes[n.getName()] = _nodes.size();
		_nodes.push_back(&n);
		// edgesInit();
	}

	return _rnodes[n.getName()];
}

// void Graph::edgesInit() {
	// _pre.push_back(set<unsigned int>());
	// _pre.push_back(set<Edge *>());
	// _pree.push_back(map<unsigned int, vector<unsigned int> >());
	// _pree.push_back(multimap<unsigned int, unsigned int>());
	// _pre.push_back(unordered_set<Edge *>());
	// _post.push_back(set<unsigned int>());
	// _post.push_back(set<Edge *>());
	// _postt.push_back(map<unsigned int, vector<unsigned int> >());
	// _postt.push_back(multimap<unsigned int, unsigned int>());
	// _post.push_back(unordered_set<Edge *>());
	// assert(_pre.size() == _post.size());
// }

Node &Graph::getNode(const unsigned int idx) {
	assert(idx < size());
	return *_nodes[idx];
}

Node &Graph::getNode(const string &name) {
	assert(existsNode(name));
	return *_nodes[_rnodes[name]];
}

unsigned int Graph::getIndex(Node &n) {
	return _rnodes[n.getName()];
}

unsigned int Graph::getIndex(const string &name) {
	return _rnodes[name];
}

bool Graph::existsNode(const string &name) const {
	return _rnodes.find(name) != _rnodes.end();
}

//// Label Methods

unsigned int Graph::addLabel(const string &label) {
	if (!isLabel(label)) {
		// cout << "add label '" << label << "'" << endl;
		Label *l = new Label(label);
		_rSigma[label] = _Sigma.size();
		_Sigma.push_back(l);
		// _edgesMap.push_back(set<Edge *>());
		// _edgesMap.push_back(unordered_set<Edge *>());
	}
	return _rSigma[label];
}

unsigned int Graph::addLabel(Label &l) {
	if (!isLabel(l.str())) {
		_rSigma[l.str()] = _Sigma.size();
		_Sigma.push_back(&l);
		// _edgesMap.push_back(set<Edge *>());
		// _edgesMap.push_back(unordered_set<Edge *>());
	}
	return _rSigma[l.str()];
}

const bool Graph::isLabel(const string &l) {
	//for (auto &e: _rSigma) cout << e.first << endl;
	//cout << "checking " << l << endl;
	return _rSigma.find(l) != _rSigma.end();
}

const bool Graph::isLetter(const string &l) {
//	for (auto &e: _rSigma) cout << e.first << endl;
  //      cout << "checking " << l << endl;
	return isLabel(l);
}

Label &Graph::getLabel(unsigned int idx) {
	return *_Sigma[idx];
}

Label &Graph::getLabel(const string &name) {
	return getLabel(_rSigma[name]);
}

void Graph::updateLabels() {
	unsigned int n = size();
	for (unsigned int i = 0; i < _Sigma.size(); ++i)
		_Sigma[i]->updateMatrices(n);
}

// set<Edge *> & Graph::edgesByLabel(const string &lstr) {
// 	return _edgesMap[_rSigma[lstr]];
// }

// set<Edge *> & Graph::edgesByLabel(const Label &l) {
// 	return edgesByLabel(l.str());
// }

//// TRIPLE METHODS ////

// void Graph::resizeEdges() {
// 	unsigned int N = size();
// 	for (unsigned int i = 0; i < _Sigma.size(); ++i) {
// 		_Sigma[i]->resize(N);
// 	}
// }

void Graph::compress() {
	for (unsigned int i = 0; i < _Sigma.size(); ++i) {
		_Sigma[i]->compress(size());
	}
	// clearNodes();
}

void Graph::clearNodes() {
	for (unsigned int i = 0; i < _nodes.size(); ++i) {
		delete _nodes[i];
	}
}

void Graph::addTriple(const string &sub, const string &pre, const string &obj) {
	unsigned int s = addNode(sub);
	unsigned int p = addLabel(pre);
	unsigned int o = addNode(obj);

	if (!(*_Sigma[p])(s, o)) {
		_Sigma[p]->set(s, o);
		++_numTriples;
		// if (_numTriples % 10000000 == 0)
		// 	cout << _numTriples << " triples added" << endl;
	}

	if (!_groups.empty()) { // if a query is parsed
		Edge *triple = new Edge(_nodes[s], _Sigma[p], _nodes[o]);
		top().insert(triple);
	}
}

void Graph::printLabels() {
	for (unsigned int i = 0; i < size(); ++i) {
		cout << " " << *_nodes[i];
	}
	cout << endl;
	for (unsigned int i = 0; i < _Sigma.size(); ++i)
		cout << *_Sigma[i] << endl;
}

// void Graph::addTriple(Edge *e) {
// 	// assert(existsNode(e->source().getName()) && existsNode(e->target().getName()) && isLabel(e->label().str()));

// 	_post[_rnodes[e->source().getName()]].insert(e);
// 	_pre[_rnodes[e->target().getName()]].insert(e);

// 	// _edges.push_back(e);

// 	// _edgesMap[_rSigma[e->label().str()]].insert(e);
// }

// vector<unsigned int> Graph::getSources(const unsigned int q, const string &l) {
// 	assert(q < size());

// 	vector<unsigned int> result;
// 	// comute intersection
// 	set<Edge *> prel;
// 	set_intersection(_pre[q].begin(), _pre[q].end(), _edgesMap[_rSigma[l]].begin(), _edgesMap[_rSigma[l]].end(), std::inserter(prel, prel.begin()));

// 	// Node &nq = getNode(q);
// 	for (Edge *e : prel) {
// 		// if (e->label() == l)
// 			result.push_back(getIndex(e->target()));
// 	}

// 	return result;
// }

// vector<unsigned int> Graph::getTargets(const unsigned int q, const string &l) {
// 	assert(q < size());

// 	vector<unsigned int> result;
// 	// compute the intersection of post[q] and edgesMap[l]
// 	set<Edge *> postl;
// 	set_intersection(_post[q].begin(), _post[q].end(), _edgesMap[_rSigma[l]].begin(), _edgesMap[_rSigma[l]].end(), std::inserter(postl, postl.begin()));

// 	// Node &nq = getNode(q);
// 	for (Edge *e : postl) {
// 		// if (e->label() == l)
// 			result.push_back(getIndex(e->target()));
// 	}

// 	return result;
// }

//// Query Methods

int Graph::distance(const unsigned int v1, const unsigned int v2) {
	int result = 0;
	cout << "Function Graph::distance() NOT IMPLEMENTED" << endl;

	// dynamic_bitset<> seen(size());
	// seen.reset();
	// // bool seen[size()];
	// // for (int i = 0; i < size(); ++i)
	// // 	seen[i] = false;
	// seen.set(v1);

	// queue<unsigned int> q;
	// q.push(v1);

	// int last = q.back();

	// while (!q.empty()) {
	// 	int current = q.front();
	// 	if (current == v2)
	// 		return result;
	// 	q.pop();
	// 	for (unsigned int edx : _pre[current]) {
	// 		int n = getIndex(_edges[edx]->source());
	// 		if (!seen[n]) {
	// 			seen.set(n);
	// 			q.push(n);
	// 		}
	// 	}
	// 	for (unsigned int edx : _post[current]) {
	// 		int n = getIndex(_edges[edx]->target());
	// 		if (!seen[n]) {
	// 			seen.set(n);
	// 			q.push(n);
	// 		}
	// 	}
	// 	if (current == last) {
	// 		result++;
	// 		last = q.back();
	// 	}
	// }

	return -1;
}

int Graph::diameter() {
	int result = 0;
	int d;
	for (int i = 0; i < size(); ++i) {
		for (int j = i+1; j < size(); ++j) {
			// cout << "  distance between " << getNode(i).getName() << " and " << getNode(j).getName();
			d = distance(i,j);
			// cout << " = " << d << endl;
			if (d < 0)
				return d;
			result = (d > result ? d : result);
		}
	}

	return result;
}

unsigned int Graph::Esize() {
	// unsigned int size = 0;
	// // cout << *this << endl;
	// // cout << size << endl;
	// for (unsigned int i = 0; i < Vsize(); ++i)
	// 	for (auto &e : _pree[i])
	// 		size += e.second.size();
	// // cout << size << endl;
	// return size;
	return _numTriples;
}

void Graph::memfree() {
	for (Node *n : _nodes) {
		delete n;
	}
	_nodes.clear();
	_rnodes.clear();
	_rSigma.clear();
	for (Label *l : _Sigma) {
		l->nostr();
	}
}

string Graph::sizeOf() {
	uint64_t size;
	stringstream s;

	for (Node *n : _nodes) {
		size += n->sizeOf();
	}
	s << "============================" << endl;
	s << "  size of node set:         " << size << endl;
	size += sizeof(Node *) * _nodes.size();
	s << "  size of pointers:         " << sizeof(Node *) * _nodes.size() << endl;

	uint64_t rnset = sizeof(unordered_map<string,unsigned>)
						+ _rnodes.size() * sizeof(unsigned);
	// for (auto &e : _rnodes) {
	// 	rnset +=  sizeof(unsigned);
	// }
	s << "  size of reverse node set: " << rnset << endl;
	size += rnset;

	uint64_t lsize = 0;
	unsigned csize = 0;
	s << "----------------------------" << endl;
	for (Label *l : _Sigma) {
		s << l->sizeOf(csize) << endl;
		lsize += csize;
	}
	s << "----------------------------" << endl;
	size += lsize;
	s << "  size of \\Sigma:          " << lsize << endl;
	size += sizeof(Label *) * _Sigma.size();
	s << "  size of pointers:         " << sizeof(Label *) * _Sigma.size();
	rnset = sizeof(map<string,unsigned>)
			+ _rSigma.size() * sizeof(unsigned);
	// for (auto &e : _rSigma) {
	// 	rnset += e.first.size() * sizeof(char) + sizeof(string)/2 + sizeof(unsigned);
	// }
	s << "  size of reverse \\Sigma:  " << rnset << endl;
	size += rnset;
	s << "  overall size:             " << size << endl;
	s << "============================" << endl;

	return s.str();
}
