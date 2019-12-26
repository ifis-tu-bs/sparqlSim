#ifndef SMATRIX_H
#define SMATRIX_H

#include <array>
#include <set>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <cmath>
#include <iostream>
#include <fstream>
#include <map>

#include "graph.io.h"

#include "bm.h"
#include "bmserial.h"

class SMatrix {

private:
	// static const bm::strategy STRATEGY = bm::BM_BIT;
	static const bm::strategy STRATEGY = bm::BM_GAP;
	// static const bm::bvector<>::optmode = bm::bvector<>::opt_free_0;
	// static const bm::bvector<>::optmode = bm::bvector<>::opt_free_01;
	static const bm::bvector<>::optmode OPTMODE = bm::bvector<>::opt_compress;

	unsigned _lastRow = 0;
	bool _LastRow = false;

public:
	void find2(const unsigned a);

	SMatrix();
	SMatrix(unsigned int n);
	SMatrix(const SMatrix &s, const bool transpose = false);
	~SMatrix();

	bool operator()(unsigned int i, unsigned int j);

	// sets specific coordinate
	void set(unsigned int i, unsigned int j);
	void set2(unsigned int i, unsigned int j);

	void makeFinal();

	void optimize() {
		if (MODE == IMPORT) {
			includeMap(*_rows);
			delete _rows;
			_rows = NULL;

			MODE = LOAD;

			_count = _rows2.size();
		}
	}

	void loadMode() {
		MODE = LOAD;
	}

	void compress(unsigned int max) {
		// std::cout << "compressing max= " << max << std::std::endl;
		_max = max;
		_colV.resize(max);
		_colV.optimize_gap_size();
		optimize();
	}

	unsigned int size() const {
		if (MODE == IMPORT)
			return _rows->size();
		return _rowNums.size();
	}

	bm::bvector<> &colV() {
		return _colV;
	}

	const unsigned int colNum() const {
		return _colV.count();
	}

	std::vector<unsigned int> getPairs();

	// unifies Y with r'th row
	void rowbv(bm::bvector<> &y, bm::bvector<> &Y, unsigned int r) {
		switch (MODE) {
			case IMPORT: {
				if (rowNull(r)) {
					return;
				}
				for (unsigned int &i : (*_rows)[r]) {
						Y.set_bit(i);
				}
				break;
			}
			case LOAD: {
				unsigned i = findRow(r);
				if (i == _rowNums.size())
					return;
				for (unsigned j = _rowBegin[i]; j < _rowBegin[i+1]; ++j) {
					Y.set_bit(_rows2[j]);
				}
				break;
			}
			default: break;
		}
	}

	bm::bvector<> rowBV(unsigned r) {
		bm::bvector<> result(_max);
		switch (MODE) {
			case IMPORT: {
				for (unsigned int i : (*_rows)[r]) {
					result.set_bit(i);
				}
				break;
			}
			case LOAD: {
				// cout << "find " << r << endl;
				unsigned i = findRow(r);
				// cout << "found " << i << endl;
				if (i == _rowNums.size())
					break;
				// cout << "sane" << endl;
				for (unsigned j = _rowBegin[i]; j < _rowBegin[i+1]; ++j) {
					result.set_bit(_rows2[j]);
				}
				break;
			}
			default: break;
		}
		return result;
	}

	unsigned char *serializedColV(unsigned int &size) {
		BM_DECLARE_TEMP_BLOCK(tb);
		bm::bvector<>::statistics st;

		_colV.optimize(tb, OPTMODE, &st);

		unsigned char *buf = new unsigned char[st.max_serialize_mem];

		bm::serializer<bm::bvector<> > ser;
		ser.set_compression_level(4);
		size = ser.serialize(_colV, buf, st.max_serialize_mem);

		return buf;
	}

	// void rowString(std::ostream &os) {
	// 	for (auto &pair: _rows) {
	// 		os << std::endl;
	// 		os << pair.first << " " << pair.second.size();
	// 		for (auto &j: pair.second) {
	// 			os << " " << j;
	// 		}
	// 	}
	// }

	void deserializeColV(unsigned char *buf) {
		bm::deserialize(_colV, buf);
	}

	double sparsity() const {
		// double result = 0.0;

		// cout << result << std::endl;
		// if (result == 0.0)
		// 	result = rowSparsity() * colSparsity();

		// cout << result << std::endl;
		return rowSparsity() * colSparsity();
		// return result;
	}

	double rowSparsity() const {
		// cout << _rowCount << " / " << _max << std::endl;
		return ( (double) _rowCount / (double) _max );
	}

	double colSparsity() const {
		// cout << _rowCount << " / " << _max << std::endl;
		return ( (double) _colV.count() / (double) _max );
	}

	void setMax(unsigned int max) {
		_max = max;
	}

	const unsigned intcount() const {
		return _rows2.size();
	}

	double count() const {
		return (double) _rows2.size();
	}

	unsigned countBits() {
		return _rows2.size();
	}

	double icount() const {
		return ((double) _rows2.size() / _colV.count());
	}

	// checks if conjunction is non-empty
	const bool check(unsigned rownum, const bm::bvector<> &reference) {
		// switch (MODE) {
		// 	case IMPORT: {
		// 		if (!rowNull(rownum)) {
		// 			for (unsigned match : (*_rows)[rownum]) {
		// 				if (reference.test(match))
		// 					return true;
		// 			}
		// 		}
		// 		break;
		// 	}
			// case LOAD: {
				unsigned i = findRow(rownum);
				if (i < _rowNums.size()) {
					for (unsigned j = _rowBegin[i]; j < _rowBegin[i+1]; ++j) {
						if (reference.test(_rows2[j]))
							return true;
					}
				}
		// 		break;
		// 	}
		// 	default: break;
		// }

		return false;
	}

    const unsigned checkMult(const unsigned rownum, const bm::bvector<> &reference) {
        unsigned r = 0;

        unsigned i = findRow(rownum);
        if (i < _rowNums.size()) {
            for (unsigned j = _rowBegin[i]; j < _rowBegin[i+1]; ++j) {
                r += (reference.test(_rows2[j]) ? 1 : 0);
            }
        }

        return r;
    }

	bm::bvector<> multiplyMe(const bm::bvector<> &v);

	static void print(const bm::bvector<> &v) {
		cerr << "size: " << v.size() << std::endl;
		cerr << "[";
		unsigned c = v.get_first();
		if (c || v.test(0)) {
			do {
				cerr << " " << c;
			} while (c = v.get_next(c));
		}
		cerr << " ]" << std::endl;
	}

	unsigned sizeOf() {
		unsigned size = sizeof(map<unsigned,vector<unsigned> >);
		for (auto &e : (*_rows)) {
			size += sizeof(unsigned) +
					sizeof(vector<unsigned>) +
					e.second.size()*sizeof(unsigned);
		}
		return size + sizeof(_colV) + sizeof(SMatrix);
	}

	/// stores the matrix in the given stream
	void storeIn(std::ostream &os);

	/// loads row from stored string format
	// void loadRow(const std::string &line);

	/// include the data from the given map
	void includeMap(std::map<unsigned, std::vector<unsigned> > &rows);

	void updateNeighbors(const unsigned node, bm::bvector<> &neigh, bm::bvector<> &known) {
		unsigned i = findRow(node);
		if (i < _rowNums.size()) {
			for (unsigned j = _rowBegin[i]; j < _rowBegin[i+1]; ++j) {
				if (!known.test(_rows2[j])) {
					neigh.set_bit(_rows2[j]);
					known.set_bit(_rows2[j]);
				}
			}
		}
	}

    void degreeDistribution(std::ostream &os);

private:
	// rows of the matrix
	// map<unsigned, Row> .. Row=vector<unsigned>
	std::map<unsigned, std::vector<unsigned> > *_rows;

	// unsigned* _rowNums;
	std::vector<unsigned> _rowNums;
	// unsigned* _rowBegin;
	std::vector<unsigned> _rowBegin;
	// rowSize
	std::vector<unsigned short> _rowSize;
	// unsigned* _rows2;
	std::vector<unsigned> _rows2;

	unsigned int _rowCount = 0; // number of entries (_rowNums)
	unsigned int _count = 0; // number of entries (_row2)

	// max-value for rows and columns
	unsigned int _max;

	// row-disjunctive column vector
	// Row _colV;
	bm::bvector<> _colV;

	/// switch import/load mode
	enum { IMPORT, LOAD } MODE;

private:
	bool rowNotNull(unsigned int i) {
		return !rowNull(i);
	}
	bool rowNull(unsigned int i);

    unsigned _lastHit = 0;

	unsigned findRow(const unsigned r) {
        unsigned result = (_rowNums[_lastHit] <= r ?
            findRow(r, _lastHit, _rowNums.size()-1) : findRow(r, 0, _rowNums.size()-1));

        if (result < _rowNums.size())
            _lastHit = result;

		return result;
	}

	unsigned findRow(const unsigned r, const unsigned min, const unsigned max);

	friend std::ostream & operator<<(std::ostream &os, SMatrix &a);

};

#endif /* SMATRIX_H */
