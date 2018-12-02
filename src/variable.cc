#include "variable.h"
#include "simulation.h"

#include <iostream>
using namespace std;

Variable::Variable(QGSimulation *sim, const string &varid, const unsigned int size) :
	_soi(sim), _varid(varid), _val(size)
{
	_val.set();
}

Variable::Variable(QGSimulation *sim, const string &varid, const bm::bvector<> &val, bool c) :
	_soi(sim), _varid(varid), _const(c), _val(val)
{
}

Variable::~Variable() {
}

Variable::Variable(QGSimulation &sim, Variable &v) :
	_soi(&sim), _varid(v._varid), _mandatory(v._mandatory),
	_changed(v._changed), _equations(v._equations),
	_const(v._const), _isNull(v._isNull), _val(v._val)
{
}

Variable::Variable(QGSimulation &sim, Variable &v, bm::bvector<> &ball) :
	_soi(&sim), _varid(v._varid), _mandatory(v._mandatory),
	_changed(v._changed), _equations(v._equations),
	_const(v._const), _isNull(v._isNull), _val(v._val)
{
	join(ball);
}

Variable::Variable(QGSimulation &sim, Variable &v, bm::bvector<> &ball, bm::bvector<> &border) :
	Variable(sim, v, ball)
{
	bm::bvector<> stable = val() & border;
	if (stable.any()) {
		propagate();
	}
}


bm::bvector<> &Variable::getVal() {
	if (_isNull)
		return _val;

	bool changes = false;
	for (unsigned int i = 0; i < _masters.size(); ++i) {
		if (_changed[i]) {
			if (join(*_masters[i])) {
				changes = true;
			}
			_changed[i] = false;
		}
	}
	// }

	if (_val.none())
		_isNull = true;

	if (changes) {
		propagate();
	}

	return _val;
}

bool Variable::updVal() {
	if (_isNull)
		return false;

	bool changes = false;
	for (unsigned int i = 0; i < _masters.size(); ++i) {
		if (_changed[i]) {
			if (join(*_masters[i])) {
				changes = true;
			}
			_changed[i] = false;
		}
	}

	if (_val.none())
		_isNull = true;

	if (changes) {
		propagate();
	}

	return changes;
}

bm::bvector<> &Variable::val() {
	return _val;
}

bool Variable::join(Variable &other) {
	return join(other.getVal());
}

bool Variable::join(const bm::bvector<> &vec) {
	bm::bvector<> tmp = _val;
	tmp -= vec;
	if (tmp.none())
		return false;
	_val.bit_and(vec);
	return true;
}

bool Variable::update(Variable &other) {
	return update(other.getVal());
}

bool Variable::update(bm::bvector<> &vec) {
	if (_isNull)
		return false;
	updVal();
	if (join(vec) && !isLeaf()) {
		propagate();
		return true;
	}
	return false;
}

void Variable::set(bm::bvector<> &val) {
	// _isNull = true;

	_val = val;
	propagate();
}

void Variable::set(bm::bvector<> &valsmall, bm::bvector<> &valbig) {
	// _isNull = true;
	// high_resolution_clock::time_point t1, t2;
	// duration<double> time_span;
	_val.reset();

	// t1 = high_resolution_clock::now();
	_val |= valsmall;
	// t2 = high_resolution_clock::now();
	// cout << duration_cast<duration<double> >(t2-t1).count() << ":";

	// t1 = high_resolution_clock::now();
	_val &= valbig;	
	// t2 = high_resolution_clock::now();
	// cout << duration_cast<duration<double> >(t2-t1).count() << ":";
	// cout << endl;
	propagate();
}

void Variable::null() {
	if (!_isNull) {
		_val.reset();
		propagate();
		_isNull = true;
	}
}

void Variable::propagate() {
	// tell the slaves
	for (unsigned int i = 0; i < _slaves.size(); ++i) {
		_slaves[i]->changed(*this);
	}

	// tell the equations
	for (unsigned eq: _equations) {
		_soi->setUnstable(eq);
	}
}

// void Variable::fullPropagate() {
// 	propagate();

// 	// tell the equations
// 	for (unsigned eq: _equations) {
// 		_soi->setUnstable(eq+1);
// 	}
// }

void Variable::changed(Variable &master) {
	_changed[_rmasters[&master]] = true;
}

void Variable::addSlave(Variable &other) {
	_slaves.push_back(&other);
}

void Variable::addMaster(Variable &other) {
	// assert(_masters.size() <= 1);
	_rmasters[&other] = _masters.size();
	_masters.push_back(&other);
	_changed.push_back(true);
}

void Variable::operator<=(Variable &master) {
	// cout << _varid <<  " <= " << master._varid << endl;
	// cout << this <<  " <= " << &master << endl;
	master.addSlave(*this);
	addMaster(master);

	for (unsigned i : _equations) {
		for (unsigned j : master._equations) {
			_soi->addTripleNeighbors(i/2,j/2);
		}
	}
}

void Variable::addEquation(const unsigned int idx, SMatrix *m) {
	_equations.push_back(idx);

	if (m != NULL && _remove.find(m) != _remove.end())
		_remove[m] = bm::bvector<>(_soi->max());
}

void Variable::addCoequation(const unsigned int idx) {
	_coequations.push_back(idx);
}

vector<unsigned> &Variable::equations() {
	return _equations;
}

bm::bvector<> Variable::unify() {
	return unifyMasters() | unifySlaves();
}

bm::bvector<> Variable::unifySlaves() {
	bm::bvector<> result = getVal();
	for (unsigned int i = 0; i < _slaves.size(); ++i) {
		result |= _slaves[i]->unifySlaves();
	}
	return result;
}

bm::bvector<> Variable::unifyMasters() {
	bm::bvector<> result = getVal();
	for (unsigned int i = 0; i < _masters.size(); ++i) {
		result |= _masters[i]->unify();
	}
	return result;
}

double Variable::degree() const {
	if (_masters.size())
		return _masters[0]->degree();
	return sdegree();
}

double Variable::sdegree() const {
	double rval = (double) _equations.size();
	for (const Variable *slave : _slaves)
		rval += slave->sdegree();
	return rval;
}

const bool Variable::isSchedulable() {
	for (Variable *master : _masters) {
		if (!master->isScheduled())
			return false;
	}
	return true;
}

const bool Variable::isScheduled() {
	for (unsigned eq : _coequations) {
		if (!_soi->scheduled(eq))
			return false;
	}
	return true;
}

void Variable::initRemoves() {
	for (auto &rem : _remove) {
		// SMatrix::print(rem.second);
		rem.second = rem.first->multiplyMe(_val);
		// SMatrix::print(rem.second);
		rem.second.flip();
		// SMatrix::print(rem.second);
	}
}

const bool Variable::subsumedBy(const bm::bvector<> &bv) {
	unsigned p = _val.get_first();
	if (!p && !_val.test(0))
		return true;

	do {
		if (!bv.test(p))
			return false;
	} while (p = _val.get_next(p));

	return true;
}

const bool Variable::subsumedBy(const Variable &v) {
	return subsumedBy(v._val);
}

const bool Variable::footloose() const {
	for (Variable *v : _group) {
		for (Variable *m : v->_masters) {
			if (v->compare(*m)) {
				return true; 		
			}			
		}
	}
	return false;
}

const bool Variable::compare(Variable &other) const {
	return _val.compare(other._val);
} 
