#ifndef LABEL_H
#define LABEL_H

#include <iostream>
#include <ostream>
#include <string>
#include <unordered_map>

using namespace std;

#include "graph.io.h"
#include "smatrix.h"

class Label {
public:
	Label(const string &l = "");
	Label(const unsigned &dim, const string &l = "");
	~Label();

	const bool operator==(const string &l) const { return l == _me; }
	const bool operator==(const Label &l) const { return l._me == _me; }
	const bool operator!=(const string &l) const { return l != _me; }
	const bool operator!=(const Label &l) const { return l._me != _me; }

	const string & str() const { return _me; }
	void nostr() { _me = ""; }

	/// sets coordinates in the matrices
	void set(unsigned int ro, unsigned int co);
	void set2(unsigned int ro, unsigned int co);

	void makeFinal();

	/// replaces the forward matrix by a new one
	void setA(SMatrix *a) {
		if (_a) {
			delete _a;
		}
		_a = a;
	}

	/// replaces the backward matrix by a new one
	void setAT(SMatrix *aT) {
		if (_aT) {
			delete _aT;
		}
		_aT = aT;
	}

	void compress(unsigned int max) {
		optimize(max);
	}

	bool operator()(const unsigned int row, const unsigned int col);

	void setAColV(unsigned char *buf) {
		_a->deserializeColV(buf);
	}

	void setATColV(unsigned char *buf) {
		_aT->deserializeColV(buf);
	}

	SMatrix &a() {
		return *_a;
	}

	SMatrix *a_p() {
		return _a;
	}

	// void addARow(const string &buf) {
	// 	_a->string2row(buf);
	// }

	SMatrix &aT() {
		return *_aT;
	}

	SMatrix *aT_p() {
		return _aT;
	}

	SMatrix &matrix(const bool &forwards) {
		return (forwards ? *_a : *_aT);
	}

	SMatrix *matrix_p(const bool &forwards) {
		return (forwards ? _a : _aT);
	}

	// void addATRow(const string &buf) {
	// 	_aT->string2row(buf);
	// }

	void swap() {
		SMatrix *tmp = _a;
		_a = _aT;
		_aT = tmp;
	}

	void optimize(unsigned int max) {
		_a->compress(max);
		_aT->compress(max);
	}

	vector<unsigned int> getPairs() {
		return _a->getPairs();
	}

	void updateMatrices(unsigned int n) {
		_a->setMax(n);
		_aT->setMax(n);
	}

	string sizeOf(unsigned &size) {
		stringstream s;
		unsigned as, ats;

		s << "    " << _me << endl;
		s << "    a:  " << (as = _a->sizeOf()) << endl;
		s << "    aT: " << (ats = _aT->sizeOf()) << endl;

		size = as + ats + _me.size()*sizeof(char);

		return s.str();
	}

	unsigned countTriples() {
		return _a->countBits();
	}

	bm::bvector<> unionRow(const unsigned ro);

private:
	string _me;

	// adjacency matrix
	SMatrix *_a;
	// transposed _a
	SMatrix *_aT;

	friend ostream &operator<<(ostream &os, Label &l);
};

#endif /* LABEL_H */
