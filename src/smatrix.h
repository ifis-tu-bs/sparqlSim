#ifndef SMATRIX_H
#define SMATRIX_H

#include <set>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <cmath>

#include "graph.io.h"

#include "bm.h"
#include "bmserial.h"

class SMatrix {

public:
	typedef std::vector<unsigned int> Row;
	typedef bm::bvector<> bRow;
	typedef std::unordered_map<unsigned int, Row> Map;

private:
	// static const bm::strategy STRATEGY = bm::BM_BIT;
	static const bm::strategy STRATEGY = bm::BM_GAP;
	// static const bm::bvector<>::optmode = bm::bvector<>::opt_free_0;
	// static const bm::bvector<>::optmode = bm::bvector<>::opt_free_01;
	static const bm::bvector<>::optmode OPTMODE = bm::bvector<>::opt_compress;

public:
	SMatrix();
	SMatrix(unsigned int n);
	~SMatrix();

	bool operator()(unsigned int i, unsigned int j) {
		if (rowNull(i) || !_colV.test(j)) /* if row or column is 0 */
			return false;                 /* return false */

		for (auto &k: _rows[i])
			if (k == j) return true;

		return false;
		// return _rows[i].find(j) != _rows[i].end();
	}

	// sets specific coordinate
	void set(unsigned int i, unsigned int j) {
		if (rowNull(i)) {
			_rows[i] = Row();
			_rowCount++;
		}
		_rows[i].push_back(j); // implementation-specific
		// _rows[i].insert(j); // implementation-specific
		_colV.set(j);
		// std::cout << *this << std::endl;

		_count++;
	}

	void optimize() {
		
	}

	void compress(unsigned int max) {
		// std::cout << "compressing max= " << max << std::endl; 
		_max = max;
		_colV.resize(max);
		_colV.optimize_gap_size();
		optimize();
	}

	unsigned int size() const {
		return _rows.size();
	}

	bRow &colV() {
		return _colV;
	}

	const unsigned int colNum() const {
		return _colV.count();
	}

	std::vector<unsigned int> getPairs();

	// unifies Y with r'th row
	void rowbv(bm::bvector<> &y, bm::bvector<> &Y, unsigned int r) {
		if (rowNull(r)) {
			return;
		}
		for (unsigned int &i : _rows[r]) {
				Y.set_bit(i);
		}
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

	void rowString(std::ostream &os) {
		for (auto &pair: _rows) {
			os << endl;
			os << pair.first << " " << pair.second.size();
			for (auto &j: pair.second) {
				os << " " << j;	
			}
		}
	}

	void string2row(std::string row) {

	}

	void deserializeColV(unsigned char *buf) {
		bm::deserialize(_colV, buf);
	}

	double sparsity() const {
		// double result = 0.0;

		// cout << result << endl;
		// if (result == 0.0)
		// 	result = rowSparsity() * colSparsity();

		// cout << result << endl;
		return rowSparsity() * colSparsity();
		// return result;
	}

	double rowSparsity() const {
		// cout << _rowCount << " / " << _max << endl;
		return ( (double) _rowCount / (double) _max );
	}

	double colSparsity() const {
		// cout << _rowCount << " / " << _max << endl;
		return ( (double) _colV.count() / (double) _max );
	}

	void setMax(unsigned int max) {
		_max = max;
	}

	double count() const {
		return (double) _count;
	}

	double icount() const {
		return ((double) _count / _colV.count());
	}

	const bool check(unsigned rownum, const bm::bvector<> &reference) {
		if (!rowNull(rownum)) {
			for (unsigned match : _rows[rownum]) {
				if (reference.test(match))
					return true;
			}
		}

		return false;
	}

private:
	// rows of the matrix
	Map _rows;

	unsigned int _rowCount = 0;

	// max-value for rows and columns
	unsigned int _max;

	// disjunctive column vector
	// Row _colV;
	bRow _colV;

	unsigned int _count = 0;

private:
	bool rowNotNull(unsigned int i) {
		return !rowNull(i);
	}
	inline bool rowNull(unsigned int i) {
		return _rows.find(i) == _rows.end();
	}

	friend std::ostream & operator<<(std::ostream &os, SMatrix &a);

};

#endif /* SMATRIX_H */