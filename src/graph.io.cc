#include "graph.h"

#include "node.h"
#include "label.h"
#include "simulation.h"
#include "edge.h"
#include "smatrix.h"
#include "utils.h"

#include "bm.h"
#include "bmserial.h"

#include <sstream>

using namespace std;

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

/* Overloaded Output Streams */

ostream & operator<<(ostream &os, Graph &g) {
	if (!g._groups.empty()) {
		return (os << g.printQuery());
	}
	// for (unsigned int i = 0; i < g.Vsize(); ++i) {
	// 	string sub = g.getNode(i).getName();
	// 	for (auto &p : g._postt[i]) {
	// 		for (unsigned int q = 0; q < p.second.size(); ++q)
	// 			os << sub << " "
	// 					<< g._Sigma[p.first]->str() << " "
	// 					<< g.getNode(p.second[q]).getName() << " ." << endl;
	// 	}
	// }
	for (unsigned int l = 0; l < g._Sigma.size(); ++l) {
		vector<unsigned int> pairs = g._Sigma[l]->getPairs();
		string pred = g._Sigma[l]->str();
		for (unsigned int v = 0; v < pairs.size(); v += 2) {
			os << g.getNode(pairs[v]) << " " 
			   << pred << " " 
			   << g.getNode(pairs[v+1]) << endl;
		}
	}
	return os;
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

	for (auto &p : a._rows) {
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
	// for (unsigned int i = 0; i < max; ++i) {
	// 	// if (rowNull(i)) {
	// 	// 	os << "[" << bm::bvector<>(_max) << "]" << endl;
	// 	// }
	// }
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
			// cout << "checking nodes " << sim._db->getNode(nodepairs[s])
			//      << " and " << sim._db->getNode(nodepairs[s+1]) << endl;
			if (sim.isTriple(nodepairs[s], p, nodepairs[s+1])) {
				// cout << "success" << endl;
				count++;
				if (step < count) {
					cout << " " << output << "%";
					step += cstep;
					output += 15;
				}

				os << endl
				   << sim._db->getNode(nodepairs[s]) << " "
				   << p << " "
				   << sim._db->getNode(nodepairs[s+1]) << " .";
			}
		}
	}

	return os;
}

/* Class specific output operations */

string Graph::printGroup(unsigned int g) {
	stringstream r;

	if (_groups[g].empty())
		return "";
	r << "{";
	for (Edge *e : _groups[g])
		r << " " << *e;
	r << " }";

	return r.str();
}

string Graph::printQuery(unsigned int g) {
	stringstream r;

	if (!g) {
		r << "SELECT * WHERE ";
	}

	if (_query[g].empty())
		r << printGroup(g);

	for (auto &x : _query[g]) {
		switch (x.first) {
			case GROUP: {
				r << "{";
				r << printQuery(x.second);
				r << " }";
				break;
			}
			case OPTIONAL: {
				r << printGroup(g);
				r << " OPTIONAL ";
				r << printQuery(x.second);
				break;
			}
			case JOIN: {
				r << printGroup(g);
				r << " ";
				r << printQuery(x.second);
				break;
			}
			case UNION: {
				r << printGroup(g);
				r << " UNION ";
				r << printQuery(x.second);
				break;
			}
			default:
				r << printGroup(g);
			  break;
		}
	}

	return r.str();
}

// string Graph::printInsert(Simulation &sim, Graph &g) {
// 	stringstream r;

// 	assert(sim.max() == g.size());
// 	for (unsigned int i = 0; i < sim.size(); ++i) {
// 		for (unsigned int j = 0; j < g.size(); ++j) {
// 			if (sim(i,j))
// 				r << g.getNode(j).getName() << " <simulates> <sim_" << i << "> ." << endl;
// 		}
// 	}

// 	return r.str();
// }

string Graph::report() {
	stringstream res;

	res << "|V|= " << Vsize() << endl;
	res << "|\\Sigma|= " << _Sigma.size() << endl;
	res << "|E|= " << _numTriples << endl;

	// cout << "#redundancies= " << _redundancy << " (not part of the graph)" << endl;
	return res.str();
}

// void Graph::print(const int i) {
// 	assert(i < Vsize());
// 	string sub = getNode(i).getName();
// 	for (auto &p : _postt[i]) {
// 		string lab = _Sigma[p.first]->str();
// 		for (unsigned int q = 0; q < p.second.size(); ++q)
// 			cout << sub << " "
// 					 << lab << " "
// 					 << getNode(p.second[q]).getName() << " ." << endl;
// 	}
// 	for (auto &p : _pree[i]) {
// 		string lab = _Sigma[p.first]->str();
// 		for (unsigned int q = 0; q < p.second.size(); ++q)
// 			cout << sub << " "
// 					 << lab << " "
// 					 << getNode(p.second[q]).getName() << " ." << endl;
// 	}
// }

string Graph::labelBitStrings() {
	stringstream result;
	for (Label *l : _Sigma) {
		result << l->str() << "," << l->a().intcount() << endl;
	}
	return result.str();
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
	Label *l = 0;
	for (unsigned i = 0; i < _Sigma.size(); ++i) {
		num.str("");
		l = _Sigma[i];
		num << prefix << i;
		ff.open(num.str());
		if (!checkStream(num.str(), ff))
			return;
		ff << l->a();
		ff.close();
	}

}
