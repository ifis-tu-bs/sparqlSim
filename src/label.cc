#include "label.h"
#include "smatrix.h"

Label::Label(const string &l) : 
	_me(l), 
	_a(new SMatrix()), 
	_aT(new SMatrix()) 
{

}

Label::Label(const unsigned &dim, const string &l) :
	_me(l),
	_a(new SMatrix(dim)),
	_aT(new SMatrix(dim))
{

}

Label::~Label() {
	if (_a)
		delete _a;
	if (_aT)
		delete _aT;

	_a = _aT = NULL;
}

void Label::set(unsigned int ro, unsigned int co) {
	// cout << "here" << endl;
	// cout << _a << " " << _aT << endl;
	_a->set(ro,co);
	_aT->set(co,ro);
}

void Label::set2(unsigned int ro, unsigned int co) {
	// cout << "here" << endl;
	// cout << _a << " " << _aT << endl;
	_a->set2(ro,co);
	// _aT->set2(co,ro, final);
	// if (final) {
	// 	setAT(new SMatrix(*_a, true));
	// }
}

void Label::makeFinal() {
	_a->makeFinal();
	setAT(new SMatrix(*_a, true));
}

bool Label::operator()(const unsigned int row, const unsigned int col) {
	// return a()[row][col];
	return (*_a)(row, col);
}

bm::bvector<> Label::unionRow(const unsigned ro) {
	return _a->rowBV(ro) | _aT->rowBV(ro);
}
