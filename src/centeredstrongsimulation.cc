#include "centeredstrongsimulation.h"
#include "variable.h"

#include "graph.h"
#include "label.h"
#include "node.h"
#include "utils.h"

#include "smatrix.h"
#include "reporter.h"

#include <chrono>
#include <cstdlib>
#include <ctime>
using namespace std::chrono;

extern Reporter Karla;
// take args_info for deciding on strategy
#include "cmdline.h"
extern gengetopt_args_info args_info;

/// CONSTRUCTORS & DESTRUCTOR
CenteredStrongSimulation::CenteredStrongSimulation(Graph &DB) :
	StrongSimulation(DB)
{
}
CenteredStrongSimulation::CenteredStrongSimulation(CenteredStrongSimulation &s) :
	StrongSimulation(s)
{
}

CenteredStrongSimulation::CenteredStrongSimulation(CenteredStrongSimulation &s, bm::bvector<> &ball, bm::bvector<> &border) :
	StrongSimulation(s, ball, border)
{
	// do{
	// 	fixpoint(3);
	// 	resolveDependencies();
	// }while(!stable());
}

CenteredStrongSimulation::~CenteredStrongSimulation() {
}

unsigned CenteredStrongSimulation::evaluate(std::ostream &os) {
			
	_reporter.start("fixpoint", _query);
	unsigned iter = fixpoint(3, os);
	_reporter.end("fixpoint", _query);
	_reporter.note("# of iterations", _query, to_string(iter));

	_sim = simulation();
	printHeader(out(), _sim);

	if (empty()) {
		_reporter.note("# of results", _query, to_string(0));
		return 0;
	}

	vector<bm::bvector<> > balls, borders;
	
	int rad = radius();

	bm::bvector<> centers(max());
	std::vector<std::string> queryNode = queryNodeSet();
	std::set<std::string> cen = center(queryNode,rad);
	for (string vname : cen) {
		// cout << vname << endl;
		for (Variable *v : _vars) {
			if (vname == v->getId()) {
				centers |= v->val();
			}
		}
	}

	unsigned result = (rad > 0 ? computeAllStrong(centers, rad) : 0);
	_reporter.note("# of results", _query, to_string(result));
	return result;
}

std::map<std::string, bm::bvector<> > &CenteredStrongSimulation::simulation() {
	if (!_computedSim) {
		for (Variable *var: _sourceV) {
	// cout << var->getId() << endl;
			if (_sim.find(var->getId()) == _sim.end()) {
	// cout << "here!" << endl;
				_sim[var->getId()] = var->unify();
			}
		}
		_computedSim = true;
	}

	return _sim;
}

//StrongSim
std::set<std::string> CenteredStrongSimulation::getPartner(std::string a){
	std::set<std::string> Set;
	
	for (auto &i: _order) {
		//cout << "eq: " << i << endl;
		if(_targetV[i]->getId() == a){
			Set.insert(_sourceV[i]->getId());
			//cout << "Child of " << a << ": " << _sourceV[i]->getId() << endl;
		}
		if(_sourceV[i]->getId() == a){
			Set.insert(_targetV[i]->getId());
			//cout << "Parent of " << a << ": " << _targetV[i]->getId() << endl;
		}
		
	}
	return Set;
}

std::set<std::string> CenteredStrongSimulation::getPartner(std::string a, std::set<std::string> Old){
	std::set<std::string> Set;
	
	for (auto &i: _order) {
		//cout << "eq: " << i << endl;
		if(_targetV[i]->getId() == a && Old.find(_sourceV[i]->getId()) == Old.end() ){
			Set.insert(_sourceV[i]->getId());
			//cout << "Child of " << a << ": " << _sourceV[i]->getId() << endl;
		}
		if(_sourceV[i]->getId() == a && Old.find(_targetV[i]->getId()) == Old.end() ){
			Set.insert(_targetV[i]->getId());
			//cout << "Parent of " << a << ": " << _targetV[i]->getId() << endl;
		}
		
	}
	return Set;
}

std::set<SMatrix *> CenteredStrongSimulation::getEdges(std::string a, std::set<std::string> Old){
	std::set<SMatrix *> Set;
	
	for (auto &i: _order) {
		//cout << "eq: " << i << endl;
		//cout << _operand[i]  << endl;
		if(_targetV[i]->getId() == a  && Old.find(_sourceV[i]->getId()) == Old.end()){
			Set.insert(_operand[i]->matrix_p(_dirs[i])); // mymm.find('y')->second
			//cout << "Child of " << a << ": " << _sourceV[i]->getId() << " with: " << _operand[i] << endl;
		}
		if(_sourceV[i]->getId() == a  && Old.find(_targetV[i]->getId()) == Old.end()){
			Set.insert(_operand[i]->matrix_p(_dirs[i]));
			//cout << "Parent of " << a << ": " << _targetV[i]->getId() << endl;
		}
		
	}
	return Set;
}

std::vector<std::string> CenteredStrongSimulation::queryNodeSet() {
	simulation();
	std::vector<std::string> setOfNodes;
	for (auto &pair: _sim) {
		//cout << "Nodes: " << pair.first << endl;
		setOfNodes.push_back(pair.first);
	}
	return setOfNodes;
}

std::vector<std::vector< std::set<SMatrix *>>>  CenteredStrongSimulation::edgeList(std::set<std::string> center, unsigned int radius ){
	std::vector<std::vector< std::set<SMatrix *>>>  fin;
	for(auto c : center) {

		std::vector< std::set<SMatrix *>>  result;
		std::set<std::string> Old;
		std::set<std::string> Set;
		std::set<std::string> Hold;
		std::set<std::string> Kill;
		std::set<std::string> Next;
		std::set<SMatrix *> HoldEdges;
		std::set<SMatrix *> Edges;
		Next.insert(c);
		Old.insert(c);
		for(int i = 0; i < radius; i++){
			Edges.clear();
			Set = Next;
			while(!Set.empty()){
				std::string a = *Set.begin();
				Set.erase(a);
				HoldEdges = getEdges(a, Old);
				while(!HoldEdges.empty()){
					SMatrix * c = *HoldEdges.begin();
					HoldEdges.erase(c);
					Edges.insert(c);
				}
				Hold = getPartner(a,Old);
				while(!Hold.empty()){
					std::string b = *Hold.begin();
					Hold.erase(b);
					Kill.insert(b);
				}
			}
			while(!Kill.empty()){
				std::string b = *Kill.begin();
				Kill.erase(b);
				Next.insert(b);
				Old.insert(b);
			}
			result.push_back(Edges);
		}
		fin.push_back(result);
	}
	return fin;
}

void CenteredStrongSimulation::centerList(bm::bvector<> &sum, const std::set<std::string> &center) {
	for (auto &pair: _sim) {
		//cout << "PC:" << pair.second << "F:" << pair.first << " SUM:" << sum << endl;
		if(center.count(pair.first)){
			sum |= pair.second;
		}
		//cout << "PC:" << pair.second << " SUM:" << sum << endl;
	}
}

void CenteredStrongSimulation::centerList(bm::bvector<> &sum, const std::string &centerNode) {
	for (auto &pair: _sim) {
		//cout << "PC:" << pair.second << "F:" << pair.first << " SUM:" << sum << endl;
		if(pair.first == centerNode){
			sum |= pair.second;
		}
		//cout << "PC:" << pair.second << " SUM:" << sum << endl;
	}
}

bm::bvector<> CenteredStrongSimulation::simList() {
	simulation();
	bm::bvector<> sum;
	for (auto &pair: _sim) {
		//cout << "PC:" << pair.second << "F:" << pair.first << " SUM:" << sum << endl;
		sum |= pair.second;
		//cout << "PC:" << pair.second << " SUM:" << sum << endl;
	}
	return sum;
}

// unsigned int CenteredStrongSimulation::diameter(std::vector<std::string> nodes){
// 	unsigned int diameter = 0;
// 	std::set<std::string> Set;
// 	std::set<std::string> Red;
// 	std::set<std::string> Blue;
// 	std::set<std::string> Partner;
// 	for(auto& it : nodes) {
// 		int dia = 0;
// 		Red.clear();Set.clear();Blue.clear();
//     	//std::cout<< "IT: " << it.getName() << endl;
// 		Set.insert(it);
// 		Red.insert(it);
// 		while(!Red.empty()){
// 			while(!Red.empty()){
// 				std::string a = *Red.begin();
// 				Red.erase(a);
// 				Partner = getPartner(a);
// 				if(Partner.empty()){
// 					dia--;
// 					break;
// 				}
// 				while(!Partner.empty()){
// 					std::string b = *Partner.begin();
// 					Partner.erase(b);
// 					if (!Set.count(b)) {
// 						Blue.insert(b);
// 						Set.insert(b);
// 					}
// 				}
// 			}
// 			if(!Blue.empty()){
// 					dia++;
// 				}
// 			Red.swap(Blue);
// 		}
// 		if(dia > diameter){
// 			diameter = dia;
// 		}
// 	}
// 	return diameter;
// }

// unsigned int CenteredStrongSimulation::radius(std::vector<std::string> nodes){
// 	unsigned int diameter = 99;
// 	std::set<std::string> Set;
// 	std::set<std::string> Center;
// 	std::set<std::string> Red;
// 	std::set<std::string> Blue;
// 	std::set<std::string> Partner;
// 	for(auto& it : nodes) {
// 		int dia = 0;
// 		Red.clear();Set.clear();Blue.clear();
//     	//std::cout<< "IT: " << it.getName() << endl;
// 		Set.insert(it);
// 		Red.insert(it);
// 		while(!Red.empty()){
// 			while(!Red.empty()){
// 				std::string a = *Red.begin();
// 				Red.erase(a);
// 				Partner = getPartner(a);
// 				if(Partner.empty()){
// 					dia--;
// 					break;
// 				}
// 				while(!Partner.empty()){
// 					std::string b = *Partner.begin();
// 					Partner.erase(b);
// 					if (!Set.count(b)) {
// 						Blue.insert(b);
// 						Set.insert(b);
// 					}
// 				}
// 			}
// 			if(!Blue.empty()){
// 					dia++;
// 				}
// 			Red.swap(Blue);
// 		}
// 		if(dia < diameter){
// 			diameter = dia;
// 			Center.clear();
// 			Center.insert(it);
// 		}else if(dia == diameter){
// 			Center.insert(it);
// 		}
// 	}
// 	return diameter;
// }

std::set<std::string> CenteredStrongSimulation::center(std::vector<std::string> nodes,unsigned int radius){
	unsigned int diameter = radius;
	std::set<std::string> Set;
	std::set<std::string> Center;
	std::set<std::string> Red;
	std::set<std::string> Blue;
	std::set<std::string> Partner;
	for(auto& it : nodes) {
		int dia = 0;
		Red.clear();Set.clear();Blue.clear();
    	//std::cout<< "IT: " << it.getName() << endl;
		Set.insert(it);
		Red.insert(it);
		while(!Red.empty()){
			while(!Red.empty()){
				std::string a = *Red.begin();
				Red.erase(a);
				Partner = getPartner(a);
				if(Partner.empty()){
					dia--;
					break;
				}
				while(!Partner.empty()){
					std::string b = *Partner.begin();
					Partner.erase(b);
					if (!Set.count(b)) {
						Blue.insert(b);
						Set.insert(b);
					}
				}
			}
			if(!Blue.empty()){
					dia++;
				}
			Red.swap(Blue);
		}
		if(dia < diameter){
			diameter = dia;
			Center.clear();
			Center.insert(it);
		}else if(dia == diameter){
			Center.insert(it);
		}
	}
	return Center;
}


std::string CenteredStrongSimulation::output(string delimiter) {
	simulation();
	std::string out;	
	std::string split = "#|#";
	for (auto &pair: _sim) {
		std::string tmp;
		unsigned int v = pair.second.get_first();
		if (empty() || (v == 0 && !pair.second.test(v))) {
			out += delimiter;
			continue;
		}
		do {
			tmp += _db->getNodeName(v) + "" + split;
			
		} while ((v = pair.second.get_next(v)) != 0);
		tmp = tmp.substr(0, tmp.size()-split.length());
		out += tmp + delimiter;
	}
	return out;
}

std::string CenteredStrongSimulation::var() {
	simulation();
	std::string var;

	for (auto &pair: _sim) {
		//cout << pair.first << "[]:";
		var += pair.first + "#||#";
	}
	return var;
}

void CenteredStrongSimulation::setFullyUnstable() {
	for(auto &i: _order){
		_stable[i] = false;
	}
}

void CenteredStrongSimulation::setOutput(const string &filename) {
	_filename = filename + ".centered.out";
	setOutputStream(*new ofstream());
	_fileout = true;
}

void CenteredStrongSimulation::statistics(const std::string &filename) {
	ofstream stats;
	stats.open(filename+".statistics", ofstream::app);
	if (!checkStream(filename+".statistics", stats)) 
		return;

	// take Karla's info
	overtakeKarla(filename);

	// output statistics
	statistics(stats);

	stats.close();
}

void CenteredStrongSimulation::statistics(std::ostream &os) {
	_reporter(os);
	_reporter.report();
}

void CenteredStrongSimulation::csv(const std::string &filename, const char delim) {
	ofstream csv_f;
	_reporter.note("filename", _query, filename);

	csv_f.open(filename+".centered.csv", ofstream::app);
	if (!checkStream(filename+".centered.csv", csv_f)) 
		return;

	overtakeKarla(filename);
	csv(csv_f, delim);

	csv_f.close();
}

void CenteredStrongSimulation::csv(std::ostream &os, const char delim) {
	os << "<<<";

	os << delim << _reporter.getValue("filename", _query);
	
	os << delim << _reporter.getValue("compilation time", _query);
	// os << delim << _reporter.getValue("fixpoint", _query);
	// os << delim << _reporter.getValue("# of iterations", _query);
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
	os << delim << diameter();

	bm::bvector<> balls;
	for (Variable *v : _vars) {
		balls |= v->val();
	}
	os << delim << balls.count();
	// os << delim << _reporter.getValue("average ball size", _query);
	os << delim << _reporter.getValue("thread number", _query);

	os  << delim << ">>>" << endl;
}

/*unsigned int CenteredStrongSimulation::fixpointY() {
	setFullyUnstable();
	unsigned int iter = 0;
	while (!stable()) {
		iter++;
		for (auto &i: _order) {
			// if current equation is already stable, continue
			if (_stable[i])
				continue;

			// get source variable (including an update)
			// y <= x \times A
			bm::bvector<> &x = _sourceV[i]->getVal();
			bm::bvector<> &y = _targetV[i]->getVal();
			//cout << "computing ";
			cout<< "Eq: " << i << " = " << _targetV[i]->getId() << " <= " << _sourceV[i]->getId() << " x " << _operand[i] << endl;
			cout << _targetV[i]->getVal() << " <= " << _sourceV[i]->getVal() << " x " << _operand[i] << endl;

			bm::bvector<> Y(max());
			unsigned int r = x.get_first(); // position of first 1
			if (r==0 && !x.test(0)) {
				_targetV[i]->null(); // if x is 0, then y is
				_stable[i] = true;
				if (_targetV[i]->isMandatory()) {
					_empty = true;
					return iter;
				}
				else {
					continue;
				}
			} else {
				do {
					_operand[i]->matrix(_dirs[i]).rowbv(y, Y, r);
				} while ((r=x.get_next(r)) != 0);
			}
			cout << "IN Target: " << _targetV[i]->getVal() << endl;
			_targetV[i]->update(Y);
			cout << "Out Target: " << _targetV[i]->getVal() << endl;
			_stable[i] = true;

			if (_targetV[i]->isEmpty() && 
				_targetV[i]->isMandatory()) {
				_empty = true;
				return iter;

			}
		}
	}

	bool changes = false;
	for (unsigned int i = 0; i < _vars.size(); ++i) {
		if (_vars[i]->updVal()) {
			changes = true;
		}
	}

	if (changes) {
		return fixpoint() + iter;
	}
	return iter;
}*/

const int CenteredStrongSimulation::radius() {
	if (diameter() < 0) {
		return -1;
	}
	if (diameter() % 2) {
		return (diameter() / 2) + 1;
	}
	return diameter() / 2;
}
