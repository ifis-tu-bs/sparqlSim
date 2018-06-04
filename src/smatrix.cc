#include "smatrix.h"

/// Constructors and Destructors

SMatrix::SMatrix() :
	_colV(STRATEGY) 
{ 
	_colV.reset();
}

SMatrix::SMatrix(unsigned int n) :
	_max(n), _colV(n, STRATEGY) 
{ 
	_colV.reset(); 
}

SMatrix::~SMatrix() {
	
}

std::vector<unsigned int> SMatrix::getPairs() {
	std::vector<unsigned int> result;

	for (auto row = _rows.begin(); row != _rows.end(); ++row) {
		for (unsigned int i = 0; i < row->second.size(); ++i) {
			result.push_back(row->first);
			result.push_back(row->second[i]);
		}
	}

	return result;
}