#include "graph.h"

#include "node.h"
#include "label.h"
#include "simulation.h"
#include "edge.h"

#include <sstream>

using namespace std;

void Graph::push() {
	_P.push_back(_groups.size());
	_query[_groups.size()] = map<QueryType, unsigned int>();

	_groups.push_back(set<Edge *>());
	_compats.push_back(vector<unsigned int>());
}

void Graph::push(unsigned int i) {
	_P.push_back(i);
}

void Graph::pop() {
	// storing current top as last
	_left = _P.back();
	_P.pop_back();
}

set<Edge *> & Graph::top() {
	return _groups[_P.back()];
}

void Graph::close() {
	set<Edge *> s = top();

	if (_query[_P.back()].count(GROUP)) {
		unsigned int ref = _query[_P.back()][GROUP];
		push();
		_query[_P.back()][GROUP] = ref;
	} else {
		push(); // new group for top()-elements
		for (Edge *n : s)
			top().insert(n);
	}
	pop();
	Group();
	top().clear();
}

// void Graph::Join(Simulation &sim, Graph &g) {
// 	push();
// 	for (unsigned int i = 0; i < sim.rows(); ++i) {
// 		stringstream inode;
// 		inode << "<sim_" << i << ">";
// 		// for (unsigned int n = sim.init(i); n < sim.max(); n = sim.next(i)) {
// 			addTriple(getNode(i).getName(), "<simulates>", inode.str());
// 		// }
// 	}
// 	if (!_query[0].count(GROUP)) {
// 		push(0);
// 		close();
// 		pop();
// 	}
// 	_query[_P.back()][JOIN] = _query[0][GROUP];
// 	_query[0][GROUP] = _P.back();
// 	pop();

// }

bool Graph::inGroup(Edge &e, unsigned int g) {
	return _groups[g].count(&e);
}

bool Graph::inGroup(const string &sub, const string &pred, const string &obj, unsigned int g) {
	for (Edge *e : _groups[g]) {
		if (e->source().getName()==sub
			&& e->target().getName()==obj
			&& e->label().str()==pred)
			return true;
	}
	return false;
}


bool Graph::nodeInGroup(Node &n, unsigned int g) {
	for (Edge *e : _groups[g])
		if (e->isNode(n))
			return true;

	return false;
}

bool Graph::nodeInGroup(unsigned int n, unsigned int g) {
	return nodeInGroup(getNode(n), g);
}

vector<unsigned int> & Graph::compatGroups(unsigned int g) {
	return _compats[g];
}

vector<unsigned int> Graph::mandatorySubs(unsigned int g, bool flag) {
	vector<unsigned int> result, r;

	// if (!flag) {
		result.push_back(g);
		// cout << g << " || " << printGroup(g) << " || " << endl;
		// cout << printQuery(g) << endl;
	// }
	for (auto &x : _query[g]) {
		switch (x.first) {
			case JOIN: {
				if (flag)
					continue;
				// cout << g << " !!! " << x.second << endl;
				r = mandatorySubs(x.second);
				break;
			}
			case GROUP: {
				r = mandatorySubs(x.second);
				break;
			}
			case UNION: // not yet implemented!!
			case OPTIONAL:
			default:
				break;
		}
		for (unsigned int i = 0; i < r.size(); ++i)
			result.push_back(r[i]);
		r.clear();
	}

	return result;
}

void Graph::subGrouping(unsigned int g) {
	vector<unsigned int> r, rr, t, tt;
	for (unsigned int g = 0; g < _groups.size(); ++g) {
		for (auto &x : _query[g]) {
			switch (x.first) {
				case JOIN: {
					// cout << g << ": join found" << printQuery(g) << endl;
					// cout << "!!!" << endl;
					rr = mandatorySubs(g, true);
					// cout << "???" << endl;
					r = subGroups(x.second, true);
					// cout << "!!!" << endl;
					tt = mandatorySubs(x.second, true);
					// cout << "???" << endl;
					t = subGroups(g, true);
					// break;
					break;
				}
				case OPTIONAL: {
					rr = mandatorySubs(g, true);
					r = subGroups(x.second, true);
					break;
				}
				case GROUP:
				case UNION: // not yet implemented!!
				default:
					break;
			}
			for (unsigned int i = 0; i < rr.size(); ++i) {
				for (unsigned int j = 0; j < r.size(); ++j) {
					// cout << rr[i] << " <- " << r[j] << endl;
					_compats[rr[i]].push_back(r[j]);
				}
			}
			rr.clear(); r.clear();
			for (unsigned int i = 0; i < tt.size(); ++i) {
				for (unsigned int j = 0; j < t.size(); ++j) {
					// cout << tt[i] << " <- " << t[j] << endl;
					_compats[tt[i]].push_back(t[j]);
				}
			}
			tt.clear(); t.clear();
		}
	}
}

vector<unsigned int> Graph::subGroups(unsigned int g, bool flag) {
	vector<unsigned int> result, r;
		result.push_back(g);
		// cout << g << " || " << printGroup(g) << " ||" << endl;
	for (auto &n : _query[g]) {
		if (flag && n.first != GROUP)
			continue;
		r = subGroups(n.second);
		for (unsigned int i = 0; i < r.size(); ++i)
			result.push_back(r[i]);
	}

	return result;
}