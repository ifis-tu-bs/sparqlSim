#include "simulation.h"
#include "variable.h"

#include "graph.h"
#include "label.h"
#include "node.h"

#include "smatrix.h"

#include "utils.h"

#include <cstdlib>
#include <iostream>
#include <algorithm>

using namespace std;

// take args_info for deciding on strategy
#include "cmdline.h"
extern gengetopt_args_info args_info;
// global variable - mode of computation

extern Reporter Karla;

//standard stream for writing nowhere
std::ostream &QGSimulation::devnull = *(new std::ostream(NULL));

//////////////////////////////////
/// Constructors & Destructors ///
//////////////////////////////////

// standard constructor
QGSimulation::QGSimulation()
{
	// useless constructor
}

QGSimulation::QGSimulation(Graph &DB) :
	_class(WD), _db(&DB), _max(DB.size())
{
}

QGSimulation::QGSimulation(QGSimulation &s) :
	_db(s._db), _max(s._max),
	_empty(s._empty), _order(s._order),
	_stable(s._stable), _dirs(s._dirs),
	_class(s._class), 
	_out(s._out), _outputDone(false), 
	_fileout(s._fileout), _filename(s._filename),
	_Labels(s._Labels)
{
	map<Variable *, unsigned> vmap;
	// Copy equations
	for (unsigned i = 0; i < s._sourceV.size(); ++i) {
		if (vmap.find(s._sourceV[i])==vmap.end()) {
			vmap[s._sourceV[i]] = _vars.size();
			_vars.push_back(new Variable(*this, *s._sourceV[i]));
			_sourceV.push_back(_vars.back());
		} else {
			_sourceV.push_back(_vars[vmap[s._sourceV[i]]]);
		}
		_operand.push_back(s._operand[i]);
		if (vmap.find(s._targetV[i])==vmap.end()) {
			vmap[s._targetV[i]] = _vars.size();
			_vars.push_back(new Variable(*this, *s._targetV[i]));
			_targetV.push_back(_vars.back());
		} else {
			_targetV.push_back(_vars[vmap[s._targetV[i]]]);
		}
	}

	// copy masters, slaves, groups relations and dependencies
	for (auto &p : vmap) {
		vector<Variable *> &masters = p.first->masters();
		for (Variable *master : masters) {
			_vars[p.second]->addMaster(*_vars[vmap[master]]);
		}

		vector<Variable *> &slaves = p.first->slaves();
		for (Variable *slave : slaves) {
			_vars[p.second]->addSlave(*_vars[vmap[slave]]);
		}

		// groups
		for (Variable *f : p.first->group()) {
			_vars[p.second]->addFriend(_vars[vmap[f]]);
		}

		// dependencies
		for (auto &e : p.first->dependencies()) {
			switch (e.second) {
				case AND: {
					_vars[p.second]->addANDDependency(_vars[vmap[e.first]]);
					break;
				}
				case OPTIONAL: {
					_vars[p.second]->addOPTDependency(_vars[vmap[e.first]]);
					break;
				}
				default:
					break;
			}
		}
	}

	// Copy labels
	for (auto &p : s._labels) {
		_labels.insert(std::pair<std::string, unsigned>(p.first, p.second));
	}
}

QGSimulation::~QGSimulation() {
	for (Variable *var : _vars) {
		delete var;
	}
}

unsigned QGSimulation::evaluate(std::ostream &os) {
	// os << "computing fixpoint" << endl;

	// _reporter.start("fixpoint", _query);
	unsigned iter = fixpoint(3, os);
	// _reporter.end("fixpoint", _query);

	// os << "   .. done in " << iter << " iterations" << endl;
	// _reporter.note("# of iterations", _query, to_string(iter));

	if (empty()) {
		_reporter.note("# of results", _query, to_string(0));
		return 0;
	}

	_reporter.note("# of results", _query, to_string(1));
	return 1;
}

unsigned QGSimulation::output() {
	if (_outputDone)
		return 0;
	unsigned result;
	if (!_fileout || checkStream(_filename, (ofstream &)out())) {
		result = output(out());
		closeOutputStream();
		_outputDone = true;
	}
}

unsigned QGSimulation::output(const std::string &filename) {
	// TO BE IMPLEMENTED
	ofstream pruning(filename+".reduced.nt");
	unsigned result;
	if (checkStream(filename+".reduced.nt", pruning)) {
		result = output(pruning);
		pruning.close();
	}

	return result;
}

unsigned QGSimulation::output(std::ostream &os) {
	os << "# |E|= " << _db->Esize() << endl;
	os << "#  Q = " << _query; 
	os << *this;

	return 0; // TODO return number of written lines
}

void QGSimulation::setOutput(const string &filename) {
	_filename = filename + ".reduced.nt";
	setOutputStream(*new ofstream(_filename));
	_fileout = true;
}

void QGSimulation::setOutputStream(ostream &os) {
	_out = &os;
	_outputDone = false;
}

void QGSimulation::closeOutputStream() {
	if (_fileout)
		((ofstream &)(*_out)).close();
	_fileout = false;
}

void QGSimulation::overtakeKarla(const std::string &filename) {
	_reporter.note("compilation time", _query, Karla.getValue("Query Compilation Time:", filename));
	_reporter.note("evaluation time", _query, Karla.getValue("Query Evaluation Time:", filename));
	// _reporter.note("output production time", _query, Karla.getValue("Producing Output:", filename));
}

void QGSimulation::statistics(const std::string &filename) {
	ofstream stats;
	stats.open(filename+".statistics", ofstream::app);
	if (!checkStream(filename+".statistics", stats)) 
		return;

	// take Karla's info
	overtakeKarla(filename);

	// output statistics
	statistics(stats);

	stats.close();
}

void QGSimulation::statistics(std::ostream &os) {
	_reporter(os);
	_reporter.report();
}

void QGSimulation::csv(const std::string &filename, const char delim) {
	ofstream csv_f;
	_reporter.note("filename", _query, filename);

	csv_f.open(filename+".csv", ofstream::app);
	if (!checkStream(filename+".csv", csv_f)) 
		return;

	overtakeKarla(filename);
	csv(csv_f, delim);

	csv_f.close();
}

void QGSimulation::csv(std::ostream &os, const char delim) {
	os << "<<<";

	os << delim << _reporter.getValue("filename", _query);
	
	os << delim << _reporter.getValue("compilation time", _query);
	os << delim << _reporter.getValue("fixpoint", _query);
	os << delim << _reporter.getValue("# of iterations", _query);
	os << delim << order();
	os << delim << _reporter.getValue("evaluation time", _query);
	os << delim << _reporter.getValue("# of results", _query);
	
	os << delim	<< _query;
	os << delim << _triplesInQuery;
	os << delim << _optsInQuery;
	switch (_class) {
		case WD: {
			os << delim << "wd";
			break;
		}
		case WWD: {
			os << delim << "wwd";
			break;
		}
		case NWD: {
			os << delim << "ud";
			break;
		}
		default: {
			os << delim << "err";
			break;
		}
	}
	os << delim << _queryDepth;

	os  << delim << ">>>" << endl;
}

std::map<std::string, bm::bvector<> > QGSimulation::simulation() {
	map<string, bm::bvector<> > _sim;
	// if (!_computedSim) {
		for (Variable *var: _sourceV) {
	// cout << var->getId() << endl;
			if (_sim.find(var->getId()) == _sim.end()) {
	// cout << "here!" << endl;
				_sim[var->getId()] = var->unify();
			} else {
				_sim[var->getId()] |= var->unify();
			}
		}
		// _computedSim = true;
	// }

	return _sim;
}

// These are the core methods of the Simulation class

unsigned int QGSimulation::fixpoint(const unsigned swtch, ostream &os) {
	bm::bvector<> Y(max());
	bm::bvector<> *x, *y;

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
		// os << "iteration: " << iter << endl;
		++iter;
		for (auto &i: _order) {
			// os << "try " << i << endl;
			// if (_stable[i] || !(_mandatory[i] || mastersStable()))
			if (_stable[i] || _targetV[i]->isLeaf())
				continue;
			// os << "pass" << endl;
			
			x = &_sourceV[i]->getVal();
			// os << "x = " << x << endl;
			y = &_targetV[i]->getVal();
			// os << "y = " << y << endl;

			// compute strategy
			if (x->count() * M < N * y->count()) {
				columnmode = false;
			} else {
				columnmode = true;
			}

			if (!columnmode) {
				r = x->get_first(); // position of first 1
				if (r==0 && !x->test(0)) {
					_targetV[i]->null(); // if x is 0, then y is
					// assert(false);
				} else {
					Y.reset();
					do {
						_operand[i]->matrix(_dirs[i]).rowbv(*y, Y, r);
					} while ((r = x->get_next(r)) != 0);
					// os << "Y (multiplication) = " << Y << endl;
					_targetV[i]->update(Y);
				}
			} else {
				// Look column-wise
				bool ychanges = false;
				if ((r = y->get_first()) > 0 || y->test(0)) {
					do {
						// check whether column r has a common 1 with x
						if (!_operand[i]->matrix(!_dirs[i]).check(r,*x)) {
							y->clear_bit(r);
							ychanges = true;
						}
					} while ((r = y->get_next(r)) != 0);
				}
				if (ychanges) {
					_targetV[i]->propagate();
				}
			}

			// os << "y (result) = " << y << endl;

			_stable[i] = true;
			if (_targetV[i]->isEmpty() && 
				_targetV[i]->isMandatory()) {
				// cout << "mandatory is empty: " << _targetV[i]->getId() << endl;
				_empty = true;
				return iter;
			}
		}
	} while (!stable2());
	_Stable = true;
	// os << "all inequalities are stable" << endl;

	for (unsigned int i = 0; i < _vars.size(); ++i) {
		if (_vars[i]->updVal()) {
			_Stable = false;
		}
	}

	if (_Stable) {
		for (unsigned &i : _order) {
			if (!_targetV[i]->isLeaf() || _stable[i])
				continue;
			// cout << _targetV[i]->getId() << endl;
			// os << "pass" << endl;
			
			x = &_sourceV[i]->getVal();
			// os << "x = " << x << endl;
			y = &_targetV[i]->getVal();
			// os << "y = " << y << endl;

			// compute strategy
			if (x->count() * M < N * y->count()) {
				columnmode = false;
			} else {
				columnmode = true;
			}

			if (!columnmode) {
				r = x->get_first(); // position of first 1
				if (r==0 && !x->test(0)) {
					_targetV[i]->null(); // if x is 0, then y is
				} else {
					Y.reset();
					do {
						_operand[i]->matrix(_dirs[i]).rowbv(*y, Y, r);
					} while ((r = x->get_next(r)) != 0);
					// os << "Y (multiplication) = " << Y << endl;
					_targetV[i]->update(Y);
				}
			} else {
				// Look column-wise
				if ((r = y->get_first()) > 0 || y->test(0)) {
					do {
						// check whether column r has a common 1 with x
						if (!_operand[i]->matrix(!_dirs[i]).check(r,*x)) {
							y->clear_bit(r);
						}
					} while ((r = y->get_next(r)) != 0);
				}
			}

			// os << "y (result) = " << y << endl;

			_stable[i] = true;
			if (_targetV[i]->isEmpty() && 
				_targetV[i]->isMandatory()) {
				// cout << "mandatory is empty: " << _targetV[i]->getId() << endl;
				_empty = true;
				return iter;
			}
		}

		return iter;
	}

	return fixpoint(swtch, os) + iter;
}

void QGSimulation::profile() {
	for (Variable *v : _vars) {
		cout << v->getId() << "(" << v->size() << ") ";
	}
	cout << endl;
}

/*unsigned int QGSimulation::fixpointWithProfile(const unsigned swtch) {
	bm::bvector<> Y(max());

	unsigned int iter = 0;
	unsigned int r;
	bool columnmode;

	bm::bvector<> *x, *y;

	unsigned N,M;
	N = M = 1;
	if (swtch) {
		N = swtch;
		M = swtch+1;
	}

	do {
		++iter;
		// profile();
		for (auto &i: _order) {
			// if current equation is already stable, continue
			if (_stable[i] || _targetV[i]->isLeaf() || !(_mandatory[i] || mastersStable()))
				continue;
			// get source variable (including an update)
			// y <= x \times A
			x = &_sourceV[i]->getVal();
			y = &_targetV[i]->getVal();

			// compute strategy
			// if x.count() < matrix().colNum() <=> !columnmode
			if (x->count() * M < N * y->count()) {
				columnmode = false;
			} else {
				columnmode = true;
			}

			if (!columnmode) {
				r = x->get_first(); // position of first 1
				if (r==0 && !x->test(0)) {
					_targetV[i]->null(); // if x is 0, then y is
				} else {
					Y.reset();
					do {
						_operand[i]->matrix(_dirs[i]).rowbv(*y, Y, r);
					} while ((r = x->get_next(r)) != 0);
					_targetV[i]->update(Y);
				}
			} else {
				// Look column-wise
				bool ychanges = false;
				if ((r = y->get_first()) > 0 || y->test(0)) {
					do {
						// check whether column r has a common 1 with x
						if (!_operand[i]->matrix(!_dirs[i]).check(r,*x)) {
							y->clear_bit(r);
							ychanges = true;
						}
					} while ((r = y->get_next(r)) != 0);
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
	} while (!stable2());

	bool changes = false; // any changes??
	for (unsigned int i = 0; i < _vars.size(); ++i) {
		if (_vars[i]->updVal()) {
			changes = true;
		}
	}

	if (!changes) {
		// compute all other variables
		for (auto &i : _order) {
			if (!_targetV[i]->isLeaf())
				continue;
			// cout << "computing " << _targetV[i]->getId() << endl;
			// get source variable (including an update)
			// y <= x \times A
			x = &_sourceV[i]->getVal();
			y = &_targetV[i]->getVal();

			// compute strategy
			// if x.count() < matrix().colNum() <=> !columnmode
			if (x->count() * M < N * y->count()) {
				columnmode = false;
			} else {
				columnmode = true;
			}

			if (!columnmode) {
				r = x->get_first(); // position of first 1
				if (r==0 && !x->test(0)) {
					_targetV[i]->null(); // if x is 0, then y is
				} else {
					Y.reset();
					do {
						_operand[i]->matrix(_dirs[i]).rowbv(*y, Y, r);
					} while ((r = x->get_next(r)) != 0);
					_targetV[i]->update(Y);
				}
			} else {
				// Look column-wise
				if ((r = y->get_first()) > 0 || y->test(0)) {
					do {
						// check whether column r has a common 1 with x
						if (!_operand[i]->matrix(!_dirs[i]).check(r,*x)) {
							y->clear_bit(r);
						}
					} while ((r = y->get_next(r)) != 0);
				}
			}

			_stable[i] = true;
			if (_targetV[i]->isEmpty() && 
				_targetV[i]->isMandatory()) {
				_empty = true;
				return iter;
			}
		}

		// no more changes!!
		return iter;
	}

	return fixpoint(swtch) + iter;
}*/

bool QGSimulation::stable2() const {
	if (_empty)
		return true;
	for (unsigned int i = 0; i < _stable.size(); ++i) {
		if (!_stable[i] && !_targetV[i]->isLeaf()) {
			return false;
		}
	}

	return true;
}

bool QGSimulation::stable() const {
	if (_empty)
		return true;
	for (unsigned int i = 0; i < _stable.size(); ++i) {
		if (!_stable[i]) {
			return false;
		}
	}

	return true;
}

bool QGSimulation::mastersStable() const {
	unsigned m = _mandatory.get_first();
	if (!m && !_mandatory[0])
		return true;
	do {
		if (!_stable[m])
			return false;
	} while (m = _mandatory.get_next(m));
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

// Simulation &QGSimulation::simulation() {
// 	if (!_computedSim) {
// 		for (Variable *var: _sourceV) {
// 	// cout << var->getId() << endl;
// 			if (_sim.find(var->getId()) == _sim.end()) {
// 	// cout << "here!" << endl;
// 				_sim[var->getId()] = var->unify();
// 			}
// 		}
// 		_computedSim = true;
// 	}

// 	return _sim;
// }

void QGSimulation::print() {
	for (Variable *v : _vars) {
		cout << v->getId() << " : " << v << " " << v->count() << endl;
	}
}

void QGSimulation::push(const bool slavemode) {
	_master.push_back(VMap());
	if (slavemode) {
		++_optsInQuery;
	}
	_slave.push_back(VMMap());
	_slavemode.push_back(slavemode);
}

void QGSimulation::pop() {
	// Update query depth for statistics
	unsigned depth = 0;
	for (unsigned i = 0; i < _slavemode.size(); ++i) {
		depth += (_slavemode[i] ? 1 : 0);
	}
	if (depth > _queryDepth) {
		_queryDepth = depth;
	}

	// handle masters/slaves at the same level
	for (auto &s: _slave.back()) {
		// cout << s.first << "(" << s.second << ")" << endl;
		if (_master.back().find(s.first) != _master.back().end()) {
			// cout << s.second << " <= " << _master.back()[s.first] << endl;
			(*s.second) <= (*_master.back()[s.first]);
		} else {
			// cout << "not " << s.second << " <= " << endl;
			// setting group variables
			for (auto &m : _master.back()) {
				m.second->addFriend(s.second);
			}

			// pushing slave to the next level
			if (_master.size() > 1) {
				// cout << "before pushing" << endl;
				for (VMMap::iterator sX = _slave[_slave.size()-2].lower_bound(s.first);
					sX != _slave[_slave.size()-2].upper_bound(s.first);
					++sX) {
					// cout << "setting _class = " << _class << " to " << NWD << endl;
					_class = NWD;
					// cout << "combine " << s.first << " with " << sX->first << endl;
					// cout << "combine " << s.second << " with " << sX->second << endl;
					if (_slavemode.back()) {
						// cout << " optionally" << endl;
						sX->second->addOPTDependency(s.second);
					} else {
						// cout << " conjunctively" << endl;
						sX->second->addANDDependency(s.second);
						s.second->addANDDependency(sX->second);
					}
				}

				Pair p(s.first, s.second);
				_slave[_slave.size()-2].insert(p);
			}
		}
	}

	// Group all current masters
	for (auto &m : _master.back()) {
		for (auto &mm : _master.back()) {
			if (m != mm) {
				m.second->addFriend(mm.second);
			}
		}
	}

	if (_slavemode.back()) {
		// Handle this optional pattern
		VMap &master = _master[_master.size()-2];
		VMMap &slave = _slave[_slave.size()-2];
		for (auto &s: _master.back()) {
			if (master.find(s.first) != master.end()) {
				// cout << s.second << " <= " << master[s.first] << endl;
				(*s.second) <= (*master[s.first]);
			} else {
				if (slave.find(s.first) != slave.end()) {
					// cout << "setting _class = " << _class << " to " << WWD << endl;
					_class = WWD;
				}
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
			VMMap &slave = _slave[_slave.size()-2];
			for (auto &s: _slave.back()) {
				Pair p(s.first, s.second);
				slave.insert(p);
			}
		} else {
			for (auto &m : _master.back()) {
				m.second->setMandatory(true);
			}
			
			if (!args_info.random_flag && !args_info.permute_flag) {
				vector<double> Vs, Vs2;
				for (unsigned int i = 0; i < _sourceV.size(); ++i) {
					Vs.push_back(_dirs[i] ? _operand[i]->a().count() : _operand[i]->aT().count());//) * // y <=
					Vs2.push_back(_targetV[i]->degree());
				}

				int min;
				for (unsigned int i = 0; i < _order.size(); ++i) {
					min = -1;
					for (unsigned int j = 0; j < _order.size(); ++j) {
						if (!schedulable(j, true))
							continue;
						if (min < 0) {
							min = j;
							continue;
						}
						if (Vs[j] < Vs[min]
						 	|| (Vs2[j] < Vs2[min] && 
								Vs[j] == Vs[min])) {
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

	// Karla << sub << " " << pred << " " << obj;

	if (sub[0] != '?') {
		s = addConstant(sub);
	} else {
		s = addVariable(sub);
	}
	if (obj[0] != '?') {
		// cout << obj << endl;
		o = addConstant(obj);
		// cout << o->val().count() << endl;
	} else {
		o = addVariable(obj);
	}

	Label &l = _db->getLabel(pred);
	_labels.insert(
		std::pair<std::string,unsigned>(
			l.str(), _sourceV.size()
		)
	);
	_Labels.insert(&l);

	s->join(l.aT().colV());
	s->addEquation(_sourceV.size());
	o->addCoequation(_sourceV.size());

	s->addRemove(l.matrix_p(true));
	o->addRemove(l.matrix_p(false));

	_mandatory.set_bit(_sourceV.size(), !_slavemode.back());
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

	_mandatory.set_bit(_sourceV.size(), !_slavemode.back());
	_sourceV.push_back(o);
	_operand.push_back(&l);
	_dirs.push_back(false);
	_targetV.push_back(s);

	_stable.push_back(false);
	_scheduled.push_back(false);
	_order.push_back(_order.size());

	// triple updates
	++_triplesInQuery;
	bm::bvector<> tmp;
	tmp.reset();
	for (unsigned i = 0; i < _tripleNeighbors.size(); ++i) {
		if (s == _sourceV[i*2] || s == _targetV[i*2]
			|| o == _sourceV[i*2] || o == _targetV[i*2]) {
			// cout << i << ":" << _tripleNeighbors.size() << endl;
			_tripleNeighbors[i].set(_tripleNeighbors.size());
			tmp.set(i);
		}
	}
	_tripleNeighbors.push_back(tmp);
}

void QGSimulation::addTripleNeighbors(const unsigned i, const unsigned j) {
	_tripleNeighbors[i].set(j);
	_tripleNeighbors[j].set(i);
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
	// cout << _vars.size() << endl;
	_master.back()[name] = _vars.back();

	// cout << name << ":" << _vars.back() << endl;
	
	return _vars.back();
}

Variable *QGSimulation::addConstant(const std::string &name) {
	for (unsigned int i = _master.size(); i > 0; --i) {
		if (_master[i-1].find(name) != _master[i-1].end())
			return _master[i-1][name];
	}

	bm::bvector<> *row = new bm::bvector<>(max());
	string newname;
	if (_db->existsNode(name)) {
		row->set(_db->getNodeIndex(name));
		newname = to_string(_db->getNodeIndex(name));
	} else {
		row->reset();
		_empty = true;
	}

	_vars.push_back(new Variable(this, newname, *row, true));
	_master.back()[newname] = _vars.back();

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

const bool QGSimulation::schedulable(unsigned eq, const bool b) {
	// static unsigned div = (_scheduled.size() <= 6 ? 2 : _scheduled.size() / 2 - 1);
	static unsigned div = 2;

	if (_scheduled[eq])
		return false;
	if (!_sourceV[eq]->isSchedulable())
		return false;
	if (!_mandatory[eq] && !mastersScheduled())
		return false;

	// breaks for actual H1 heuristic
	if (b)
		return true;

	unsigned offset = eq % 2;
	if (_mandatory[eq]) {
		if (rand() % div)
			return true;
		if (offset ? _scheduled[eq-1] : _scheduled[eq+1]) {
			for (unsigned i = 0; i < _scheduled.size(); i=i+2) {
				if (_mandatory[i] && !(_scheduled[i] || _scheduled[i+1]))
					return false;
			}				
		}
	} else {
		if (rand() % div)
			return true;
		if (offset ? _scheduled[eq-1] : _scheduled[eq+1]) {
			for (unsigned i = 0; i < _scheduled.size(); i=i+2) {
				if (!_mandatory[i] && !(_scheduled[i] || _scheduled[i+1]))
					return false;
			}
		}
	}
	return true;
}

const bool QGSimulation::resolveDependencies() {
	bool result = false;
	map<Variable *, Dependency> deps;
	for (Variable *v : _vars) {
		// cout << v->getId() << "(" << v << ")" << ":" << endl;
		deps = v->dependencies();
		for (auto &dep : deps) {
			// cout << "  " << dep.first->getId() << "(" << dep.first << ")";
			switch (dep.second) {
				case AND:
					// cout << " AND" << endl;
					if (v->isEmpty(true) || dep.first->isEmpty(true))
						break;
					// cout << "before: " << v->getVal() << " || " << dep.first->getVal() << endl;
					result = result | v->update(*dep.first);
					result = result | dep.first->update(*v);
					break;
				case OPTIONAL:
					// cout << " OPT" << endl;
					// if not dep <= v
					if (!dep.first->subsumedBy(*v)) {
						if (!v->footloose()) {
							result = result | dep.first->update(*v);
						}
					}
					break;
				default:
					break;
			}
		}
	}

	return result;
}

void QGSimulation::printHeader(std::ostream &os, std::map<std::string, bm::bvector<> > &sim) {
	unsigned first = 0;
	for (auto &e : sim) {
		os << (first++ ? "||" : "" ) << "# " << e.first << " #";
	}
	os << endl;
}
