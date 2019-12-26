#include "smatrix.h"

#include <cassert>

using namespace std;

/// Constructors and Destructors

SMatrix::SMatrix() :
	_colV(STRATEGY), 
	MODE(IMPORT), 
	_rows(new map<unsigned, vector<unsigned> >())
{ 
	_colV.reset();
}

SMatrix::SMatrix(unsigned int n) :
	_max(n), 
	_colV(n, STRATEGY), 
	MODE(IMPORT),
	_rows(new map<unsigned, vector<unsigned> >())
{ 
	_colV.reset(); 
}

SMatrix::SMatrix(const SMatrix &s, const bool transpose) :
	_max(s._max), /// maximal size the same
	MODE(s.MODE),
	_rows(NULL)
{
	_colV.reset();

	switch (MODE) {
		case IMPORT: {
			_rows = new map<unsigned, vector<unsigned> >();
			break;
		}
		case LOAD:
		default: 
			break;
	}

	if (transpose) { /* transposed deep copy */

		// set column vector
		for (unsigned row = 0; row <  s._rowNums.size(); ++row) {
			_colV.set_bit(s._rowNums[row]);
		}

		// fill row numbers
		map<unsigned,vector<unsigned> > pre;
		unsigned col = s._colV.get_first();
		if (col || s._colV.test(0)) {
			do {
				pre[col] = vector<unsigned>();
			} while (col = s._colV.get_next(col));
		}

		// fill pre-matrix
		for (unsigned i = 0; i < s._rowNums.size(); ++i) {
			unsigned row = s._rowNums[i];
			for (unsigned r = s._rowBegin[i]; r < s._rowBegin[i+1]; ++r) {
				pre[s._rows2[r]].push_back(row);
			}
		}

		// transform pre-matrix
		includeMap(pre);
	} else { /* normal deep copy */

	}
}

SMatrix::~SMatrix() {
	
}

std::vector<unsigned int> SMatrix::getPairs() {
	std::vector<unsigned int> result;

	switch (MODE) {
		case IMPORT: {
			for (auto row = _rows->begin(); row != _rows->end(); ++row) {
				for (unsigned int i = 0; i < row->second.size(); ++i) {
					result.push_back(row->first);
					result.push_back(row->second[i]);
				}
			}
			break;
		}
		case LOAD: {
			for (unsigned r = 0; r < _rowNums.size(); ++r) {
				for (unsigned c = _rowBegin[r]; c < _rowBegin[r+1]; ++c) {
					result.push_back(_rowNums[r]);
					result.push_back(_rows2[c]);
				}
			}
			break;
		}
		default: break;
	}

	return result;
}

bool SMatrix::operator()(unsigned int i, unsigned int j) {
	if (rowNull(i) || !_colV.test(j)) /* if row or column is 0 */
		return false;                 /* return false */

	switch (MODE) {
		case IMPORT: {
			for (auto &k: (*_rows)[i]) {
				if (k == j) return true;
			}
			break;
		}
		case LOAD: {
			unsigned row = findRow(i);
			for (unsigned k = _rowBegin[i]; k < _rowBegin[i+1]; ++k) {
				if (_rows2[k] == j) return true;
			}
			break;
		}
		default: break;
	}

	return false;
}

void SMatrix::set(unsigned int i, unsigned int j) {
	// cout << "setting " << i << "," << j << endl;
	// cout << "here " << (_rows == NULL) << endl;
	if (rowNull(i)) {
		(*_rows)[i] = vector<unsigned>();
		_rowCount++;
	}
	(*_rows)[i].push_back(j); // implementation-specific
	// (*_rows)[i].insert(j); // implementation-specific
	_colV.set(j);
	// std::cout << *this << std::std::endl;

	++_count;
}

void SMatrix::set2(unsigned int i, unsigned int j) {
	// cout << "setting " << i << "," << j << endl;
	if (!_LastRow || _lastRow != i) { // if first row or next
	// cout << "here " << i << " " << j << endl;
		_rowNums.push_back(i);
		_rowBegin.push_back(_rows2.size());
		++_rowCount;

		_LastRow = true;
		_lastRow = i;
	}
	_rows2.push_back(j); // implementation-specific
	// (*_rows)[i].insert(j); // implementation-specific
	_colV.set(j);
	// std::cout << *this << std::std::endl;

	++_count;
}

void SMatrix::makeFinal() {
	_rowBegin.push_back(_rows2.size());
}

// void SMatrix::loadRow(const std::string &line) {
// 	// cout << "load" << endl;
// 	string row, size, p;
// 	unsigned last, next;
// 	last = 0;


// 	// read row
// 	next = line.find(':');
// 	row = line.substr(last, next);
// 	last = next+1;
// 	// read size
// 	next = line.find(':', last);
// 	size = line.substr(last, next-last);
// 	last = next+1;

// 	unsigned rnum = stoi(row);
// 	unsigned rsize = stoi(size);


// 	// initialize row
// 	// (*_rows)[rnum] = vector<unsigned>(rsize);
// 	_rowNums.push_back(rnum);
// 	_rowSize.push_back(rsize);
// 	// _rows2.push_back(vector<unsigned>(rsize));
// 	if (!_rowBegin.size())
// 		_rowBegin.push_back(_rows2.size());

// 	vector<unsigned> tmp;
// 	bool substr_possible = true;

// 	for (unsigned i = 0; i < rsize-1; ++i) {
// 		next = line.find(':',last);

// 		tmp.push_back(stoi(line.substr(last,next-last)));

// 		if (!_colV.test(tmp.back())) {
// 			_colV.set(tmp.back());
// 			substr_possible = false;
// 		}


// 		last = next+1;
// 	}

// 	tmp.push_back(stoi(line.substr(last)));
// 	if (!_colV.test(tmp.back())) {
// 		_colV.set(tmp.back());
// 		substr_possible = false;
// 	}

// 	// finding perfect substring within _rows2
// 	// bool substr_found = false;
// 	unsigned pos = 0; 
// 	if (substr_possible) {
// 		for (unsigned i = 0; i < _rows2.size(); ++i) {
// 			if (_rows2[i] != tmp[pos]) {
// 				pos = 0;
// 			} else {
// 				++pos;
// 			}
// 			if (pos == rsize) {
// 				// cout << "succ:" << rsize << endl;
// 				_rowBegin.back() = i - pos;
// 				break;
// 			}
// 		}
// 	}
// 	if (pos < rsize) {
// 		for (unsigned i = 0; i < rsize; ++i) {
// 			_rows2.push_back(tmp[i]);
// 		}
// 	}

// 	// closing rowBegin for the sake of for-loops
// 	_rowBegin.push_back(_rows2.size());
// }

void SMatrix::includeMap(map<unsigned, vector<unsigned> > &rows) {
	_rowCount = rows.size();
	_rowNums.resize(_rowCount);
	_rowBegin.resize(_rowCount+1);
	_count = 0;
	for (auto &e : rows) {
		_count += e.second.size();
	}
	_rows2.resize(_count);
	unsigned p = 0; // current position in _rows2
	unsigned pp = 0; // current position in _rowNums
	_rowBegin[0] = 0;
	for (auto &pair : rows) {
		_rowNums[pp] = pair.first;
		// if (pp) {
		// 	assert(_rowNums[pp] > _rowNums[pp-1]);
		// }
		for (unsigned i = 0; i < pair.second.size(); ++i) {
			_rows2[p++] = pair.second[i];
			// cout << "setting " << pair.first << "," << pair.second[i] << endl;
		}
		_rowBegin[++pp] = p;
	}
}

void SMatrix::storeIn(ostream &os) {
	switch (MODE) {
		case IMPORT: {
			os << "rows:" << _rows->size() << std::endl;
			for (auto &r : (*_rows)) {
				os << r.first << ":" << r.second.size();
				for (unsigned p : r.second) {
					os << ":" << p;
				}
				os << std::endl;
			}
			break;
		}
		case LOAD: {
			os << "rows:" << _rowNums.size() << std::endl;
			for (unsigned i = 0; i < _rowNums.size(); ++i) {
				os << _rowNums[i] << ":" << _rowBegin[i+1] - _rowBegin[i];
				for (unsigned j = _rowBegin[i]; j < _rowBegin[i+1]; ++j) {
					os << ":" << _rows2[j];
				}
				os << std::endl;
			}
		}
		default: {
			break;
		}
	}
}

bool SMatrix::rowNull(unsigned int i) {
	switch (MODE) {
		case IMPORT: {
			return _rows->find(i) == _rows->end();
		}
		case LOAD: {
			return findRow(i) == _rowNums.size();
		}
		default:
			break;
	}
	return true;
}

unsigned SMatrix::findRow(const unsigned r, const unsigned min, const unsigned max) {
	// cout << "find " << r << ":" << min << ":" << max << endl;
	if (max >= min) {
		// cout << "here" << endl;
		const unsigned p = min + (max > 1 ? (max-min) / 2 : 0);
		// cout << "  p = " << p << "  _rowNums[p] = " << _rowNums[p] << endl;
		if (_rowNums[p] == r)
			return p;
		if (_rowNums[p] < r)
			return findRow(r, p+1, max);
		if (_rowNums[p] > r && p)
			return findRow(r, min, p-1);
	}
	return _rowNums.size();
}

void SMatrix::find2(const unsigned a) {
	for (unsigned i = 0; i < _rows2.size(); ++i) {
		if (_rows2[i]==a)
			cout << a << " ";
	}
	cout << endl;
}

// void rowbv(bm::bvector<> &y, bm::bvector<> &Y, unsigned int r) {
// 	switch (MODE) {
// 		case IMPORT: {
// 			if (rowNull(r)) {
// 				return;
// 			}
// 			for (unsigned int &i : (*_rows)[r]) {
// 					Y.set_bit(i);
// 			}
// 			break;
// 		}
// 		case LOAD: {
// 			unsigned i = findRow(r);
// 			if (i == _rowNums.size())
// 				return;
// 			for (unsigned j = _rowBegin[i]; j < _rowBegin[i+1]; ++j) {
// 				Y.set_bit(_rows2[j]);
// 			}
// 			break;
// 		}
// 		default: break;
// 	}
// }

bm::bvector<> SMatrix::multiplyMe(const bm::bvector<> &v) {
        bm::bvector<> result(_max);
        result.reset();
        if (v.none())
            return result;
        switch (MODE) {
            case IMPORT: {
                for (auto &r : (*_rows)) {
                    if (v.test(r.first)) {
                        for (unsigned i : r.second) {
                            result.set(i);
                        }
                    }
                }
                break;
            }
            case LOAD: {
                unsigned i = v.get_first();
                do {
                    unsigned p = findRow(i);
                    if (p != _rowNums.size()) { // row found
                        for (unsigned j = _rowBegin[p]; j < _rowBegin[p+1]; ++j) {
                            // cout << "trying to insert " << j << endl;
                            result.set(_rows2[j]);
                        } 
                    }
                } while (i = v.get_next(i));
                break;
            }
            default: {
                break;
            }
        }
        return result;
    }

void SMatrix::degreeDistribution(std::ostream &os) {
    map<unsigned,unsigned> freq;
    for (unsigned i = 0; i < _rowNums.size(); ++i) {
        freq[_rowBegin[i+1]-_rowBegin[i]] = 0;
    }
    for (unsigned i = 0; i < _rowNums.size(); ++i) {
        ++freq[_rowBegin[i+1]-_rowBegin[i]];
    }
    for (auto &f: freq) {
        os << "," << f.first << ":" << f.second;
    }
}

