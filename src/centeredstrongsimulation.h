#ifdef MULTITHREADING

#ifndef CENTEREDSTRONGSIMULATION_H
#define CENTEREDSTRONGSIMULATION_H

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
#include "strongsimulation.h"
//#include "strongsimulation.h"


class Graph;
class Variable;
class Label;

class CenteredStrongSimulation : public StrongSimulation {

public:
	// Standard Constructor
	CenteredStrongSimulation(Graph &DB);

	// Copy Constructors
	CenteredStrongSimulation(CenteredStrongSimulation &s);
	CenteredStrongSimulation(CenteredStrongSimulation &s, bm::bvector<> &ball, bm::bvector<> &border);

	// Destructor
	~CenteredStrongSimulation();

	// computes and prints simulation; and returnes number of results
	virtual unsigned evaluate(std::ostream &os = devnull);
	virtual void setOutput(const string &filename);

	virtual void statistics(const std::string &filename);
	virtual void statistics(std::ostream &os);

	virtual void csv(const std::string &filename, const char delim = ',');
	virtual void csv(std::ostream &os, const char delim = ',');

	// returns the query graph simulation (union of all groups)
	std::map<std::string, bm::bvector<> > &simulation();

	//CenteredStrongSim
	std::set<std::string> getPartner(std::string a);
	std::set<std::string> getPartner(std::string a, std::set<std::string> Old);
	std::set<SMatrix *> getEdges(std::string a, std::set<std::string> Old);
	std::vector< std::vector< std::set<SMatrix *>>> edgeList(std::set<std::string> center, unsigned int radius);
	// unsigned int diameter(std::vector<std::string> set);
	// unsigned int radius(std::vector<std::string> set);
	std::set<std::string> center(std::vector<std::string> set,unsigned int radius);
	bm::bvector<> simList();
	void centerList(bm::bvector<> &sum, const std::string &center);
	void centerList(bm::bvector<> &sum, const std::set<std::string> &center);
	std::vector<std::string> queryNodeSet();
	std::string output(std::string delimiter);
	std::string var();
	//unsigned int fixspointY();
	void setFullyUnstable();

	const int radius();
	void computeCenters(bm::bvector<> &centers, const unsigned radius);

	friend std::ostream &operator<<(std::ostream &os, QGSimulation &s);

private:

	std::map<std::string, bm::bvector<> > _sim;
	bool _computedSim = false;

};

#endif /* CENTEREDSTRONGSIMULATION_H */

#endif /* MULTITHREADING */
