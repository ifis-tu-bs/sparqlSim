#include "graph.h"

#include <cstdint>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <fstream>
#include <queue>
#include <bitset>
#include <string>

// multi-threading
#include <functional>
#include <thread>
#include <chrono>

using namespace std;

#include "node.h"
#include "label.h"
#include "smatrix.h"
#include "reporter.h"

#include "simulations.h"

extern Reporter Karla;

#include "cmdline.h"
extern gengetopt_args_info args_info;
// external graph database

/// STATIC VARIABLES


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

//////////////////////
/// Adder & Getter ///
//////////////////////

const unsigned int Graph::addNode(const string &name) {
	if (!existsNode(name)) {
		_rnodes[name] = _nodes.size();

		_nodes.push_back(new Node(name));
		// NEIGHBORS.push_back(bm::bvector<>());
	}

	return _rnodes[name];
}

const string Graph::getNodeName(const unsigned idx) const {
	if (_nodes.size())
		return _nodes[idx]->str();

	return to_string(idx);
}

const unsigned Graph::rnodes(const string &name) {
	if (_rnodes.size())
		return _rnodes[name];
	return _Rnodes[strHash(name)];
}

const unsigned int Graph::getNodeIndex(const string &name) {
	return rnodes(name);
}

const bool Graph::existsNode(const string &name) {
	return (_Rnodes.find(strHash(name)) != _Rnodes.end() || _rnodes.find(name) != _rnodes.end());
}

const bool Graph::isNode(const string &name) {
	return existsNode(name);
}

//// Label Methods

const unsigned int Graph::addLabel(const string &label) {
	if (!isLabel(label)) {
		Label *l = new Label(label);
		_rSigma[label] = _Sigma.size();
		_Sigma.push_back(l);
	}
	return _rSigma[label];
}

const bool Graph::isLabel(const string &l) {
	return (_RSigma.find(strHash(l)) != _RSigma.end() || _rSigma.find(l) != _rSigma.end());
}

const bool Graph::isLetter(const string &l) {
	if (l[0] == '?') {
		return true;
	}
	return isLabel(l);
}

Label &Graph::getLabel(unsigned int idx) {
	return *_Sigma[idx];
}

Label &Graph::getLabel(const string &name) {
	return getLabel(rSigma(name));
}

Label &Graph::createLabel(const string &name, const bool subj) {
	Label *r = new Label(name);
	r->a().loadMode(); r->aT().loadMode();
	bm::bvector<> row;
	for (unsigned i = 0; i < _Sigma.size(); ++i) {
		if (subj) {
			row = _Sigma[i]->a().rowBV(getNodeIndex(name));
		} else {
			row = _Sigma[i]->aT().rowBV(getNodeIndex(name));
		}
		unsigned p = row.get_first();
		if (p || row.test(0)) {
			do {
				r->set2(i, p);
			} while (p = row.get_next(p));
		}
	}

	r->makeFinal();

	return *r;
}

const unsigned Graph::rSigma(const string &name) {
	if (_rSigma.size())
		return _rSigma[name];
	return _RSigma[strHash(name)];
}

//// TRIPLE METHODS ////

void Graph::compress() {
	for (unsigned int i = 0; i < _Sigma.size(); ++i) {
		_Sigma[i]->compress(size());
	}
	// clearNodes();
}

void Graph::addTriple(const string &sub, const string &pre, const string &obj) {
	if (args_info.quota_arg > 0 && _numTriples >= args_info.quota_arg)
		return;

	unsigned int s = addNode(sub);
	unsigned int p = addLabel(pre);
	unsigned int o = addNode(obj);

	// setting neighborhoods
	// NEIGHBORS[s].set(o);
	// NEIGHBORS[o].set(s);
	addNeighbors(s,o);

	if (!(*_Sigma[p])(s, o)) {
		_Sigma[p]->set(s, o);
		++_numTriples;
	}
}

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
	cout << "Function Graph::diameter() NOT IMPLEMENTED" << endl;
	int result = 0;
	int d;
	for (int i = 0; i < size(); ++i) {
		for (int j = i+1; j < size(); ++j) {
			d = distance(i,j);
			if (d < 0)
				return d;
			result = (d > result ? d : result);
		}
	}

	return result;
}

/////////////////
/// Size Info ///
/////////////////

// returns size of the node set
const unsigned Graph::size() const {
	return Vsize();
}

// returns size of the node set
const unsigned Graph::Vsize() const {
	return ( _nodes.size() ? _nodes.size() : _nodenum ); 
}

// returns size of the edge set
const unsigned Graph::Esize() {
	if (!_numTriples) {
		for (Label *l : _Sigma) {
			_numTriples += l->countTriples();
		}
	}
	return _numTriples;
}

// returns the size of the label set
const unsigned Graph::Ssize() const { 
	return _Sigma.size(); 
}

//////////////////////////////////////////
///              The Dummy Node 	   ///
/// ---------------------------------- ///
/// This node is meant for output only ///
//////////////////////////////////////////

// the node
Node &Graph::_dummy = *(new Node());

// sets the name to be the index and returns the node
const Node &Graph::dummy(const unsigned idx) {
	Graph::_dummy.setName(to_string(idx));
	return Graph::_dummy;
}

// sets the name to be the index and returns the node
const Node &Graph::dummy(const string &name) {
	Graph::_dummy.setName(name);
	return Graph::_dummy;
}

//StrongSim 

void Graph::increaseDiameter(bm::bvector<> &ball, bm::bvector<> &border, std::vector<std::set<SMatrix *> > &spheres, const unsigned rad) {
	static bm::bvector<> borderX(size());
	
	// assert(rad == spheres.size());

	for (unsigned r = 0; r < rad; ++r) {
		borderX.reset();
		// bm::bvector<> w = cut;
		// bm::bvector<> wX = in;
		unsigned int s = border.get_first();
		if (!s && !border.test(0)) {
			return;
		}
		// // std::vector< std::set<SMatrix *>> *Edges = &allEdges[c];
		// cout << "before pointer" << endl;
		// std::set<SMatrix *> &sphere = allEdges[c][radius];
		// cout << "after pointer" << endl;
		
		do {
			for (auto &ite: spheres[r]) {
				// cout << (ite != NULL ? "not null" : "null") << endl;
				// cout.flush();
				ite->updateNeighbors(s, borderX, ball);
				// cout << "nothing wrong" << endl;
				// cout.flush();
			}
		} while (s=border.get_next(s));

		border.swap(borderX);
	}
}

void Graph::increaseDiameter(bm::bvector<> &ball, bm::bvector<> &border, std::set<SMatrix *> &sphere, const unsigned rad) {
	static bm::bvector<> borderX(size());
	
	// assert(rad == spheres.size());

	for (unsigned r = 0; r < rad; ++r) {
		borderX.reset();
		// bm::bvector<> w = cut;
		// bm::bvector<> wX = in;
		unsigned int s = border.get_first();
		if (!s && !border.test(0)) {
			return;
		}
		// // std::vector< std::set<SMatrix *>> *Edges = &allEdges[c];
		// cout << "before pointer" << endl;
		// std::set<SMatrix *> &sphere = allEdges[c][radius];
		// cout << "after pointer" << endl;
		
		do {
			for (auto &ite: sphere) {
				// cout << (ite != NULL ? "not null" : "null") << endl;
				// cout.flush();
				ite->updateNeighbors(s, borderX, ball);
				// cout << "nothing wrong" << endl;
				// cout.flush();
			}
		} while (s=border.get_next(s));

		border.swap(borderX);
	}
}

// bm::bvector<> Graph::increaseDiameter( bm::bvector<> in, bm::bvector<> cut) {
	
// 	bm::bvector<> w = cut;//leer wird nicht mehr in rowbv benutzt
// 	bm::bvector<> wX = in;
// 	unsigned int r = in.get_first();

// 	//cout << "IN = " << wX << ", r= "<< r << endl;

// 	//cout << "Old: " <<  wX << endl;
// 	do {
		
// 		for (auto &ite:  _Sigma){
// 			//cout << "ite: " << ite->str() << endl;
// 			ite->a().rowbv(w,wX,r);
// 		}
// 		//cout << "a = " << wX << endl;
// 		for (auto &ite: _Sigma){
// 			ite->aT().rowbv(w,wX,r);
// 		}
// 		//cout << "aT = " << wX << endl;
// 	} while ((r=in.get_next(r)) != 0);
	
// 	//cout << "New result: " << wX << endl;
// 	return wX;
// }

void Graph::computeBall(bm::bvector<> &ball, const unsigned dia, bm::bvector<> &border) {
	unsigned rad = 0;
	unsigned nd;
	bm::bvector<> borderX(size());
	// while (rad++ < dia) {

	// 	// cout << "ball: " << ball << endl << "border: " << border << endl;

	// 	borderX.reset(); // reset new border

	// 	nd = border.get_first();
	// 	if (!nd && !border.test(0)) {
	// 		// cout << "failed at 0" << endl;
	// 		return;
	// 	}

	// 	do {
	// 		for (Label *l : _Sigma) {
	// 			// cout << l->str() << endl;
	// 			l->a().updateNeighbors(nd, borderX);
	// 			l->aT().updateNeighbors(nd, borderX);
	// 		}
	// 	} while (nd = border.get_next(nd));
	// 	borderX -= ball;
	// 	ball |= borderX;
	// 	border.swap(borderX);
	// }
}

// void Graph::computeAllBalls(vector<bm::bvector<> > &balls, vector<bm::bvector<> > &borders, const unsigned radius, bm::bvector<> &which) {
// 	unsigned brow = which.get_first();
// 	if (brow || which.test(0)) {
// 		do {
// 			balls.push_back(bm::bvector<>(size()).set(brow));
// 			borders.push_back(bm::bvector<>(size()));
// 		} while (brow = which.get_next(brow));
// 	}


// 	unsigned threadsNum = (args_info.threads_arg < balls.size() ? args_info.threads_arg : balls.size());
// 	unsigned workload = balls.size() / threadsNum;

// 	thread threads[threadsNum];
// 	for (unsigned i = 0; i < threadsNum; ++i) {
// 		threads[i] = std::thread(inflate0, i, i*workload, (i+1)*workload, ref(*this), ref(balls), ref(borders), radius);
// 	}

// 	// cout << "computing " << balls.size() << " balls in " << threadsNum << " threads (r=" << radius << ")" << endl;

// 	threads[0].join();
// 	threads[0] = std::thread(inflate0, 0, threadsNum * workload, balls.size(), ref(*this), ref(balls), ref(borders), radius);
// 	for (unsigned i = 0; i < threadsNum; ++i) {
// 		threads[i].join();
// 	}
// }

SMatrix &Graph::neighbors() {
	return NEIGHBORS;
}

void Graph::addNeighbors(const unsigned n1, const unsigned n2) {
	// static unsigned short need4neighbor = 0; // need not evaluated, yet

	// if (!need4neighbor) {
	// 	need4neighbor = 1; // neighbors not needed
	// 	for (unsigned i = 0; i < args_info.eval_given; ++i) {
	// 		if (args_info.eval_arg[i] >= eval_arg_STRONG) {
	// 			need4neighbor = 2; // neighbors needed
	// 			break;
	// 		}
	// 	}
	// }

	// if (need4neighbor > 1) {
	// 	NEIGHBORS.set(n1, n2);
	// 	NEIGHBORS.set(n2, n1);
	// }
}


// void inflate(const unsigned pid, Graph &g, vector<bm::bvector<> > &balls, const int d, StrongSimulation &filter) {
// 	unsigned workload = balls.size() / args_info.threads_arg;

// 	// actual computation
// 	unsigned i0 = pid * workload;
// 	unsigned imax = (pid+1) * workload;
// 	inflate0(pid,
// 		pid * workload,
// 		(pid+1) * workload,
// 		g, balls, d, filter);
	

// 	if (!pid) {
// 		inflate0(pid,
// 			args_info.threads_arg * workload, 
// 			balls.size() % args_info.threads_arg,
// 			g, balls, d, filter);
// 	}
// }

// void inflate(bm::bvector<> &ball, bm::bvector<> &border, Graph &g, const unsigned radius) {
// 	bm::bvector<> border1;
// 	for (unsigned r = 0; r < radius; ++r) {
// 		border1.reset();
// 		unsigned ro = border.get_first();
// 		do {
// 			border1 |= g.NEIGHBORS[ro] - ball;
// 		} while (ro = border.get_next(ro));
// 		if (border1.none()) {
// 			break;
// 		}
// 		ball |= border1;
// 		border.swap(border1);
// 	}
// }

// void inflate0(const unsigned pid, const unsigned min, const unsigned max, Graph &g, vector<bm::bvector<> > &balls, vector<bm::bvector<> > &borders, const int d) {
// 	bm::bvector<> border(g.size()), border1(g.size());
// 	for (unsigned i = min; i < max; ++i) {
// 		border = balls[i];
// 		border1.reset();
// 		for (unsigned r = 0; r < d; ++r) {
// 			unsigned ro = border.get_first();
// 			do {
// 				border1 |= g.NEIGHBORS[ro] - balls[i];
// 			} while (ro = border.get_next(ro));
// 			if (border1.none()) {
// 				break;
// 			}
// 			balls[i] |= border1;
// 			border.swap(border1);
// 			border1.reset();
// 		}
// 		// cout << "here" << endl;
// 		borders[i].swap(border);
// 		// cout << "there" << endl;
// 	}
// }
