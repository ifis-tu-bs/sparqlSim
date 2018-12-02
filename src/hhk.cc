#include "hhk.h"
#include "variable.h"

#include "graph.h"
#include "label.h"
#include "node.h"

#include "smatrix.h"


HHK::HHK(Graph &db) :
  QGSimulation(db)
{
}

HHK::~HHK() {

}

unsigned HHK::evaluate(std::ostream &os) {
	return doHHK();
}

unsigned int HHK::doHHK() {

	/// 0. Initialization
	unsigned iter = 0;

	// cerr << "HHK IS NOT IMPLEMENTED, YET" << endl;
	// return 0;

	/// 1. Initialization of remove vectors
	for (Variable *v : _vars) {
		// cerr << "init remove of " << v->getId() << endl;
		v->initRemoves();
	}


	bool changes = true;
	/// 2. Iteration
	while (changes) {
		++iter;
		changes = false;

		for (unsigned i : _order) {
			SMatrix *s = _operand[i]->matrix_p(_dirs[i]);

			// SMatrix::print(_targetV[i]->val());
			// SMatrix::print(_sourceV[i]->remove(s));
			bm::bvector<> removals = _targetV[i]->val() & _sourceV[i]->remove(s);
			if (removals.none())
				continue;
			// cerr << "HHK: there are removals" << endl;
			_targetV[i]->minus(removals);
			// cerr << "updated target value" << endl;

			/// Update Remove Set of Target
			for (unsigned eq : _targetV[i]->equations()) {
				if (eq == i)
					continue;
				SMatrix *seq = _operand[eq]->matrix_p(_dirs[eq]);
				bm::bvector<> wprime = seq->multiplyMe(removals);
				wprime -= seq->multiplyMe(_targetV[i]->val());

				if (wprime.any()) {
					changes = true;
					_targetV[i]->addRemoveSet(seq, wprime);
				}
			}

			_sourceV[i]->clearRemoveSet(s);
		}

	}

	return iter;
}