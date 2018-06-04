#ifndef VARIABLE_H
#define VARIABLE_H

#include "bm.h"

#include <map>
#include <string>
#include <vector>

#include <cmath>

using namespace std;

class QGSimulation;

class Variable {
public:
	// constructs new variable
	Variable(QGSimulation *sim, const string &varid, const unsigned int size);
	// constructs new variable with existing val
	Variable(QGSimulation *sim, const string &varid, const bm::bvector<> &val, bool c = false);
	~Variable();

	Variable(QGSimulation &sim, Variable &v, bm::bvector<> &ball);
	Variable(QGSimulation &sim, Variable &v, bm::bvector<> &ball, bm::bvector<> &border);

	string getId() const { return _varid; }

	bm::bvector<> &val(); // returns _val
	bm::bvector<> &getVal(); // recomputes _val and returns it
	bool updVal();

	bool operator()(unsigned i) { return _val.test(i); }

	bool join(Variable &); // recomputes _val
	bool join(const bm::bvector<> &); // 

	bool subsumedBy(bm::bvector<> &);

	void update(bm::bvector<> &); // updates my value accordingly
	void null(); // sets _val to null

	void propagate(); // propagates my changes to the slaves
	void fullPropagate(); // propagates my changes to the slaves

	void changed(Variable &); // sets master index to changed
	// bool changed() const { return _changedf; }
	// bool stable() const { return !_changedf; }

	void addSlave(Variable &);
	void addMaster(Variable &);

	vector<Variable *> &masters() { return _masters; }
	vector<Variable *> &slaves() { return _slaves; }

	void operator<=(Variable &);

	void addEquation(const unsigned int idx);
	void addCoequation(const unsigned int idx);
	vector<unsigned> &equations();

	bm::bvector<> unify();

	bool constant() const { return _const; }

	bool isEmpty(const bool deep = false) {
		if (!deep)
			return _val.none();
		return unify().none();
	}

	void setMandatory(const bool val) {
		_mandatory = val;
	}

	bool isMandatory() const {
		return _mandatory;
	}

	// computing the degree of the variable
	double degree() const;
	double sdegree() const;

	// invert values, e.g., i(degree()) = 1 / degree();
	static double i(const double val) {
		return 1.0 / val;
	}

	static double l(const double val) {
		return log2(val);
	}

	static double p(const double val) {
		return pow(2.0, val);
	}

	double count() const {
		return (double) _val.count();
	}

	const unsigned size() const {
		return _val.count();
	}

	const bool isSchedulable();
	const bool isScheduled();

private:
	bm::bvector<> unifyMasters();
	bm::bvector<> unifySlaves();

private:
	// variable name in query
	string _varid;

	// a flag showing whether or not mandatory variable
	bool _mandatory = false;

	// value of this variable
	bm::bvector<> _val;

	// dependencies to other variables: other <= this
	vector<Variable *> _slaves;
	
	// dependencies from other variables: this <= other
	vector<Variable *> _masters;
	map<Variable *, unsigned int> _rmasters;
	vector<bool> _changed; // tracks changes propagated from masters

	// System of Inequalities
	QGSimulation *_soi;
	vector<unsigned> _equations;
	vector<unsigned> _coequations;

	bool _const = false;
	bool _isNull = false;

};

#endif /* VARIABLE_H */