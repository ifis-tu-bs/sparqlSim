#ifndef STRONGSIMULATION_H
#define STRONGSIMULATION_H

#include <cassert>
#include <deque>
#include <map>
#include <unordered_set>
#include <vector>
#include <ostream>
#include <iostream>
#include <string>

#include <cstdlib>
#include <ctime>

#include "bm.h"
#include "smatrix.h"
#include "simulation.h"

class Graph;
class Variable;
class Label;
class QGSimulation;

class StrongSimulation : public QGSimulation {

public:
	// Standard Constructor
	StrongSimulation(Graph &DB);

	// Copy Constructors
	StrongSimulation(StrongSimulation &s);
	StrongSimulation(StrongSimulation &s, bm::bvector<> &ball, bm::bvector<> &border);

	// Destructor
	~StrongSimulation();

	// computes and prints simulation; and returnes number of results
	virtual unsigned evaluate(std::ostream &os = devnull);

	virtual void setOutput(const string &filename);

	virtual void statistics(const std::string &filename);
	virtual void statistics(std::ostream &os);

	virtual void csv(const std::string &filename, const char delim = ',');
	virtual void csv(std::ostream &os, const char delim = ',');

	/*// returns the query graph simulation (union of all groups)
	std::map<std::string, bm::bvector<> > &simulation();*/

	void set(StrongSimulation &s, bm::bvector<> &ball, bm::bvector<> &border);

	const unsigned computeAllStrong(vector<bm::bvector<> > &balls, vector<bm::bvector<> > &borders);
	const unsigned computeAllStrong(bm::bvector<> &balls, const unsigned radius);
	// friend void inflate(const unsigned tid);

	//StrongSim
	std::set<std::string> getPartner(std::string a);
	std::set<std::string> getPartner(std::string a, std::set<std::string> Old);
	std::set<SMatrix *> getEdges(std::string a, std::set<std::string> Old);
	std::vector< std::vector< std::set<SMatrix *>>> edgeList(std::set<std::string> center, unsigned int radius);
	unsigned int diameter(std::vector<std::string> set);
	const int diameter();
	unsigned int radius(std::vector<std::string> set);
	std::set<std::string> center(std::vector<std::string> set,unsigned int radius);
	bm::bvector<> simList();
	bm::bvector<> centerList(std::set<std::string> center);
	bm::bvector<> centerList(std::string centerNode);
	std::vector<std::string> queryNodeSet();
	std::string output(std::string delimiter);
	std::string var();
	unsigned int fixpointY();	
	void setFullyUnstable();

	friend std::ostream &operator<<(std::ostream &os, QGSimulation &s);
	
	// void operator=(StrongSimulation &ref);

protected:
	int _diameter = 0; /// -1: infinity ; 0: not computed ; >0: computed

	map<string, bm::bvector<> > _sim;
	bm::bvector<> _unionsim;

	friend void inflate(bm::bvector<> &ball, bm::bvector<> &border, Graph &g, const unsigned radius);
	// friend void inflate0(const unsigned pid, const unsigned min, const unsigned max, Graph &g, vector<bm::bvector<> > &balls, const int d, StrongSimulation &filter);

	friend const unsigned strongsimulation0(const unsigned tid, bm::bvector<> &balls, SMatrix &N, const unsigned radius, StrongSimulation &sim);
	// friend void strongsimulation(const unsigned pid, bm::bvector<> balls, const unsigned first, unsigned workload, vector<bm::bvector<> > N, const unsigned radius, unsigned &res, StrongSimulation *sim);
	// friend void strongsimulation0(const unsigned pid, const unsigned min, const unsigned max, vector<bm::bvector<> > &balls, vector<bm::bvector<> > &borders, StrongSimulation &sim, unsigned &res);

};

void strongsimulation(const unsigned tid, bm::bvector<> &balls, SMatrix &N, const unsigned radius, unsigned &res, StrongSimulation &sim);
const unsigned strongsimulation0(const unsigned tid, bm::bvector<> &balls, SMatrix &N, const unsigned radius, StrongSimulation &sim);
// void strongsimulation(const unsigned pid, bm::bvector<> balls, const unsigned first, unsigned workload, vector<bm::bvector<> > N, const unsigned radius, unsigned &res, StrongSimulation *sim);
// // void strongsimulation(const unsigned pid, bm::bvector<> balls, const unsigned first, const unsigned workload, const unsigned &rest, vector<bm::bvector<> > N, const unsigned radius, unsigned &res, StrongSimulation *sim);
// // void strongsimulation(const unsigned pid, bm::bvector<> &balls, const unsigned first, unsigned &prev, Graph &g, const unsigned radius, bm::bvector<> &result, unsigned &res, StrongSimulation &sim);
// // void strongsimulation0(const unsigned pid, const unsigned min, const unsigned max, vector<bm::bvector<> > &balls, vector<bm::bvector<> > &borders, StrongSimulation &sim, unsigned &res);
// void strongsimulation0(const unsigned pid, bm::bvector<> &balls, const unsigned first, unsigned workload, vector<bm::bvector<> > &N, const unsigned radius, unsigned &res, StrongSimulation *sim);

bm::bvector<> operator-(vector<unsigned> &, bm::bvector<> &);

#endif /* STRONGSIMULATION_H */