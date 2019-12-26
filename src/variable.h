#ifndef VARIABLE_H
#define VARIABLE_H

#include "bm.h"
#include "smatrix.h"

#include <cassert>
#include <map>
#include <string>
#include <vector>

#include <cmath>

using namespace std;

class QGSimulation;

typedef enum { AND, OPTIONAL } Dependency;


class Variable {
public:
	// constructs new variable
	Variable(QGSimulation *sim, const string &varid, const unsigned int size);
	// constructs new variable with existing val
	Variable(QGSimulation *sim, const string &varid, const bm::bvector<> &val, bool c = false);
	~Variable();

	Variable(QGSimulation &sim, Variable &v);
	Variable(QGSimulation &sim, Variable &v, bm::bvector<> &ball);
	Variable(QGSimulation &sim, Variable &v, bm::bvector<> &ball, bm::bvector<> &border);

	string getId() const { return _varid; }

	bm::bvector<> &val(); // returns _val
	bm::bvector<> &getVal(); // recomputes _val and returns it
	bool updVal();

	bool operator()(unsigned i) { return _val.test(i); }

	bool join(Variable &); // recomputes _val
	bool join(const bm::bvector<> &); //

	const bool subsumedBy(const bm::bvector<> &);
	const bool subsumedBy(const Variable &);

	bool update(Variable &); // updates my value accordingly
	bool update(bm::bvector<> &); // updates my value accordingly

	void set(bm::bvector<> &); // sets the value
	void set(bm::bvector<> &, bm::bvector<> &);
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

	void addEquation(const unsigned int idx, SMatrix * = NULL);
	void addCoequation(const unsigned int idx);
	vector<unsigned> &equations();
	vector<unsigned> &coEquations() { return _coequations; }

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

	const bool isLeaf() const {
		return (_equations.size() == 1);
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

	void addFriend(Variable *v) {
		_group.push_back(v);
	}

	void addANDDependency(Variable *v) {
		_deps[v] = AND;
	}

	void addOPTDependency(Variable *v) {
		_deps[v] = OPTIONAL;
	}

	vector<Variable *> &group() {
		return _group;
	}

	map<Variable *, Dependency> &dependencies() {
		return _deps;
	}

	const bool footloose() const;

	const bool compare(Variable &) const;

	unsigned get_first() const;
	unsigned get_next(unsigned i) const;
	const bool test(unsigned i) const;

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

	// the other variables of the group this variable occurs in
	vector<Variable *> _group;

	std::map<Variable *, Dependency> _deps;

public:
	void initRemoves();

	bm::bvector<> remove(SMatrix *s) {
		return _remove[s];
	}

	void addRemove(SMatrix *s) {
		_remove[s] = bm::bvector<>();
		_remove[s].reset();
	}

	void minus(bm::bvector<> &rset) {
		// cerr << "trying to subtract" << endl;
		// SMatrix::print(_val);
		// SMatrix::print(rset);
		_val -= rset;
		// cerr << "success" << endl;
	}

	void clearRemoveSet(SMatrix *s) {
		_remove[s].reset();
	}

	void addRemoveSet(SMatrix *s, bm::bvector<> &rem) {
		_remove[s] |= rem;
	}

private:
	// for HHK algorithm:
	map<SMatrix *, bm::bvector<> > _remove;
	//vector<SMatrix *> _remover; // pre/post to update remove

};

#endif /* VARIABLE_H */
