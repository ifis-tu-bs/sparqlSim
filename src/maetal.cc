#include "maetal.h"
#include "variable.h"

#include "graph.h"
#include "label.h"
#include "node.h"

#include "smatrix.h"


MaEtAl::MaEtAl(Graph &db) :
  QGSimulation(db)
{
}

MaEtAl::~MaEtAl() {

}

unsigned MaEtAl::evaluate(std::ostream &os) {
	doMaEtAl();

	for (unsigned i = 0; i < _vars.size(); ++i) {
		// cout << _vars[i]->val() << endl;
		if (_vars[i]->val().none()) {
			_reporter.note("# of results", _query, to_string(0));
			return 0;
		}
	}
	_reporter.note("# of results", _query, to_string(1));
	return 1;
}

unsigned int MaEtAl::doMaEtAl() {
	bool changes = true;
	unsigned iter = 0;
	while (changes) {
		changes = false;
		iter++;

		for (auto &i : _order) {
			// get source and target vectors
			bm::bvector<> &x = _sourceV[i]->getVal();
			bm::bvector<> &y = _targetV[i]->getVal();

			if (x.any()) {
				unsigned r = x.get_first();
				do {
					bm::bvector<> rr(_max);
					rr.set(r);
					rr = _operand[i]->matrix(_dirs[i]).multiplyMe(rr);
					bool remove = true;
					if (rr.any()) {
						unsigned rrr = rr.get_first();
						do {
							if (y.test(rrr)) {
								remove = false;
								break;
							}
						} while (rrr = rr.get_next(rrr));
					}

					if (remove)
						x.set(r,false);
				} while (r = x.get_next(r));
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