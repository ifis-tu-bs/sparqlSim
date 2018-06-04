#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>

#include <string>

#include <vector>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>

#include <boost/dynamic_bitset.hpp>
#include "bm.h"

#include "simulation.h"
#include "graph.io.h"

using namespace std;
using namespace boost;

class Node;
class Edge;
class Triple;
class Label;
class MinGraph;

class Graph {

public:
	void resizeEdges();
	void compress();

	friend class MinGraph;

public:
	// standard constructor
	Graph();
	// destructor
	~Graph();

	// Constructor for balls g(w,dq) with center w and distance dq
	// Graph(Graph &g, const unsigned int w, const unsigned int dq);
	// Extract Match Graph according to sim
	Graph(Graph &g, Graph &q, Simulation &sim);
	// ExtractPerfectMatch constructor (connected components)
	Graph(Graph &g, const unsigned int w = 0);

	// edge-reduction wrt. the given alphabet of g: simulation initialized
	void reduce(Graph &g, Simulation &sim);
	pair<unsigned int, unsigned int> pruning(ofstream &os, Graph &q, Simulation &sim);
	pair<unsigned int, unsigned int> pruning(Graph &q, Simulation &sim);

	// Returns a graph containing sim-equivalence classes of nodes
	Graph & operator[](Simulation &sim);

	// Node Methods
	unsigned int addNode(const string &name);

private:
	unsigned int addNode(Node &n); // private node adder for ball constructor

public:
	Node &getNode(const unsigned int idx);
	Node &getNode(const string &name);

	unsigned int getIndex(Node &n);
	unsigned int getIndex(const string &name);

	Node &operator()(const int idx) { return getNode(idx); }
	Node &operator()(const string &name) { return getNode(name); }

	bool existsNode(const string &name) const;

	void clearNodes();

	// Label methods
	unsigned int addLabel(const string &label);

private:
	unsigned int addLabel(Label &l);

public:
	const bool isLabel(const string &l);
	const bool isLetter(const string &l);

	Label &getLabel(unsigned int idx);
	Label &getLabel(const string &name);

	void updateLabels();

	// Triple Methods
	void addTriple(const string &sub, const string &pre, const string &obj);
	// void addTriple(Edge *e);
	void addTriplesFromNTriple(const string &fname);

	// Edge & edge(const unsigned int &i) const;
	void printLabels();


	// query methods
	unsigned int size() const {
		return Vsize();// + Esize();
	}
	unsigned int Vsize() const { return _nodes.size(); }
	unsigned int Esize();
	bool empty() const { return _nodes.empty(); }

	unsigned int Ssize() const { return _Sigma.size(); }

	// computes the shortest path between two nodes v1 and v2
	int distance(const unsigned int v1, const unsigned int v2);
	// computes the maximal distance within the graph
	int diameter();

	// returns report string
	string report();
	void print(const int i);

private:
	unsigned int _redundancy = 0;

	vector<Node *> _nodes;
	// unordered_map<string, unsigned int> _rnodes;
	unordered_map<string, unsigned int> _rnodes;

	vector<Label *> _Sigma;
	// unordered_map<string, unsigned int> _rSigma;
	map<string, unsigned int> _rSigma;

	unsigned long _numTriples = 0;
	// vector<Edge *> _edges;
	// vector<set<Edge *> > _edgesMap; // label i -> edgeset

	vector<map<unsigned int, vector<unsigned int> > > _pree;
	vector<map<unsigned int, vector<unsigned int> > > _postt;

	// void edgesInit();

/*
 *  Friend Functions
 */
public:
	friend ostream & operator<<(ostream &os, Graph &g);

	friend void dualSim(Graph &q, Graph &ball, Simulation &sim);

/*
 *  Group Capabilities for Query Graphs
 */
public:
	// pushes another layer of groups
	void push();
	// pushes a certain layer of groups
	void push(unsigned int i);

	// removes group layer from stack
	void pop();

	// returns reference to top group
	set<Edge *> & top();

	// closes current group, i.e., shifts current group objects to new subgroup
	void close();

	// specifies operator type
	void Optional() {
		_query[_left][OPTIONAL] = _P.back();
	}
	void Join() {
		_query[_left][JOIN] = _P.back();
	}
	void Join(Simulation &sim, Graph &g);
	void Union() {
		_query[_left][UNION] = _P.back();
	}
	void Group() {
		_query[_P.back()][GROUP] = _left;
	}

	// prints the query using group g as root
	string printQuery(unsigned int g = 0);
	// prints a single group if nonempty
	string printGroup(unsigned int g);

	// print insert graph in n-triple format
	string printInsert(Simulation &sim, Graph &g);

	// returns the number of groups (size of container _groups)
	unsigned int groupSize() const { return _groups.size(); }
	unsigned int groupCount() const { return groupSize(); }

	// returns whether or not ith group is empty
	bool groupEmpty(unsigned int i) const { return _groups[i].empty(); }

	// checks whether a specific edge is contained in group _groups[g]
	bool inGroup(Edge &e, unsigned int g);
	bool inGroup(const string &sub, const string &pred, const string &obj, unsigned int g);

	// checks whether a node is contained in group _groups[g]
	bool nodeInGroup(Node &n, unsigned int g);
	bool nodeInGroup(unsigned int n, unsigned int g);

	vector<unsigned int> & compatGroups(unsigned int g);
	void subGrouping(unsigned int g = 0);
	vector<unsigned int> subGroups(unsigned int g, bool flag = false);
	vector<unsigned int> mandatorySubs(unsigned int g, bool flag = false);

	set<Edge *> &group(unsigned int g) { return _groups[g]; }

/*
 *  Grouping Fields
 */
private:
	// stores all groups
	vector<set<Edge *> > _groups;
	vector<vector<unsigned int> > _compats; // mapping of compatible groups
	// stack of group pointers
	vector<unsigned int> _P;
	// last group left by pop()
	unsigned int _left;

	// QueryType to navigate through main functionalities
	typedef enum { GROUP, OPTIONAL, JOIN, UNION } QueryType;
	// stores to each group its operand and type
	map<unsigned int, map<QueryType, unsigned int> > _query;

};

#endif /* GRAPH_H */
