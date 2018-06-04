#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>

#include "graph.io.h"

using namespace std;

// QueryType: mandatory, optional, union
typedef enum { MAND, OPT, OR } QueryType;

class Node {
public:
	Node(); // standard constructor
	Node(const string &name); // initialize name

	// Node(const Node &node, bool deep = true); // copy constructor
	~Node();

	// void setName(const string &name);
	const string & getName() const;

private:
	// char *_name;
	string _name;

public:
	friend ostream & operator<<(ostream &os, const Node &n);

};

