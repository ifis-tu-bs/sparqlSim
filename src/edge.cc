#include <iostream>
using namespace std;

#include "edge.h"

#include "graph.h"
#include "node.h"
#include "label.h"

bool Edge::_queryMode = false;

Edge::Edge(Graph &g, const string &subj, const string &pred, const string &obj) :
	_source(&g(g.addNode(subj))),
	_label(new Label(pred)),
	_target(&g(g.addNode(obj)))
{
}

Edge::Edge(Node *subj, Label *pred, Node *obj) :
	_source(subj),
	_label(pred),
	_target(obj)
{
}

Node & Edge::source() const {
	return *_source;
}

Node & Edge::target() const {
	return *_target;
}

Label & Edge::label() const {
	return *_label;
}

Node & Edge::operator()(Node &other) const {
	if (_source == &other)
		return target();
	else
		return source();
}

const bool Edge::isLabel(const string &l) const {
	return (*_label) == l;
}


ostream & operator<<(ostream &os, const Edge &e) {
	os 	<< (e._queryMode ? "?" : "")
	    << e.source().getName() << " "
			<< e.label() << " "
			<< (e._queryMode ? "?" : "")
	    << e.target().getName() << " .";
	return os;
}

