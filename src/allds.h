#ifndef ALLDS_H
#define ALLDS_H

#include <iostream>
#include <set>
#include <string>
#include <vector>
#include <map>

using namespace std;

#include "bm.h"
#include "simulation.h"

class Graph;

class AllDS : public QGSimulation {

public:
	AllDS(Graph &DB);
	~AllDS();

	// evaluation
	virtual unsigned evaluate(ostream &os = devnull);

	// define output file name
	virtual void setOutput(const string &filename);

	virtual void statistics(const std::string &filename);
	virtual void statistics(std::ostream &os);

	virtual void csv(const std::string &filename, const char delim = ',');
	virtual void csv(std::ostream &os, const char delim = ',');

private:
	// stack of simulations
	vector<map<string, bm::bvector<> > > _stack;

	unsigned evaluateStack(ostream &os);
	void initialize(map<string, bm::bvector<> > &sim);

	map<string, bm::bvector<> > _root;
	bool minusminus(map<string, bm::bvector<> > &sim);

};

#endif /* ALLDS_H */
