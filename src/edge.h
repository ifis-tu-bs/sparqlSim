#include <string>
#include <iostream>

using namespace std;

class Graph;
class Node;
class Label;

class Edge {
public:
	Edge(Graph &g, const string &subj, const string &pred, const string &obj);
	Edge(Node *subj, Label *pred, Node *obj);
	~Edge() {}

	Node  & source() const;
	Node  & target() const;
	Label & label()  const;

	// if other is source, returns target, otherwise returns source
	Node & operator()(Node &other) const;

	const bool isSource(Node &n) const { return &n == _source; }
	const bool isTarget(Node &n) const { return &n == _target; }
	const bool isNode(Node &n) const { return isSource(n) || isTarget(n); }
	const bool isLabel(const string &l) const;

	friend ostream & operator<<(ostream &os, const Edge &e);

	static void queryMode() { _queryMode = true; }
	static void graphMode() { _queryMode = false; }

private:
	Node *_source;
	Node *_target;
	Label *_label;

	static bool _queryMode;

};

ostream & operator<<(ostream &os, const Edge &e);
