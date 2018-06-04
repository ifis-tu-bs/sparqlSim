#include "simulation.h"
#include "variable.h"

#include "graph.h"
#include "edge.h"
#include "label.h"
#include "node.h"

#include <cstdlib>

using namespace std;

// take args_info for deciding on strategy
#include "cmdline.h"
extern gengetopt_args_info args_info;
// global variable - mode of computation

QGSimulation::QGSimulation(Graph &DB) :
	_db(&DB), _q(NULL), _max(DB.size())
{
}

QGSimulation::QGSimulation(QGSimulation &s, bm::bvector<> &ball, bm::bvector<> &border) :
	_db(s._db), _q(NULL), _max(s._max),
	_empty(s._empty), _order(s._order),
	_stable(s._stable), _dirs(s._dirs)
{
	map<Variable *, unsigned> vmap;
	// Copy equations
	for (unsigned i = 0; i < s._sourceV.size(); ++i) {
		if (vmap.find(s._sourceV[i])==vmap.end()) {
			vmap[s._sourceV[i]] = _vars.size();
			_vars.push_back(new Variable(*this, *s._sourceV[i], ball));
			_sourceV.push_back(_vars.back());
		} else {
			_sourceV.push_back(_vars[vmap[s._sourceV[i]]]);
		}
		_operand.push_back(s._operand[i]);
		if (vmap.find(s._targetV[i])==vmap.end()) {
			vmap[s._targetV[i]] = _vars.size();
			_vars.push_back(new Variable(*this, *s._targetV[i], ball));
			_targetV.push_back(_vars.back());
		} else {
			_targetV.push_back(_vars[vmap[s._targetV[i]]]);
		}
	}

	// copy masters & slaves relations
	for (auto &p : vmap) {
		vector<Variable *> &masters = p.first->masters();
		for (Variable *master : masters) {
			_vars[p.second]->addMaster(*_vars[vmap[master]]);
		}

		vector<Variable *> &slaves = p.first->slaves();
		for (Variable *slave : slaves) {
			_vars[p.second]->addSlave(*_vars[vmap[slave]]);
		}


	}

	// Copy labels
	for (auto &p : s._labels) {
		_labels.insert(std::pair<std::string, unsigned>(p.first, p.second));
	}

	// Propagate stability
	for (Variable *v : _vars) {
		bm::bvector<> stable = border & v->val();
		if (stable.any()) {
			v->fullPropagate();
		}
	}
}

QGSimulation::~QGSimulation() {
	for (Variable *var : _vars) {
		delete var;
	}
}

// void QGSimulation::prefixpoint() {
// 	for (auto &i: _order) {
// 		// if current equation is already stable, continue

// 		bm::bvector<> &x = _sourceV[i]->getVal();
// 		bm::bvector<> &y = _targetV[i]->getVal();

// 		bm::bvector<> Y(max());
// 		unsigned int r = x.get_first(); // position of first 1
// 		if (r==0 && !x.test(0)) {
// 			_targetV[i]->null(); // if x is 0, then y is
// 			_stable[i] = true;
// 			if (_targetV[i]->isMandatory()) {
// 				_empty = true;
// 			}
// 			else {
// 				continue;
// 			}
// 		} else {
// 			do {
// 				_operand[i]->rowbv(y, Y, r);
// 			} while ((r=x.get_next(r)) != 0);
// 		}

// 		_targetV[i]->update(Y);

// 		if (_targetV[i]->isEmpty() && 
// 			_targetV[i]->isMandatory()) {
// 			_empty = true;
// 		}
// 	}
// }

unsigned int QGSimulation::fixpoint(const unsigned swtch) {
	bm::bvector<> Y(max());

	unsigned int iter = 0;
	unsigned int r;
	bool columnmode;

	unsigned N,M;
	N = M = 1;
	if (swtch) {
		N = swtch;
		M = swtch+1;
	}

	do {
		iter++;
		for (auto &i: _order) {
			// if current equation is already stable, continue
			if (_stable[i])
				continue;
			// get source variable (including an update)
			// y <= x \times A
			bm::bvector<> &x = _sourceV[i]->getVal();
			bm::bvector<> &y = _targetV[i]->getVal();

			// compute strategy
			// if x.count() < matrix().colNum() <=> !columnmode
			if (x.count() * M < N * y.count()) {
				columnmode = false;
			} else {
				columnmode = true;
			}

			if (!columnmode) {
				r = x.get_first(); // position of first 1
				if (r==0 && !x.test(0)) {
					_targetV[i]->null(); // if x is 0, then y is
				} else {
					Y.reset();
					do {
						_operand[i]->matrix(_dirs[i]).rowbv(y, Y, r);
					} while ((r = x.get_next(r)) != 0);
					_targetV[i]->update(Y);
				}
			} else {
				// Look column-wise
				bool ychanges = false;
				if ((r = y.get_first()) > 0 || y.test(0)) {
					do {
						// check whether column r has a common 1 with x
						if (!_operand[i]->matrix(!_dirs[i]).check(r,x)) {
							y.clear_bit(r);
							ychanges = true;
						}
					} while ((r = y.get_next(r)) != 0);
				}
				if (ychanges) {
					_targetV[i]->propagate();
				}
			}

			_stable[i] = true;
			if (_targetV[i]->isEmpty() && 
				_targetV[i]->isMandatory()) {
				_empty = true;
				return iter;
			}
		}

		// unsigned min, minv, tmp;
		// for (unsigned int opt = 0; opt < _order.size(); ++opt) {
		// 	min = opt;
		// 	tmp = _order[min];
		// 	minv = _sourceV[min]->count();
		// 	for (unsigned int opt2 = opt+1; opt2 < _order.size(); ++opt2) {
		// 		if (minv > _sourceV[_order[opt2]]->count()) {
		// 			min = opt2;
		// 			minv = _sourceV[min]->count();
		// 		}
		// 	}
		// 	_order[opt] = _order[min];
		// 	_order[min] = tmp;
		// }
	} while (!stable());

	bool changes = false; // any changes??
	for (unsigned int i = 0; i < _vars.size(); ++i) {
		if (_vars[i]->updVal()) {
			changes = true;
		}
	}

	return (changes ? fixpoint(swtch) : 0) + iter;
}

void QGSimulation::profile() {
	for (Variable *v : _vars) {
		cout << v->getId() << "(" << v->size() << ") ";
	}
	cout << endl;
}

unsigned int QGSimulation::fixpointWithProfile(const unsigned swtch) {
	bm::bvector<> Y(max());

	unsigned int iter = 0;
	unsigned int r;
	bool columnmode;

	unsigned N,M;
	N = M = 1;
	if (swtch) {
		N = swtch;
		M = swtch+1;
	}

	do {
		iter++;
		profile();
		for (auto &i: _order) {
			// if current equation is already stable, continue
			if (_stable[i])
				continue;
			// get source variable (including an update)
			// y <= x \times A
			bm::bvector<> &x = _sourceV[i]->getVal();
			bm::bvector<> &y = _targetV[i]->getVal();

			// compute strategy
			// if x.count() < matrix().colNum() <=> !columnmode
			if (x.count() * M < N * y.count()) {
				columnmode = false;
			} else {
				columnmode = true;
			}

			if (!columnmode) {
				r = x.get_first(); // position of first 1
				if (r==0 && !x.test(0)) {
					_targetV[i]->null(); // if x is 0, then y is
				} else {
					Y.reset();
					do {
						_operand[i]->matrix(_dirs[i]).rowbv(y, Y, r);
					} while ((r = x.get_next(r)) != 0);
					_targetV[i]->update(Y);
				}
			} else {
				// Look column-wise
				bool ychanges = false;
				if ((r = y.get_first()) > 0 || y.test(0)) {
					do {
						// check whether column r has a common 1 with x
						if (!_operand[i]->matrix(!_dirs[i]).check(r,x)) {
							y.clear_bit(r);
							ychanges = true;
						}
					} while ((r = y.get_next(r)) != 0);
				}
				if (ychanges) {
					_targetV[i]->propagate();
				}
			}

			_stable[i] = true;
			if (_targetV[i]->isEmpty() && 
				_targetV[i]->isMandatory()) {
				_empty = true;
				return iter;
			}
		}
	} while (!stable());

	bool changes = false; // any changes??
	for (unsigned int i = 0; i < _vars.size(); ++i) {
		if (_vars[i]->updVal()) {
			changes = true;
		}
	}

	return (changes ? fixpoint(swtch) : 0) + iter;
}

unsigned int QGSimulation::MaEtAl() {
	bool changes = true;
	unsigned iter = 0;
	while (changes) {
		changes = false;
		iter++;

		for (auto &i : _order) {
			// get source and target vectors
			bm::bvector<> &x = _sourceV[i]->getVal();
			bm::bvector<> &y = _targetV[i]->getVal();

			unsigned r = x.get_first();
			if (r>0 || x.test(0)) {
				do {
					if (!_operand[i]->matrix(_dirs[i]).check(r,y)) {
						x.clear_bit(r);
						changes = true;
					}
				} while (r  = x.get_next(r));
			}

			if (!x.any()) {
				_empty = true;
				break;
			}
		}

		if (_empty) {
			changes = false;
		}
	}

	return iter;
}

bool QGSimulation::stable() const {
	if (_empty)
		return true;
	// if (!_isEmpty[gid]) {
	for (unsigned int i = 0; i < _stable.size(); ++i) {
		if (!_stable[i]) {
			return false;
		}
	}
	// }
	return true;
}

void QGSimulation::unstable(const unsigned int varid) {
	std::vector<unsigned> &eq = _targetV[varid]->equations();
	for (unsigned i: eq) {
		_stable[i] = false;
	}
}

void QGSimulation::unstable1(const unsigned int varid) {
	std::vector<unsigned> &eq = _sourceV[varid]->equations();
	for (unsigned i: eq) {
		_stable[i] = false;
	}
}

void QGSimulation::setUnstable(const unsigned int i) {
	_stable[i] = false;
}

Simulation &QGSimulation::simulation() {
	if (!_computedSim) {
		for (Variable *var: _sourceV) {
	// cout << var->getId() << endl;
			if (_sim.find(var->getId()) == _sim.end()) {
	// cout << "here!" << endl;
				_sim[var->getId()] = var->unify();
			}
		}
		_computedSim = true;
	}

	return _sim;
}

void QGSimulation::print() {
	for (Variable *v : _vars) {
		cout << v->getId() << " : " << v << " " << v->count() << endl;
	}
}

void QGSimulation::push(const bool slavemode) {
	_master.push_back(VMap());
	_slave.push_back(VMMap());
	_slavemode.push_back(slavemode);
}

// void QGSimulation::pop() {
// 	// handle masters/slaves at the same level
// 	for (auto &s: _slave.back()) {
// 		if (_master.back().find(s.first) != _master.back().end()) {
// 			(*s.second) <= (*_master.back()[s.first]);
// 		} else {
// 			if (_master.size() > 1) {
// 				Pair p(s.first, s.second);
// 				_slave[_slave.size()-2].insert(p);
// 			}
// 		}
// 	}

// 	if (_slavemode.back()) {
// 		// Handle this optional pattern
// 		VMap &master = _master[_master.size()-2];
// 		VMMap &slave = _slave[_slave.size()-2];
// 		for (auto &s: _master.back()) {
// 			if (master.find(s.first) != master.end()) {
// 				(*s.second) <= (*master[s.first]);
// 			} else {
// 				Pair p(s.first, s.second);
// 				slave.insert(p);
// 			}
// 		}
// 	} else {
// 		// Copy masters
// 		if (_master.size() > 1) {
// 			VMap &master = _master[_master.size()-2];
// 			for (auto &s: _master.back()) {
// 				master[s.first] = s.second;
// 			}
// 		} else {
// 			for (auto &m : _master.back()) {
// 				m.second->setMandatory(true);
// 			}
			
// 			if (!args_info.random_flag) {
// 				vector<double> Vs, Vs2;
// 				for (unsigned int i = 0; i < _sourceV.size(); ++i) {
// 					Vs.push_back(_dirs[i] ? _operand[i]->a().count() : _operand[i]->aT().count());//) * // y <=
// 					Vs2.push_back(_targetV[i]->degree());
// 				}

// 				for (unsigned int i = 0; i < _order.size(); ++i) {
// 					unsigned int min = i;
// 					for (unsigned int j = i+1; j < _order.size(); ++j) {
// 						if (Vs[_order[j]] < Vs[_order[min]]
// 						 	|| (
// 								Vs[_order[j]] == Vs[_order[min]] && 
// 								Vs2[_order[j]] < Vs2[_order[min]])
// 							) {
// 							min = j;
// 						}
// 					}

// 					unsigned int tmp = _order[min];
// 					_order[min] = _order[i];
// 					_order[i] = tmp;
// 				}
// 			} else {
// 				shuffle();
// 			}
// 		}
// 	}

// 	_master.pop_back();
// 	_slave.pop_back();
// 	_slavemode.pop_back();
// }

void QGSimulation::pop() {
	// handle masters/slaves at the same level
	for (auto &s: _slave.back()) {
		if (_master.back().find(s.first) != _master.back().end()) {
			(*s.second) <= (*_master.back()[s.first]);
		} else {
			if (_master.size() > 1) {
				Pair p(s.first, s.second);
				_slave[_slave.size()-2].insert(p);
			}
		}
	}

	if (_slavemode.back()) {
		// Handle this optional pattern
		VMap &master = _master[_master.size()-2];
		VMMap &slave = _slave[_slave.size()-2];
		for (auto &s: _master.back()) {
			if (master.find(s.first) != master.end()) {
				(*s.second) <= (*master[s.first]);
			} else {
				Pair p(s.first, s.second);
				slave.insert(p);
			}
		}
	} else {
		// Copy masters
		if (_master.size() > 1) {
			VMap &master = _master[_master.size()-2];
			for (auto &s: _master.back()) {
				master[s.first] = s.second;
			}
		} else {
			for (auto &m : _master.back()) {
				m.second->setMandatory(true);
			}
			
			if (!args_info.random_flag) {
				vector<double> Vs, Vs2;
				for (unsigned int i = 0; i < _sourceV.size(); ++i) {
					Vs.push_back(_dirs[i] ? _operand[i]->a().count() : _operand[i]->aT().count());//) * // y <=
					Vs2.push_back(_targetV[i]->degree());
				}

				int min;
				for (unsigned int i = 0; i < _order.size(); ++i) {
					min = -1;
					for (unsigned int j = 0; j < _order.size(); ++j) {
						if (_scheduled[j] || !_sourceV[j]->isSchedulable())
							continue;
						if (min < 0) {
							min = j;
							continue;
						}
						if (Vs[j] < Vs[min]
						 	|| (Vs[j] == Vs[min] && 
								Vs2[j] < Vs2[min])) {
							min = j;
						}
					}

					_order[i] = min;
					_scheduled[min] = true;
				}
			} else {
				shuffle();
			}
		}
	}

	_master.pop_back();
	_slave.pop_back();
	_slavemode.pop_back();
}

void QGSimulation::addTriple(const std::string &sub, const std::string &pred, const std::string &obj) {
	Variable *s;
	Variable *o;

	if (sub[0] != '?') {
		s = addConstant(sub);
	} else {
		s = addVariable(sub);
	}
	if (obj[0] != '?') {
		o = addConstant(obj);
	} else {
		o = addVariable(obj);
	}

	Label &l = _db->getLabel(pred);
	_labels.insert(
		std::pair<std::string,unsigned>(
			l.str(), _sourceV.size()
		)
	);

	s->join(l.aT().colV());
	s->addEquation(_sourceV.size());
	o->addCoequation(_sourceV.size());

	_sourceV.push_back(s);
	_operand.push_back(&l);
	_dirs.push_back(true);
	_targetV.push_back(o);

	_stable.push_back(false);
	_scheduled.push_back(false);
	_order.push_back(_order.size());

	o->join(l.a().colV());
	o->addEquation(_sourceV.size());
	s->addCoequation(_sourceV.size());

	_sourceV.push_back(o);
	_operand.push_back(&l);
	_dirs.push_back(false);
	_targetV.push_back(s);

	_stable.push_back(false);
	_scheduled.push_back(false);
	_order.push_back(_order.size());
}

bool QGSimulation::isTriple(const unsigned int src, const string &lab, const unsigned int tar) {
	
	for (std::multimap<std::string, unsigned>::iterator it = _labels.lower_bound(lab);
		it != _labels.upper_bound(lab); ++it) {
		if (//(*_operand[it->second])(src, tar) &&
			(*_sourceV[it->second])(src) && 
			(*_targetV[it->second])(tar)) {
			return true;
		}
	}

	return false;
}

Variable *QGSimulation::addVariable(const std::string &name) {
	if (_slavemode.back()) {
		if (_master.back().find(name) != _master.back().end()) {
			return _master.back()[name];
		}
	} else {
		for (unsigned int i = _master.size(); i > 0; --i) {
			if (_master[i-1].find(name) != _master[i-1].end()) {
				return _master[i-1][name];
			}
		}
	}

	_vars.push_back(new Variable(this, name, max()));
	_master.back()[name] = _vars.back();
	
	return _vars.back();
}

Variable *QGSimulation::addConstant(const std::string &name) {
	for (unsigned int i = _master.size(); i > 0; --i) {
		if (_master[i-1].find(name) != _master[i-1].end())
			return _master[i-1][name];
	}

	bm::bvector<> *row = new bm::bvector<>(max());
	row->set(_db->getIndex(name));

	_vars.push_back(new Variable(this, name, *row, true));
	_master.back()[name] = _vars.back();

	return _vars.back();
}

void QGSimulation::resetAll() {
	if (_empty)
		return;
	for (unsigned i = 0; i < _stable.size(); ++i) {
		_stable[i] = false;
		_sourceV[i]->getVal();
		_targetV[i]->getVal();
	}

	for (Variable *v : _vars)
		v->getVal();
}

unsigned QGSimulation::countTriples() {
	if (_triplescounted)
		return _triplecount;
	_triplescounted = true;
	_triplecount = 0;

	if (empty()) {
		return 0;
	}

	for (unsigned int i = 0; i < _db->Ssize(); ++i) {
		Label &l = _db->getLabel(i);
		const std::string p = l.str();
		// cout << "label: " << p << endl;
		std::vector<unsigned int> nodepairs = l.getPairs();

		for (unsigned int s = 0; s < nodepairs.size(); s += 2) {
			// cout << "checking nodes " << sim._db->getNode(nodepairs[s])
			//      << " and " << sim._db->getNode(nodepairs[s+1]) << endl;
			if (isTriple(nodepairs[s], p, nodepairs[s+1])) {
				// cout << "success" << endl;
				++_triplecount;
			}
		}
	}

	return _triplecount;
}
