#include "allds.h"
#include "variable.h"

#include "graph.h"
#include "graph.io.h"
#include "label.h"
#include "node.h"

#include "smatrix.h"

#include "utils.h"

// take args_info for deciding on strategy
#include "cmdline.h"
extern gengetopt_args_info args_info;

AllDS::AllDS(Graph &DB) :
	QGSimulation(DB)
{
}

AllDS::~AllDS() {
}

unsigned AllDS::evaluate(ostream &os) {
	unsigned iter = fixpoint(3);
	_reporter.note("# of iterations", _query, to_string(iter));

	if (empty()) {
		_reporter.note("# of results", _query, to_string(0));
		return 0;
	}
	
	// _stack.push_back(simulation());
	_root = simulation();
	printHeader(out(), _root);

	unsigned results = evaluateStack(os);
	_reporter.note("# of results", _query, to_string(results));

	closeOutputStream();

	return results;
}

unsigned AllDS::evaluateStack(ostream &os) {
	map<string, bm::bvector<> > top = _root;
	unsigned results = 0;

	do {
		// initialize with top element
		initialize(top);
		// compute fixpoint with dependency resolution
		do {
			fixpoint(3);
			resolveDependencies();
		} while (!stable());
		// increase result set size
		if (!empty()) {
			++results;
			// output of the results;
			if (args_info.output_given)
				out() << simulation() << endl;
		}
	} while (minusminus(top));

	return results;
}

void AllDS::initialize(map<string, bm::bvector<> > &sim) {
	for (Variable *v : _vars) {
		// cout << "before: " << v->val() << endl;
		v->set(sim[v->getId()]);
		// cout << "after: " << v->val() << endl;
	}
}

bool AllDS::minusminus(map<string, bm::bvector<> > &sim) {
	unsigned coin, tmp;
	bool result = false;
	for (auto i = sim.rbegin(); i != sim.rend(); ++i) {
		coin = i->second.get_first();
		if (coin || i->second.test(0)) {
			// find last
			while (tmp = i->second.get_next(coin)) {
				coin = tmp;
			}
			// coin = last bit

			result = true;
			auto j = i;
			do {
				if (!coin && !j->second.test(0)) {
					coin = j->second.get_first();
					if (!coin) {
						continue;
					}
				}
				do {
					j->second.flip(coin);
				} while (coin = _root[j->first].get_next(coin));
			} while (j-- != sim.rbegin());
			break;
		}
	}

	return result;
}

void AllDS::setOutput(const string &filename) {
	_filename = filename + ".dual.out";
	setOutputStream(*new ofstream(_filename));
	_fileout = true;
}

void AllDS::statistics(const std::string &filename) {
	ofstream stats;
	stats.open(filename+".dual.statistics", ofstream::app);
	if (!checkStream(filename+".dual.statistics", stats)) 
		return;

	// take Karla's info
	overtakeKarla(filename);

	// output statistics
	statistics(stats);

	stats.close();
}

void AllDS::statistics(std::ostream &os) {
	_reporter(os);
	_reporter.report();
}

void AllDS::csv(const std::string &filename, const char delim) {
	ofstream csv_f;
	_reporter.note("filename", _query, filename);

	csv_f.open(filename+".dual.csv", ofstream::app);
	if (!checkStream(filename+".dual.csv", csv_f)) 
		return;

	overtakeKarla(filename);
	csv(csv_f, delim);

	csv_f.close();
}

void AllDS::csv(std::ostream &os, const char delim) {
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
		case NWD:
		default: {
			os << delim << "ud";
			break;
		}
	}
	os << delim << _queryDepth;

	os  << delim << ">>>" << endl;
}
