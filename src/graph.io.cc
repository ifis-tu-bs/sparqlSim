#include "graph.h"

#include "node.h"
#include "label.h"
#include "simulation.h"
#include "smatrix.h"
#include "utils.h"
#include "reporter.h"

#include "bm.h"
#include "bmserial.h"

#include <cstdint>
#include <sstream>

using namespace std;

extern Reporter Karla;

/* helper functions */
static unsigned char* serialize_bvector(bm::serializer<bm::bvector<> >& bvs, 
                                 bm::bvector<>& bv)
{
    // It is reccomended to optimize vector before serialization.
    BM_DECLARE_TEMP_BLOCK(tb)
    bm::bvector<>::statistics st;
    bv.optimize(tb, bm::bvector<>::opt_compress, &st);
    // cout << "Bits count:" << bv.count() << endl;
    // cout << "Bit blocks:" << st.bit_blocks << endl;
    // cout << "GAP blocks:" << st.gap_blocks << endl;
    // cout << "Memory used:"<< st.memory_used << endl;
    // cout << "Max.serialize mem.:" << st.max_serialize_mem << endl;
    // Allocate serialization buffer.
    unsigned char*  buf = new unsigned char[st.max_serialize_mem];
    // Serialization to memory.
    unsigned len = bvs.serialize(bv, buf, st.max_serialize_mem);
    // cout << "Serialized size:" << len << endl << endl;
    return buf;
}

ostream & operator<<(ostream &os, const Node &n) {
	return os << n._name;
}

ostream & operator<<(ostream &os, Label &l) {
	// 1. write label name
	os << l._me;

	return os;
}

std::ostream & operator<<(std::ostream &os, SMatrix &a) {
	unsigned char *buf = 0;
	bm::serializer<bm::bvector<> > bvs;

	// next settings provide lowest size
    bvs.byte_order_serialization(false);
    bvs.gap_length_serialization(false);
    bvs.set_compression_level(4);

	for (auto &p : (*a._rows)) {
		bm::bvector<> row = a.rowBV(p.first);
		try {
			buf = serialize_bvector(bvs,row);
		} catch (std::exception &ex) {
			cerr << ex.what() << endl;
			delete [] buf;
			break;
		}

		os << p.first << endl << buf << endl;
	}

	return os;
}

std::ostream & operator<<(std::ostream &os, const bm::bvector<> &a) {
	// os << "(" << a._max << ")";
	os << "[";
	
	unsigned int pos = a.get_first();
	if (pos == 0 && !a.test(pos)) {
		for (unsigned int i = 0; i < 7; ++i) {
		// for (unsigned int i = 0; i < a.size(); ++i) {
			os << " 0"; 
		}
		os << " ]";
		return os;
	}

	unsigned int lpos = 0;
	do {
		for (unsigned int i = lpos; i < pos; ++i) {
			os << " 0";
		}
		os << " 1";
		lpos = pos + 1;
		pos = a.get_next(pos);
	} while (pos != 0);
	for (unsigned int i = lpos; i < 7; ++i) {
	// for (unsigned int i = lpos; i < a.size(); ++i) {
		os << " 0";
	}
	os << " ]";

	return os;
}

ostream & operator<<(ostream &os, QGSimulation &sim) {

	if (sim.empty())
		return os;

	// sim.print();

	for (unsigned int i = 0; i < sim._db->Ssize(); ++i) {
		Label &l = sim._db->getLabel(i);
		const std::string p = l.str();
		// cout << "label: " << p << endl;
		std::vector<unsigned int> nodepairs = l.getPairs();
		unsigned count = 0;
		unsigned cstep = (sim.countTriples() * 3) / 20;
		unsigned step = cstep;
		unsigned output = 15;

		for (unsigned int s = 0; s < nodepairs.size(); s += 2) {
			if (sim.isTriple(nodepairs[s], p, nodepairs[s+1])) {
				os << endl
				   << sim._db->getNodeName(nodepairs[s]) << " "
				   << p << " "
				   << sim._db->getNodeName(nodepairs[s+1]) << " .";
			}
		}
	}

	return os;
}

void Graph::report(ostream &os) const {
	os << "      |V| = " << Vsize() << endl;
	os << " |\\Sigma| = " << _Sigma.size() << endl;
	os << "      |E| = " << Esize() << endl;
}

/*
 *  Stores the current graph on disk
 *    dir/nodes
 *       /predicates
 *       /labela
 *       /labelb
 *       /...
 *       /labelz
 */
void Graph::store(const string &dir) {
	ofstream f, ff;
	string prefix = dir;
	if (prefix[prefix.size()-1] != '/')
		prefix += '/';

	f.open(prefix+"sparqlSim.conf");
	f << "nodes:nodes" << endl
	  << "nodenum:" << _nodes.size() << endl
	  << "predicates:predicates" << endl
	  << "predicatenum:" << _Sigma.size() << endl
	  << "matrices:matrices" << endl
	  << "triples:" << _numTriples << endl;
	f.close();

	// writing the nodes
	f.open(prefix+"nodes");
	if (!checkStream(prefix+"nodes",f)) {
		cerr << "error: make sure '" << prefix << "' exists" << endl;
		return;
	}
	for (Node *n : _nodes)
		f << *n << endl;
	f.close();

	// writing the predicates
	f.open(prefix+"predicates");
	if (!checkStream(prefix+"predicates",f)) {
		cerr << "error: make sure '" << prefix << "' exists" << endl;
		return;
	}
	for (unsigned i = 0; i < _Sigma.size(); ++i) {
		f << *_Sigma[i] << endl;
	}
	f.close();

	// writing adjacency matrices
	stringstream num;
	f.open(prefix+"matrices");
	for (Label *l : _Sigma) {
		l->a().storeIn(f);
	}
	f.close();

}

void Graph::load(const string &dir) {
	ifstream f;
	string line;

	size_t h; // hash values

	config["prefix"] = dir;
	if (config["prefix"][config["prefix"].size()-1] != '/')
		config["prefix"] += '/';

	f.open(config["prefix"]+"sparqlSim.conf");
	while (getline(f,line)) {
		unsigned split = line.find(':');
		config[line.substr(0,split)] = line.substr(split+1);
	}
	f.close();

	// cout << "prefix" << endl;

	
	// maximal number for progress bar
	unsigned N = stoi(config["nodenum"]);
	
	// loading the nodes
	f.open(config["prefix"]+config["nodes"]);
	if (!checkStream(config["prefix"]+config["nodes"],f)) {
		cerr << "error: make sure '" << config["prefix"] << "' exists" << endl;
		return;
	}
	// Karla.initProgress(stoi(config["nodenum"]), 12);
	while (getline(f,line)) {
		// cout << "node '" << line << "'" << endl;
		_Rnodes[strHash(line)] = _nodenum++;
		// NEIGHBORS.push_back(bm::bvector<>(N));
		/*if (!_nodenum % Karla.step()) {
			Karla.updateProgress(_nodenum);
		}*/
	}
	// Karla.finishProgress();
	f.close();
	// cout << "nodes" << endl;
	// report(cout);

	// loading and preparing the predicates
	f.open(config["prefix"]+config["predicates"]);
	if (!checkStream(config["prefix"]+config["predicates"],f)) {
		cerr << "error: make sure '" << config["prefix"] << "' exists" << endl;
		return;
	}
	while (getline(f,line)) {
		h = strHash(line);
		assert(_RSigma.find(h) == _RSigma.end());
		_RSigma[h] = _Sigma.size();
		_Sigma.push_back(new Label(_nodenum, line));
		_Sigma.back()->a().loadMode();
	}
	f.close();


	if (config.find("triples") != config.end() &&
		config["triples"].size()) {
		_numTriples = stoi(config["triples"]);
	}

	// loading adjacency matrices
	f.open(config["prefix"]+config["matrices"]);
	
	unsigned last, next;
	unsigned numrows;
	unsigned p = 0;
	while (getline(f,line)) {
		next = line.find(':');

		
		numrows = stoi(line.substr(next+1));
		while (numrows--) {
			getline(f,line);
			loadRow(p,line);
		}

		// _Sigma[p]->setAT(new SMatrix(_Sigma[p]->a(), true));
		_Sigma[p]->makeFinal();
		_Sigma[p]->optimize(stoi(config["nodenum"]));

		++p;
	}

	f.close();

	NEIGHBORS.optimize();
}

ostream &operator<<(ostream &os, const map<string, bm::bvector<> > &sim) {
	const string vDelim = "||";
	const string mDelim = "|";
	const string pre = "# ";
	const string post = " #";

	unsigned first = 0;

	for (auto &pair : sim) {
		if (first++) {
			os << vDelim;
		}
		unsigned p = pair.second.get_first();
		if (p || pair.second.test(0)) {
			os << pre << p << post;
			while (p = pair.second.get_next(p)) {
				os << mDelim << pre << p << post;
			}
		}
	}

	return os;
}
