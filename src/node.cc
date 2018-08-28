#include "node.h"

#include <unordered_set>
#include <queue>
#include <string.h>

Node::Node()
{

}

Node::Node(const string &name) :
	_name(name)
{
}

Node::~Node() {
	_name = "";
	_name.clear();
}

const string & Node::getName() const {
	return _name;
}
