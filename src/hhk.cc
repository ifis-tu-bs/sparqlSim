#include "hhk.h"
#include "variable.h"

#include "utils.h"

#include "graph.h"
#include "label.h"
#include "node.h"

#include "smatrix.h"

#include "cmdline.h"

extern Reporter Karla;
extern gengetopt_args_info args_info;

void HHK::initCounts() {
	unsigned iter = 0;
	unsigned iter2 = 0;

	if (_wc) {
		bm::bvector<> uv(_max);
		bm::bvector<> ucs(_max);
		for (unsigned i = 0; i < _vars.size(); ++i) {
			map<SMatrix *, unsigned*> current;

			vector<unsigned> edges_to_v = _vars[i]->equations();
			for (unsigned eq: edges_to_v) {
				SMatrix *bw = _operand[eq]->matrix_p(!_dirs[eq]);
				current[bw] = new unsigned[_max];
				ucs = _trans[bw]->multiplyMe(_vars[i]->val());
				++iter;
				if (ucs.any()) {
					unsigned u = ucs.get_first();
					do {
						uv.set(u);
						current[bw][u] = (bw->multiplyMe(uv) & _vars[i]->val()).count();
						++iter2;
						uv.reset();
					} while (!_timeOut && (u = ucs.get_next(u)));
				}
				if (_timeOut)
					break;
			}

			_count.push_back(current);
			if (_timeOut)
				break;
		}
	}

	cout << "computing us (in u-a->v): " << iter << endl << "computing count entries: " << iter2 << endl;
}

void HHK::initRemoves() {
  for (unsigned i = 0; i < _vars.size(); ++i) {
    map<SMatrix *, bm::bvector<> > current;

    vector<unsigned> edges_to_v = _vars[i]->equations();
    for (unsigned eq: edges_to_v) {

      SMatrix *fw = _operand[eq]->matrix_p(_dirs[eq]);
      SMatrix *bw = _operand[eq]->matrix_p(!_dirs[eq]);

      // collect transposed matrices
      _trans[bw] = fw;
      _trans[fw] = bw;

     	// Standard remove(v) turns into remove-a->(v), i.e., remove_Fa(v)
      current[bw] = fw->multiplyMe(_vars[i]->getVal()).flip();
      current[bw].resize(_max);

      // set pointers to first variable for hhk()
      // this is only done if
      if (_nextM == NULL && current[bw].any()) {
        _nextV = i;
        _nextM = bw;
      }
    }

    _remove.push_back(current);
  }
}

HHK::HHK(Graph &db, const bool withCount) :
  QGSimulation(db), _wc(withCount)
{
}

HHK::~HHK() {

}

void HHK::timeOut(const unsigned t, bool *toRef) {
	high_resolution_clock::time_point start = high_resolution_clock::now();
	high_resolution_clock::time_point T = high_resolution_clock::now();
	duration<double> time_span;

	do {
		T = high_resolution_clock::now();
		time_span = duration_cast<std::chrono::seconds>(T-start);
	} while (time_span.count() < t && !(*toRef));
	cout << "timeout" << endl;

	(*toRef) = true;
}

unsigned HHK::evaluate(std::ostream &os) {
	unsigned iter;

	if (args_info.timeout_arg) {
		tout = thread(timeOut,args_info.timeout_arg,&_timeOut);
	}

	_reporter.start("init removes",_query);
	initRemoves();
	_reporter.end("init removes",_query);
	cout << " .. removes initialized" << endl;
	if (!_timeOut) {
		_reporter.start("init counts",_query);
		initCounts();
		_reporter.end("init counts",_query);
		cout << " .. counts initialized" << endl;
	}
	if (!_timeOut) {
		cout << "    computing fixpoint .." << endl;
		_reporter.start("hhk",_query);
		iter = doHHK();
		_reporter.end("hhk",_query);
	}

	if (_finish) {
		if (args_info.timeout_arg) {
			_timeOut = true;
			tout.join();
		}
		_reporter.note("# of iterations",_query,to_string(iter));

		for (unsigned i = 0; i < _vars.size(); ++i) {
			// cout << _vars[i]->val() << endl;
			if (_vars[i]->val().none() || _empty) {
				_reporter.note("# of results", _query, to_string(0));
				return 0;
			}
		}
		_reporter.note("# of results", _query, to_string(1));
		return 1;
	}
	if (args_info.timeout_arg) {
		tout.join();
		cout << "    abort due to timeout." << endl;
	}

	_reporter.note("# of results", _query, "timeout");

	return 0;
}

unsigned int HHK::doHHK() {
	bool changes = true;
	unsigned iter = 0;

	// current remove set
	bm::bvector<> rem(_max);
	// variables w and u
	unsigned w,u,wprimev,w2primev;
	// predecessors of u
	bm::bvector<> wprime(_max);
	bm::bvector<> w2prime(_max);

	bool inter;

	while (changes && !_timeOut) {
		changes = false;
		++iter;

		unsigned ic = 0;


		for (auto &i: _order) {
			for (_nextV=0; _targetV[i] != _vars[_nextV]; ++_nextV);
			_nextM = _operand[i]->matrix_p(_dirs[i]);

			rem.swap(_remove[_nextV][_nextM]);
			_remove[_nextV][_nextM].reset();

			if (rem.any()) {
				for (unsigned eq: _vars[_nextV]->equations()) {
					w = rem.get_first();
					do {
						if (_operand[eq]->matrix_p(!_dirs[eq]) != _nextM)
							continue;
						if (_targetV[eq]->val().test(w)) {
							// find variable u
							for (u = 0; _vars[u]!=_targetV[eq]; ++u);

							_vars[u]->val().set(w,false);

							if (_vars[u]->val().none()) { // if variable turns out empty, terminate
								_empty = true;
								return iter;
							}

							for (auto &p: _remove[u]) {
								wprime.set(w);
								wprime = _trans[p.first]->multiplyMe(wprime); // get all w''

								if (wprime.any()) {
									wprimev = wprime.get_first();
									do {
										if (_wc) {
											if (_count[u][p.first][wprimev]) {
												--_count[u][p.first][wprimev];
											}
											if (!_count[u][p.first][wprimev]) {
												changes = true;
												p.second.set(wprimev);
											}
										} else {
											w2prime.set(wprimev);
											w2prime = p.first->multiplyMe(w2prime);

											inter = false; // flag, showing intersection non-empty
											if (w2prime.any()) {
												w2primev = w2prime.get_first();

												do {
													if (_vars[u]->val().test(w2primev)) {
														inter = true;
														break;
													}
												} while (w2primev = w2prime.get_next(w2primev));

												w2prime.reset();

												if (!inter) {
													changes = true;
													p.second.set(wprimev);
												}
											}
										}
									} while (wprimev = wprime.get_next(wprimev));
								}

								// clean up wprime
								wprime.reset();
							}
						}
					} while (w = rem.get_next(w));
				}
			}
		}
	}

	// for (unsigned i = 0; i < _vars.size(); ++i)
	// 	for (auto &p: _remove[i]) {
	// 		assert(p.second.none());
	// 	}

	if (!_timeOut)
		_finish = true;

	return iter;
}

void HHK::csv(const std::string &filename, const char delim) {
	ofstream csv_f;
	_reporter.note("filename", _query, filename);

	csv_f.open(filename+".csv", ofstream::app);
	if (!checkStream(filename+".csv", csv_f))
		return;

	overtakeKarla(filename);
	csv(csv_f, delim);

	csv_f.close();
}

void HHK::csv(std::ostream &os, const char delim) {
	os << "<<<";

	os << delim << _reporter.getValue("filename", _query);

	os << delim << _reporter.getValue("compilation time", _query);

	os << delim << _reporter.getValue("init removes", _query);
	os << delim << _reporter.getValue("init counts", _query);
	os << delim << _reporter.getValue("hhk", _query);

	os << delim << _reporter.getValue("evaluation time", _query);
	os << delim << _reporter.getValue("# of results", _query);

	os << delim << _reporter.getValue("# of iterations", _query);

  	// os << delim << countTriples();

	// os << delim << order();
	os << delim	<< _query;
	os << delim << _vars.size();
	os << delim << _Labels.size();
	os << delim << _triplesInQuery;
	// os << delim << _optsInQuery;
	// switch (_class) {
	// 	case WD: {
	// 		os << delim << "wd";
	// 		break;
	// 	}
	// 	case WWD: {
	// 		os << delim << "wwd";
	// 		break;
	// 	}
	// 	case NWD: {
	// 		os << delim << "ud";
	// 		break;
	// 	}
	// 	default: {
	// 		os << delim << "err";
	// 		break;
	// 	}
	// }
	// os << delim << _queryDepth;

	os  << delim << ">>>" << endl;
}
