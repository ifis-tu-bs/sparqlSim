#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>

#include <string>

#include <vector>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <functional>

#include "smatrix.h"
#include "bm.h"

using namespace std;

class StrongSimulation;
class Node;
class Edge;
class Triple;
class Label;
class MinGraph;
class SMatrix;

class Graph {

public:
	void resizeEdges();
	void compress();

	friend class MinGraph;

public:
	// standard constructor
	Graph();
	// Graph Constructor with Hash Set Initializers
	Graph(unsigned, unsigned);
	// destructor
	~Graph();

	/// Adder and Getter

	// add node by name
	const unsigned int addNode(const string &name);
	// get node name by index
	const string getNodeName(const unsigned idx) const;
	// get node index by name
	// const unsigned int getIndex(const string &name) const;
	const unsigned int getNodeIndex(const string &name);

	// add label by name
	const unsigned int addLabel(const string &label);

	// get label by index
	Label &getLabel(const unsigned int idx);
	// get label by name
	Label &getLabel(const string &name);
	// create and return new label subject->object matrices
	Label &createLabel(const string &name, const bool subj = true);

	// add triple by sub, pred, obj strings
	void addTriple(const string &sub, const string &pre, const string &obj);

	// dummy node business
	const Node &dummy(const unsigned idx);
	const Node &dummy(const string &name);

	void computeBall(bm::bvector<> &ball, const unsigned dia, bm::bvector<> &border);
	void computeAllBalls(vector<bm::bvector<> > &balls, vector<bm::bvector<> > &borders, const unsigned radius, bm::bvector<> &which);

private:
	// handy node to return when no other is available
	static Node &_dummy;

public: /// Query and Info Methods ///

	const bool existsNode(const string &name);
	const bool isNode(const string &name);

	const bool isLabel(const string &l);
	const bool isLetter(const string &l);

	/// Size Info
	const unsigned int size() const;
	const unsigned int Vsize() const;
	const unsigned int Esize();
	const unsigned int Ssize() const;

	// computes the shortest path between two nodes v1 and v2
	int distance(const unsigned int v1, const unsigned int v2);
	// computes the maximal distance within the graph
	int diameter();

	//inflate Balls in StrongSim
	void increaseDiameter(bm::bvector<> &ball, bm::bvector<> &border, std::vector<std::set<SMatrix *> > &spheres, const unsigned rad);
	void increaseDiameter(bm::bvector<> &ball, bm::bvector<> &border, std::set<SMatrix *> &sphere, const unsigned rad);

public: /// Streaming and Output ///
	// returns report string
	void report(ostream &os, const bool full = false);

	// store this database to disk
	void store(const string &dir);
	// load database from disk (after stored)
	void load(const string &dir);

private:
	// hash function to be used in load mode
	hash<string> strHash;
	// configurations as stored for load option
	map<string,string> config;

	// set of nodes
	vector<Node *> _nodes;
	// reverse index on _nodes
	unordered_map<string, unsigned int> _rnodes;
	// stores hashes instead of strings as key
	map<size_t, unsigned> _Rnodes;

	// returns index of node "name" in _nodes
	const unsigned rnodes(const string &name);

	// node number: replaces the size in load mode
	unsigned _nodenum;

	unsigned _maxDegree = 0;
	unsigned _maxNode = 0;
	unsigned _maxLabel = 0;

	double _meanDegree = 0.0;

	void updateDegree(const unsigned nod, const unsigned deg, const unsigned lab, const unsigned k);

private:
	// set of labels
	vector<Label *> _Sigma;
	// reverse index on _Sigma
	unordered_map<string, unsigned int> _rSigma;
	// stores hashes instead of string predicates
	map<size_t, unsigned> _RSigma;

	// returns the index if label "name" in _Sigma
	const unsigned rSigma(const string &name);

	// number of triples read
	unsigned long _numTriples = 0;

	// vector<bm::bvector<> > NEIGHBORS;
	SMatrix NEIGHBORS;

	void addNeighbors(const unsigned n1, const unsigned n2);
	
public:
	// vector<bm::bvector<> > &neighbors();
	SMatrix &neighbors();

private:
	void loadRow(const unsigned label, const string &line);

	// vector<bm::bvector<> > &neighbors();

	// friend void inflate(bm::bvector<> &ball, bm::bvector<> &border, Graph &g, const unsigned radius);
	// friend void inflate0(const unsigned pid, const unsigned min, const unsigned max, Graph &g, vector<bm::bvector<> > &balls, vector<bm::bvector<> > &borders, const int d);

};

// void inflate(bm::bvector<> &ball, bm::bvector<> &border, Graph &g, const unsigned radius);
// void inflate0(const unsigned pid, const unsigned min, const unsigned max, Graph &g, vector<bm::bvector<> > &balls, vector<bm::bvector<> > &borders, const int d);

#endif /* GRAPH_H */
