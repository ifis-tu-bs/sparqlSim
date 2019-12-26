#ifndef SIMULATION_H
#define SIMULATION_H

#include <cassert>
#include <deque>
#include <map>
#include <unordered_set>
#include <vector>
#include <set>
#include <ostream>
#include <iostream>
#include <string>

#include <cstdlib>
#include <ctime>

#include "bm.h"
#include "smatrix.h"
#include "reporter.h"

class Graph;
class Variable;
class Label;
class Reporter;
class StrongSimulation;


typedef enum { WD, WWD, NWD } QueryClass;

class QGSimulation {

protected:
	Graph *_db; // the database the query is evaluated on

	unsigned int _max; // max-size of simulation vectors

	std::vector<bool> _isEmpty; // tracks whether simulation is empty per variable

	// standard stream for verbose output
	static std::ostream &devnull;

public:
	// Standard Constructor
	QGSimulation();
	// Constructor init database
	QGSimulation(Graph &DB);

	// Copy Constructor
	QGSimulation(QGSimulation &s);
	// QGSimulation(QGSimulation &s, bm::bvector<> &ball, bm::bvector<> &border);

	// Destructor
	~QGSimulation();

	// computes and prints simulation; and returnes number of results
	virtual unsigned evaluate(std::ostream &os = devnull);

	// outputs the results upon evaluation
	unsigned output();
	virtual unsigned output(const std::string &filename);
	virtual unsigned output(std::ostream &os);

	// sets the output stream
	virtual void setOutput(const string &filename);
	void setOutputStream(ostream &os);
	void closeOutputStream();

	// outputs statistics line
	virtual void statistics(const std::string &filename);
	virtual void statistics(std::ostream &os);

	virtual void csv(const std::string &filename, const char delim = ',');
	virtual void csv(std::ostream &os, const char delim = ',');

	// returns the query graph simulation (union of all groups)
	std::map<std::string, bm::bvector<> > simulation();
	void printHeader(std::ostream &os, std::map<std::string, bm::bvector<> > &sim);

protected:
	friend class Variable;
	// compute fixpoint: returns number of iterations
	//void prefixpoint();
	unsigned int fixpoint(const unsigned = 0, ostream &os = devnull);
	unsigned int fixpointWithProfile(const unsigned = 0);

	// resolves dependencies
	const bool resolveDependencies();

	// sets the i'th equation to unstable
	void setUnstable(const unsigned int i);

	// returns number of equations
	unsigned int size() const {
		return _sourceV.size();
	}

	void print();

	void profile();

	unsigned int max() const { return _max; }

	bool empty() const { return _empty; }

	std::string order() const {
		std::stringstream r;
		for (unsigned int i = 0; i < _order.size(); ++i)
			r << (i==0 ? "" : " ") << _order[i];
		return r.str();
	}

	void resetAll();

	bool isProjected(const std::string &var) {
		if (_projections.empty())
			return true;
		return _projections.count(var);
	}

	unsigned countTriples() const;
	unsigned triples() const { return countTriples(); }
	unsigned triplesBaseline() const;

	const bool schedulable(unsigned eq, const bool b = false);

	const bool scheduled(const int eq) {
		return _scheduled[eq];
	}

	const bool mastersScheduled() const {
		unsigned m = _mandatory.get_first();
		if (!m && !_mandatory[0]) {
			return true;
		}
		do {
			if (!_scheduled[m])
				return false;
		} while (m = _mandatory.get_next(m));
		return true;
	}

	void setOrder(std::vector<unsigned> order) {
		_order = order;
	}

	std::vector<unsigned> getOrder() {
		return _order;
	}

protected:

	std::string _query;

	std::set<Label *> _Labels;

	// target <= source x operand
	std::vector<Variable *> _sourceV;
	std::vector<Label *> _operand;
	std::vector<bool> _dirs; // stores forwards/backwards edge information
	std::vector<Variable *> _targetV;

	std::vector<bm::bvector<> > _tripleNeighbors;

	bm::bvector<> _mandatory;

	std::multimap<std::string, unsigned> _labels;
	
	std::vector<Variable *> _vars;
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
	bool _Stable = false;

	// statistics
	unsigned _triplecount = 0;
	bool _triplescounted = false;
	
	unsigned _triplesInQuery = 0;
	unsigned _optsInQuery = 0;
	unsigned _queryDepth = 0;

	QueryClass _class = WD;

	bool stable() const;
	bool stable2() const;
	bool mastersStable() const;

	void unstable(const unsigned int id);
	void unstable1(const unsigned int id);

public: // methods for constructing the query
	void push(const bool slavemode = false);
	void pop();

	void addTriple(const std::string &sub, const std::string &pred, const std::string &obj);
	bool isTriple(const unsigned int, const std::string &, const unsigned int);

	void slavemode() { _slavemode.back() = true; }
	void mastermode() { _slavemode.back() = false; }

	void setEmpty(const bool val) { _empty = val; }

	void toProject(const std::string &var) {
		_projections.insert(var);
	}

	void setQuery(const std::string &query) {
		_query = query;
	}

	void addTripleNeighbors(const unsigned i, const unsigned j);

protected:
	// associates variable names with existing variables
	typedef std::map<std::string, Variable *> VMap;
	typedef std::multimap<std::string, Variable *> VMMap;
	typedef std::pair<std::string, Variable *> Pair;

	std::vector<VMap> _master;
	std::vector<VMMap> _slave;
	// std::vector<VMMap> _shadow; // shadow slaves

	// bool _slavemode = false;
	std::vector<bool> _slavemode;

	Variable *addVariable(const std::string &name);
	Variable *addConstant(const std::string &name);

	friend std::ostream &operator<<(std::ostream &os, QGSimulation &s);

protected:
	Reporter _reporter; // object reporter for storing statistics

	void overtakeKarla(const std::string &filename);

protected:
	bool _fileout = false;
	string _filename;
	ostream *_out = NULL;
	bool _outputDone = false;

	inline ostream &out() { return (_out ? *_out : devnull); }

	friend void inflate(bm::bvector<> &ball, bm::bvector<> &border, Graph &g, const unsigned radius);
	friend void strongsimulation(const unsigned pid, bm::bvector<> &balls, const unsigned first, unsigned &prev, Graph &g, const unsigned radius, bm::bvector<> &result, unsigned &res, StrongSimulation &sim);
};

#endif /* SIMULATION_H */
