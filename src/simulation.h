#ifndef SIMULATION_H
#define SIMULATION_H

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

class Graph;
class Variable;
class Label;

typedef std::map<std::string, bm::bvector<> > Simulation;
// typedef std::vector<bm::bvector<> > PWSet; // Powerset Type
typedef std::vector<Simulation> GSimulation;

class QGSimulation {

private:
	Graph *_q;
	Graph *_db;

	GSimulation _gsim;
	Simulation _sim;
	bool _computedSim = false;

	unsigned int _max; // max-size of simulation vectors
	unsigned int max() const { return _max; }

	std::vector<bool> _isEmpty; // tracks whether simulation is empty

public:
	// Standard Constructor
	QGSimulation(Graph &DB);

	// Copy Constructor
	QGSimulation(QGSimulation &s, bm::bvector<> &ball, bm::bvector<> &border);

	// Destructor
	~QGSimulation();

	// compute fixpoint: returns number of iterations
	//void prefixpoint();
	unsigned int fixpoint(const unsigned = 0);
	unsigned int fixpointWithProfile(const unsigned = 0);

	// original algorithm as proposed by Ma et al. (2011/2014)
	unsigned int MaEtAl();

	// sets the i'th equation to unstable
	void setUnstable(const unsigned int i);

	// returns the query graph simulation (union of all groups)
	Simulation &simulation();

	// returns number of equations
	unsigned int size() const {
		return _sourceV.size();
	}

	void print();

	void profile();

	void setEmpty(const bool val) { _empty = val; }

	bool empty() const { return _empty; }

	std::string order() const {
		std::stringstream r;
		for (unsigned int i = 0; i < _order.size(); ++i)
			r << (i==0 ? "" : " ") << _order[i];
		return r.str();
	}

	void resetAll();

	void toProject(const std::string &var) {
		_projections.insert(var);
	}

	bool isProjected(const std::string &var) {
		if (_projections.empty())
			return true;
		return _projections.count(var);
	}

	unsigned countTriples();

	const bool scheduled(const int eq) {
		return _scheduled[eq];
	}

private:
	typedef std::vector<Variable *> VarList;
	typedef std::vector<Label *> OpList;
	typedef std::vector<bool> Directions;

	// target <= source x operand
	VarList _sourceV;
	OpList _operand;
	Directions _dirs; // stores forwards/backwards edge information
	VarList _targetV;

	std::multimap<std::string, unsigned> _labels;
	
	VarList _vars;
	std::set<std::string> _projections;

	bool _empty = false; // flag saying whether there is a solution

	std::vector<unsigned> _order;
	std::vector<bool> _scheduled;

	// shuffles the order vector
	void shuffle() {
		unsigned j, x;
		srand(time(NULL));
		for (unsigned i = _order.size()-1; i > 0; --i) {
			j = rand() % (i+1);
			x = _order[i];
			_order[i] = _order[j];
			_order[j] = x;
		}
	}

	std::vector<bool> _stable; // stable equations

	// statistics
	unsigned _triplecount = 1; //TODO: revert to 0
	bool _triplescounted = false;

	bool stable() const;

	void unstable(const unsigned int id);
	void unstable1(const unsigned int id);

public:
	void push(const bool slavemode = false);
	void pop();

	void addTriple(const std::string &sub, const std::string &pred, const std::string &obj);
	bool isTriple(const unsigned int, const std::string &, const unsigned int);

	void slavemode() { _slavemode.back() = true; }
	void mastermode() { _slavemode.back() = false; }

private:
	// associates variable names with existing variables
	typedef std::map<std::string, Variable *> VMap;
	typedef std::multimap<std::string, Variable *> VMMap;
	typedef std::pair<std::string, Variable *> Pair;

	std::vector<VMap> _master;
	std::vector<VMMap> _slave;

	// bool _slavemode = false;
	std::vector<bool> _slavemode;

	Variable *addVariable(const std::string &name);
	Variable *addConstant(const std::string &name);

	friend std::ostream &operator<<(std::ostream &os, QGSimulation &s);

};

#endif /* SIMULATION_H */
