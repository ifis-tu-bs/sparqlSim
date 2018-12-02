#include "strongsimulation.h"
#include "simulation.h"
#include "variable.h"

#include "graph.h"
#include "label.h"
#include "node.h"
#include "utils.h"

#include "smatrix.h"

// take args_info for deciding on strategy
#include "cmdline.h"
extern gengetopt_args_info args_info;

// multi-threading
#include <functional>
#include <thread>
#include <chrono>

#include <array>

using namespace std;

/// CONSTRUCTORS & DESTRUCTOR
StrongSimulation::StrongSimulation(Graph &DB) :
	QGSimulation(DB)
{
}

StrongSimulation::StrongSimulation(StrongSimulation &s) :
	QGSimulation(s)
{
}

StrongSimulation::StrongSimulation(StrongSimulation &s, bm::bvector<> &ball, bm::bvector<> &border) :
	QGSimulation(s)
{	
	for (Variable *v : _vars) {
		v->join(ball);
		bm::bvector<> tmp = v->val() & border;
		if (tmp.any()) {
			v->propagate();
		}
	}

	// recompute fixpoint until all dependencies resolved
	do{
		fixpoint(3);
		resolveDependencies();
	}while(!stable());
}

StrongSimulation::~StrongSimulation() {
}

// void StrongSimulation::operator=(StrongSimulation &ref) {
// 	this = &ref;
// }

unsigned StrongSimulation::evaluate(std::ostream &os) {
	
	if (!diameter()) {
		_reporter.note("# of results",_query,to_string(0));
		return 0;
	}
	if (diameter() < 0) {
		// reporter.note("# of results",_query,to_string(0));
		cerr << "error in eval(" 
			<< _query 
			<< "): query with infinite diameter" 
			<< endl;
		return 0;
	}

	// _reporter.start("fixpoint", _query);
	unsigned iter = fixpoint(3, os);
	// _reporter.end("fixpoint", _query);
	// _reporter.note("# of iterations", _query, to_string(iter));
	// cout << "pruning in " << iter << " iterations" << endl;

	_sim = simulation();
	printHeader(out(), _sim);

	if (empty()) {
		_reporter.note("# of results", _query, to_string(0));
		return 0;
	}

	_unionsim.resize(max());
	_unionsim.reset();
	for (Variable *v : _vars) {
		_unionsim |= v->val();
	}

	unsigned result = computeAllStrong(_unionsim, diameter());
	_reporter.note("# of results", _query, to_string(result));
	return result;
}

const unsigned StrongSimulation::computeAllStrong(bm::bvector<> &balls, const unsigned radius) {
	if (!balls.size())
		return 0;

	cout << "#balls: " << balls.count() << " (r=" << radius << ")" << endl;

	unsigned result = 0;
	unsigned threadsNum = 
		(args_info.threads_arg < balls.size() ? 
			args_info.threads_arg : balls.size()) - 1;
	_reporter.note("thread number", _query, to_string(threadsNum+1));

	thread *threads;
	unsigned *res;
	// vector<bm::bvector<> > *N_copies;
	vector<StrongSimulation *> sim_copies;
	vector<bm::bvector<> > allballs;
	
	if (threadsNum) {
		threads = new thread[threadsNum];
		res = new unsigned[threadsNum];

		sim_copies.resize(threadsNum);
		allballs.resize(threadsNum);
	}

	unsigned workload = balls.count() / (threadsNum+1);

	unsigned last = balls.get_first();

	// printTime();
	stringstream fname;
	// cout << threadsNum << " vs. " << balls.count() << endl;
	// cout << workload << endl;
	for (unsigned i = 0; i < threadsNum && balls.any(); ++i) {
		res[i] = 0;
		// printTime();
		sim_copies[i] = new StrongSimulation(*this);

		if (_fileout) {
			fname << _filename << "_" << i;
			sim_copies[i]->_filename = fname.str();
			sim_copies[i]->_out = new ofstream();
			sim_copies[i]->_fileout = true;
			sim_copies[i]->_outputDone = false;
			fname.str("");
		}

		allballs[i] = bm::bvector<>(max());
		unsigned j = 0;
		for (unsigned j = 0; j < workload && 
			(last = balls.extract_next(last)); ++j) {
			// cout << i << ":" << last << endl;
			allballs[i].set(last);
			// cout << i << ":" << allballs[i].count() << endl;
			// if (!(last= balls.extract_next(last))) {
			// 	break;
			// }
		}
		// printTime();
		threads[i] = thread(strongsimulation, i, ref(allballs[i]), ref(_db->neighbors()), radius, ref(res[i]), ref(*sim_copies[i]));
	}
	// printTime();
	// cout << balls.count() << endl;

	// _db->neighbors().find2(27225568);
	
	if (balls.any()) {
		result += strongsimulation0(threadsNum, balls, _db->neighbors(), radius, *this);
	}


	// printTime();
	for (unsigned i = 0; i < threadsNum; ++i) {
		threads[i].join();
		result += res[i];
		// avg_ballsize[threadsNum] += avg_ballsize[i];
	}
	// printTime();

	// _reporter.note("average ball size", _query, to_string(avg_ballsize[threadsNum]/(threadsNum+1)));

	return result;
}

void strongsimulation(const unsigned tid, bm::bvector<> &balls, 
		SMatrix &N, 
		const unsigned radius, unsigned &res, StrongSimulation &sim) {
	
	// printTime();
	unsigned result = strongsimulation0(tid, balls, N, radius, sim);
	// cout << result << endl;
	res += result;
}

const unsigned strongsimulation0(const unsigned tid, bm::bvector<> &balls,
	SMatrix &N, const unsigned radius, 
	StrongSimulation &sim) {

	StrongSimulation &tim = *(new StrongSimulation(sim));
	// bm::bvector<> &ball = *(new bm::bvector<>());
	// bm::bvector<> &border = *(new bm::bvector<>());
	// bm::bvector<> &border1 = *(new bm::bvector<>());
	bm::bvector<> ball;
	bm::bvector<> border;
	bm::bvector<> border1;
	unsigned result = 0;
	// printTime();
	unsigned &radius2 = *(new unsigned);
	radius2 = radius;

	// unsigned ballnum = balls.count();
	if (sim._fileout) {
		((ofstream *)sim._out)->open(sim._filename);		
	}

	// unsigned current = candidates.get_first();
	// // set current to first
	// for (unsigned i = 0; i < tid * workload; ++i) {
	// 	current = candidates.get_next(current);
	// }
	// unsigned iteration = workload;
	// // cout << workload << endl;
	// while (iteration) {
	// 	// cout << iteration << endl;
	// 	--iteration;
	// auto n = N.cbegin();
	unsigned current = balls.get_first();
	// cout << balls.count() << endl;
	// for (unsigned i = tid * workload; i < (tid+1) * workload; ++i) {
	do {
		// printTime();
		// cout << current << endl;
		ball.set(current);
		border.set(current);

		// build the ball
		// inflate(ref(ball), ref(border), ref(g), radius);
		// cout << tid << "before: ";
		for (unsigned r = 0; r < radius2; ++r) {
			// cout << ball << endl;
			border1.reset();
			unsigned ro = border.get_first();
			do {
				// cout << "b: " << ro << "(" << current << ")" << endl;
				for (Label *l : sim._Labels)
					border1 |= l->unionRow(ro);
				// border1 |= N.rowBV(ro) - ball;
				border1 -= ball;
				// assert(!border1.test(27225568));
				// cout << "a" << endl;
			} while (ro = border.get_next(ro));
			if (border1.none()) {
				break;
			}
			ball |= border1;
			border.swap(border1);
			// cout << border << endl;
		}
		// avg_ballsize += ball.count();
		// set up tim
		tim.set(sim, ball, border);

		// compute fixpoint
		do {
			tim.fixpoint(3);
			tim.resolveDependencies();
		} while (!tim.stable());

		if (!tim.empty()) {
			if (args_info.output_given)
				(*sim._out) << tim.simulation() << endl;
			++result;
		}

		ball.reset();
		border.reset();

	} while (current = balls.get_next(current));

	delete &tim;
	// delete &ball;
	// delete &border;
	// delete &border1;
	delete &radius2;

	if (sim._fileout) {
		((ofstream *) sim._out)->close();
	}

	// avg_ballsize /= ballnum;

	return result;
}

// const unsigned StrongSimulation::computeAllStrong(vector<bm::bvector<> > &balls, vector<bm::bvector<> > &borders) {
// 	unsigned result = 0;

// 	unsigned threadsNum = args_info.threads_arg;
// 	thread threads[threadsNum];
// 	unsigned res[threadsNum];
// 	unsigned workload = balls.size() / threadsNum;
// 	// unsigned it;
// 	// unsigned counter = 0;
// 	// bm::bvector<> ball(_max);
// 	// bm::bvector<> border(_max);
// 	// vector<StrongSimulation *> tim;
// 	for (unsigned i = 0; i < threadsNum; ++i) {
// 		res[i] = 0;
// 		// tim.push_back(new StrongSimulation(*this));
// 	// }
	
// 	// _reporter.start("computing simulations",_query);
// 	// for (unsigned i = 0; i < threadsNum; ++i) {
// 		threads[i] = thread(strongsimulation0, i, i * workload, (i+1)*workload, ref(balls), ref(borders), ref(*this), ref(res[i]));
// 	}

// 	threads[0].join();
// 	threads[0] = thread(strongsimulation0, 0, threadsNum * workload, balls.size(), ref(balls), ref(borders), ref(*this), ref(res[0]));
// 	for (unsigned i = 0; i < threadsNum; ++i) {
// 		threads[i].join();
// 		result += res[i];
// 	}

// 	return result;
// }


// // void strongsimulation(const unsigned pid, bm::bvector<> &balls, const unsigned first, unsigned &prev, Graph &g, const unsigned radius, bm::bvector<> &result, unsigned &res, StrongSimulation &sim) {
// void strongsimulation(const unsigned pid, bm::bvector<> balls, const unsigned first, unsigned workload, vector<bm::bvector<> > N, const unsigned radius, unsigned &res, StrongSimulation *sim) {
// 	cout << pid << ":" << &balls << "(" << balls.count() << "):" << first << workload << ":" << &N << "(size:" << N.size() << "):" << radius << ":" << res << ":" << sim << endl;
// 	return;


// 	StrongSimulation tim(*sim);
// 	bm::bvector<> ball, border;
// 	unsigned result = 0;

// 	unsigned current = first;
// 	unsigned iteration = workload;
// 	while (iteration--) {

// 	// }
// 	// do {
// 		// if (!(rand() % 10)) {
// 		// 	prev = current;
// 		// }

// 		// if (!(rand() % 10))
// 			// cout << pid << ": ping" << endl;

// 		ball.set(current);
// 		border.set(current);

// 		// build the ball
// 		// inflate(ref(ball), ref(border), ref(g), radius);
// 		bm::bvector<> border1;
// 		for (unsigned r = 0; r < radius; ++r) {
// 			border1.reset();
// 			unsigned ro = border.get_first();
// 			do {
// 				border1 |= N[ro] - ball;
// 			} while (ro = border.get_next(ro));
// 			if (border1.none()) {
// 				break;
// 			}
// 			ball |= border1;
// 			border.swap(border1);
// 		}

// 		// set up tim
// 		tim.set(*sim, ball, border);

// 		// compute fixpoint
// 		do {
// 			tim.fixpoint(3);
// 			tim.resolveDependencies();
// 		} while (!tim.stable());

// 		if (!tim.empty()) {
// 			// if (args_info.output_given)
// 			// 	sim.out() << tim.simulation() << endl;
// 			// result.set(current);
// 			++result;
// 		}

// 		ball.reset();
// 		border.reset();
// 		current = balls.get_next(current);
// 	}
// }

// void strongsimulation0(const unsigned pid, bm::bvector<> balls, const unsigned first, unsigned workload, vector<bm::bvector<> > N, const unsigned radius, unsigned &res, StrongSimulation *sim) {
// 	cout << pid << ":" << &balls << "(" << balls.count() << "):" << first << workload << ":" << &N << "(size:" << N.size() << "):" << radius << ":" << res << ":" << sim << endl;
// 	return;


// 	StrongSimulation tim(*sim);
// 	bm::bvector<> ball, border;
// 	unsigned result = 0;

// 	unsigned current = first;
// 	unsigned iteration = workload;
// 	while (iteration--) {

// 	// }
// 	// do {
// 		// if (!(rand() % 10)) {
// 		// 	prev = current;
// 		// }

// 		// if (!(rand() % 10))
// 			// cout << pid << ": ping" << endl;

// 		ball.set(current);
// 		border.set(current);

// 		// build the ball
// 		// inflate(ref(ball), ref(border), ref(g), radius);
// 		bm::bvector<> border1;
// 		for (unsigned r = 0; r < radius; ++r) {
// 			border1.reset();
// 			unsigned ro = border.get_first();
// 			do {
// 				border1 |= N[ro] - ball;
// 			} while (ro = border.get_next(ro));
// 			if (border1.none()) {
// 				break;
// 			}
// 			ball |= border1;
// 			border.swap(border1);
// 		}

// 		// set up tim
// 		tim.set(*sim, ball, border);

// 		// compute fixpoint
// 		do {
// 			tim.fixpoint(3);
// 			tim.resolveDependencies();
// 		} while (!tim.stable());

// 		if (!tim.empty()) {
// 			// if (args_info.output_given)
// 			// 	sim.out() << tim.simulation() << endl;
// 			// result.set(current);
// 			++result;
// 		}

// 		ball.reset();
// 		border.reset();
// 		current = balls.get_next(current);
// 	}
// }

// void strongsimulation(const unsigned pid, vector<bm::bvector<> > &balls, StrongSimulation &sim, StrongSimulation &tim, unsigned &res) {
// 	unsigned workload = balls.size() / args_info.threads_arg;

// 	strongsimulation0(pid, pid * workload, (pid+1) * workload, balls, sim, tim, res);
// 	if (!pid && balls.size() % workload) {
// 		strongsimulation0(pid, workload * args_info.threads_arg, balls.size() % workload, balls, sim, tim, res);
// 	}
// }

// void strongsimulation0(const unsigned pid, const unsigned min, const unsigned max, vector<bm::bvector<> > &balls, vector<bm::bvector<> > &borders, StrongSimulation &sim, unsigned &res) {
// 	StrongSimulation tim(sim);

// 	for (unsigned i = min; i < max; ++i) {
// 		// cout << balls[i] << endl;
// 		tim.set(sim, balls[i], borders[i]);

// 		do {
// 			tim.fixpoint(3);
// 			tim.resolveDependencies();
// 		} while (!tim.stable());

// 		if (!tim.empty()) {
// 			// if (args_info.output_given)
// 			// 	sim.out() << tim.simulation() << endl;
// 			++res;
// 		}
// 	}
// }

const int StrongSimulation::diameter() {
	if (!_diameter) {
		if (_sourceV.size()) {
			_diameter = 1; // minimal diameter
		} else {
			return 0;
		}
		unsigned numtriples = _sourceV.size()/2;
		bm::bvector<> bfs(numtriples);
		for (unsigned i = 0; i < _sourceV.size(); i+= 2) {
			for (unsigned j = i+2; j < _sourceV.size(); j+= 2) {
				bfs.reset();
				bfs.set(i/2);
				unsigned dist = 1;
				do {
					unsigned trip = bfs.get_first();
					bm::bvector<> bfs0 = bfs;
					do {
						// cout << "here" << endl;
						bfs |= _tripleNeighbors[trip];
						// cout << "there" << endl;
					} while (trip = bfs0.get_next(trip));
					++dist; // distance increased
					// cout << bfs << " [" << dist << "]" << endl;
				} while (dist <= numtriples && !bfs.test(j/2));
				if (dist > numtriples) {
					_diameter = -1;
					return -1;
				}
				_diameter = (dist > _diameter ? dist : _diameter);
			}
		}
	}
	return _diameter;
}

//StrongSim
std::set<std::string> StrongSimulation::getPartner(std::string a){
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

std::set<std::string> StrongSimulation::getPartner(std::string a, std::set<std::string> Old){
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

std::set<SMatrix *> StrongSimulation::getEdges(std::string a, std::set<std::string> Old){
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

std::vector<std::string> StrongSimulation::queryNodeSet() {
	_sim = simulation();
	std::vector<std::string> setOfNodes;
	for (auto &pair: _sim) {
		//cout << "Nodes: " << pair.first << endl;
		setOfNodes.push_back(pair.first);
	}
	return setOfNodes;
}

std::vector<std::vector< std::set<SMatrix *>>>  StrongSimulation::edgeList(std::set<std::string> center, unsigned int radius ){
	std::vector<std::vector< std::set<SMatrix *>>>  fin;
	for(auto c : center) {

		std::vector< std::set<SMatrix *>>  result;
		std::set<std::string> Old;
		std::set<std::string> Set;
		std::set<std::string> Hold;
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
					Next.insert(b);
					Old.insert(b);
				}
			}
			result.push_back(Edges);
		}
		fin.push_back(result);
	}
	return fin;
}

bm::bvector<> StrongSimulation::centerList(std::set<std::string> center) {
	_sim = simulation();
	bm::bvector<> sum;
	for (auto &pair: _sim) {
		//cout << "PC:" << pair.second << "F:" << pair.first << " SUM:" << sum << endl;
		if(center.count(pair.first) == 1){
			sum |= pair.second;
		}
		//cout << "PC:" << pair.second << " SUM:" << sum << endl;
	}
	return sum;
}

bm::bvector<> StrongSimulation::centerList(std::string centerNode) {
	_sim = simulation();
	bm::bvector<> sum;
	for (auto &pair: _sim) {
		//cout << "PC:" << pair.second << "F:" << pair.first << " SUM:" << sum << endl;
		if(pair.first == centerNode){
			sum |= pair.second;
		}
		//cout << "PC:" << pair.second << " SUM:" << sum << endl;
	}
	return sum;
}

bm::bvector<> StrongSimulation::simList() {
	_sim = simulation();
	bm::bvector<> sum;
	for (auto &pair: _sim) {
		//cout << "PC:" << pair.second << "F:" << pair.first << " SUM:" << sum << endl;
		sum |= pair.second;
		//cout << "PC:" << pair.second << " SUM:" << sum << endl;
	}
	return sum;
}

unsigned int StrongSimulation::diameter(std::vector<std::string> nodes){
	unsigned int diameter = 0;
	std::set<std::string> Set;
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
		if(dia > diameter){
			diameter = dia;
		}
	}
	return diameter;
}

unsigned int StrongSimulation::radius(std::vector<std::string> nodes){
	unsigned int diameter = 99;
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
	return diameter;
}

std::set<std::string> StrongSimulation::center(std::vector<std::string> nodes,unsigned int radius){
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


std::string StrongSimulation::output(string delimiter) {
	_sim = simulation();
	std::string out;	
	std::string split = " #|# ";
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

std::string StrongSimulation::var() {
	simulation();
	std::string var;

	for (auto &pair: _sim) {
		//cout << pair.first << "[]:";
		var += pair.first + " #||# ";
	}
	return var;
}

void StrongSimulation::setFullyUnstable() {
	for(auto &i: _order){
		_stable[i] = false;
	}
}

// unsigned int StrongSimulation::fixpointY() {
// 	setFullyUnstable();
// 	unsigned int iter = 0;
// 	while (!stable()) {
// 		iter++;
// 		for (auto &i: _order) {
// 			// if current equation is already stable, continue
// 			if (_stable[i])
// 				continue;

// 			// get source variable (including an update)
// 			// y <= x \times A
// 			bm::bvector<> &x = _sourceV[i]->getVal();
// 			bm::bvector<> &y = _targetV[i]->getVal();
// 			//cout << "computing ";
// 			cout<< "Eq: " << i << " = " << _targetV[i]->getId() << " <= " << _sourceV[i]->getId() << " x " << _operand[i] << endl;
// 			cout << _targetV[i]->getVal() << " <= " << _sourceV[i]->getVal() << " x " << _operand[i] << endl;

// 			bm::bvector<> Y(max());
// 			unsigned int r = x.get_first(); // position of first 1
// 			if (r==0 && !x.test(0)) {
// 				_targetV[i]->null(); // if x is 0, then y is
// 				_stable[i] = true;
// 				if (_targetV[i]->isMandatory()) {
// 					_empty = true;
// 					return iter;
// 				}
// 				else {
// 					continue;
// 				}
// 			} else {
// 				do {
// 					_operand[i]->matrix(_dirs[i]).rowbv(y, Y, r);
// 				} while ((r=x.get_next(r)) != 0);
// 			}
// 			cout << "IN Target: " << _targetV[i]->getVal() << endl;
// 			_targetV[i]->update(Y);
// 			cout << "Out Target: " << _targetV[i]->getVal() << endl;
// 			_stable[i] = true;

// 			if (_targetV[i]->isEmpty() && 
// 				_targetV[i]->isMandatory()) {
// 				_empty = true;
// 				return iter;

// 			}
// 		}
// 	}

// 	bool changes = false;
// 	for (unsigned int i = 0; i < _vars.size(); ++i) {
// 		if (_vars[i]->updVal()) {
// 			changes = true;
// 		}
// 	}

// 	if (changes) {
// 		return fixpoint() + iter;
// 	}
// 	return iter;
// }

void StrongSimulation::setOutput(const string &filename) {
	_filename = filename + ".strong.out";
	setOutputStream(*new ofstream());
	_fileout = true;
}

void StrongSimulation::statistics(const std::string &filename) {
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

void StrongSimulation::statistics(std::ostream &os) {
	_reporter(os);
	_reporter.report();
}

void StrongSimulation::csv(const std::string &filename, const char delim) {
	ofstream csv_f;
	_reporter.note("filename", _query, filename);

	csv_f.open(filename+".strong.csv", ofstream::app);
	if (!checkStream(filename+".strong.csv", csv_f)) 
		return;

	overtakeKarla(filename);
	csv(csv_f, delim);

	csv_f.close();
}

void StrongSimulation::csv(std::ostream &os, const char delim) {
	os << "<<<";

	os << delim << _reporter.getValue("filename", _query);
	
	os << delim << _reporter.getValue("compilation time", _query);
	os << delim << _reporter.getValue("fixpoint", _query);
	os << delim << _reporter.getValue("# of iterations", _query);
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

void StrongSimulation::set(StrongSimulation &s, bm::bvector<> &ball, bm::bvector<> &border) {
	_empty = s._empty;

	// if (ball.test(27225568))
	// 	cout << "!";
	for (unsigned i = 0; i < _vars.size(); ++i) {
		_vars[i]->set(ball, s._vars[i]->val());
		// assert(_vars[i]->val().count());
	}
}
