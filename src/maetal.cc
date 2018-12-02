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
	return doMaEtAl();
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

			unsigned r = x.get_first();
			if (r>0 || x.test(0)) {
				do {
					if (!_operand[i]->matrix(_dirs[i]).check(r,y)) {
						x.clear_bit(r);
						changes = true;
					}
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