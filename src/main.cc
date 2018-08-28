#include <cstdio>
#include <cstdlib>

#include <algorithm>

#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

// #include "algorithms.h"
#include "graph.h"
// #include "label.h"
// #include "node.h"
// #include "edge.h"
#include "reporter.h"
#include "simulation.h"

#include "config.h"
#include "cmdline.h"         // gengetopt
#include "utils.h"
#include "cmdline-parser.h"

typedef struct yy_buffer_state * YY_BUFFER_STATE;
extern YY_BUFFER_STATE _yy_scan_string(const char * str);
extern void _yy_delete_buffer(YY_BUFFER_STATE buffer);
extern int _yyerror(char const *msg);

extern vector<string> files;

/// MACRO DEFINITIONS
#define NODEMAX 500000000
#define LABELMAX 100000


// Graph *current;

gengetopt_args_info args_info;
Reporter Karla;

Graph *G, *DB;
// Graph *G, *Q, *DB;
QGSimulation *Q;

extern CmdReturn cret;

bool FIXPOINT_STRATEGY = false;

enum { DEFAULT, MAETAL, HHK } algorithm = DEFAULT;

int main(int argc, char **argv) {

	/****************************************\
	| 0. Parse parameters                    |
	|  - use gengetopt-parser                |
	|  - pass verbose flag to reporter class |
	\****************************************/
	cmdline_parser(argc,argv,&args_info);
	if (args_info.icde2019_flag) {
		args_info.pruning_flag = 1;
		args_info.csv_flag = 1;
		args_info.verbose_flag = 1;
		args_info.virtuoso_flag = 1;
	}
	if (args_info.maetal_flag) {
		args_info.verbose_flag = 1;
		algorithm = MAETAL;
	}
	if (args_info.hhk_flag) {
		args_info.verbose_flag = 1;
		algorithm = HHK;
	}

	Karla(args_info.verbose_flag);
	// FIXPOINT_STRATEGY = args_info.vertical_flag;

	/****************************************\
	| 1. Initialize varibales + Construct db |
	|  - Graph db .. database file as graph  |
	\****************************************/
	DB = new Graph(NODEMAX, LABELMAX);
	Graph &db = *DB;
	string fname; // the filename currently read
	string bla;

		Karla.start("import initial db");
		// A. Add the nodes
		for (unsigned int i = 0; i < args_info.inputs_num; ++i) {
			string f = args_info.inputs[i];
			cout << "importing file '" << f << "' (" << i+1 << " / " << args_info.inputs_num << ")" << endl;
			loadDBFile(f);
		}
		Karla.end("import initial db");
	// } else {
		// Karla.start("load initial db");
		// // A. Add the nodes
		// for (unsigned int i = 0; i < args_info.inputs_num; ++i) {
		// 	string f = args_info.inputs[i];
		// 	cout << "loading file " << i+1 << " / " << args_info.inputs_num << endl;
		// 	loadGraph(f);
		// }
		// Karla.end("load initial db");


	// Verbose output of db size
	cout << db.report();

	// Compress and optimize storage
	Karla.start("begin compression");
	db.compress();
	Karla.end("begin compression");
	// string q;
	// cout << "Want to quit? ";
	// cin >> q;
	// if (q[0]=='y'||q[0]=='Y')
	// 	return 0;

	// Minimization of the database
	Karla.start("minimize db");
	// TO BE FILLED BY DENIS!!
	Karla.end("minimize db");

	Karla.report();

	if (args_info.store_flag) {
		Karla.start("store db to disk");
		db.store(args_info.directory_arg);
		Karla.end("store db to disk");
	} 

	// free all memory but matrices
	if (args_info.memory_flag) {
		ofstream cfile("memory.csv");
		if (checkStream("memory.csv", cfile)) {
			cfile << db.labelBitStrings() << endl << endl;

			cfile << db.sizeOf();

			cfile.close();
		}

		// return 0;
	}

	/* 2. Read more parameters */
	vector<string> queries; // queries to process

	for (unsigned int i = 0; i < args_info.input_given; ++i) {
		files.push_back(args_info.input_arg[i]);
	}

	for (unsigned int i = 0; i < args_info.exec_given; ++i) {
		queries.push_back(args_info.exec_arg[i]);
	}

	/****************************************\
	| 3. Main Loop                           |
	|  - read file(s) given as input         |
	|  - interactive flag gives prompt       |
	\****************************************/
	bool quit = false; // quit flag
	unsigned int qinline = 0;
	unsigned int qinfile = 0;
	string Fname;

	while (!quit) {
		// Variable initialization
		string in;
		stringstream input;

		Q=NULL;
		fname="";

		if (files.empty() && queries.empty()) {
			if (!args_info.shell_flag) {
				quit = true;
				continue;
			}
			cout << "input> ";
			getline(cin, in);
			while (in[in.size()-1]!=';') {
				input << in;
				cout << "close by ';'> ";
				getline(cin, in);
			}
			if (input.str()[0] == '#') {
				continue;
			}
			input << in.substr(0,in.size()-1);

			int flag;
			char *query = strdup(input.str().c_str());
			YY_BUFFER_STATE b = _yy_scan_string(query);
			flag = _yyparse();
			_yy_delete_buffer(b);
			delete query;
		} else {
			if (queries.empty()) {
				cout << "trying file '" << files.back() << "'" << endl;
				Fname = files.back();
				ifstream f(Fname);
				if (!checkStream(Fname, f)) {
					files.pop_back();
					continue;
				}
				files.pop_back();
				string tmp;
				while (getline(f,tmp)) {
					if (tmp[0] != '#')
						queries.push_back(tmp);
				}
				f.close();
				// for (unsigned int j = 0; j < queries.size(); ++j)
				// 	cout << queries[j] << endl;
				qinfile = 0;
				continue;
			} else {
				// for (int j = 0; j < queries.size(); ++j) {
				if (qinfile >= queries.size()) {
					Fname = "";
					queries.clear();
					continue;
				}
				input << Fname << "_" << qinfile;
				fname = input.str();
				input.str("");
				input << queries[qinfile++];
				cout << input.str() << endl;
				cret = QUERY;
				// }
			}
		}

		switch (cret) {
			case LOAD:
			case IMPORT: {
				cret = NOTHING;
				for (int i = files.size(); i > 0; --i) {
					cout << "importing file '" << files[i-1] << "' (" << files.size()-i+1 << " / " << files.size() << ")" << endl;
      				loadDBFile(files[i-1]);
    			}
    			files.clear();
				cout << db.report();
				continue;
			}
			case QUIT: {
				cret = NOTHING;
				quit = true;
				continue;
			}
			case QUERY: {
				cret = NOTHING;
				if (Fname.empty()) { // only in this case we get inlines
					fname = "inline_query_" + to_string(qinline++);
				}
				break;
			}
			default: {
				continue;
			}
		}

		// Showing the current time
		time_t raw;
		struct tm * timeinfo;

		time(&raw);
		timeinfo = localtime(&raw);
		cout << endl << asctime(timeinfo) << endl;
		
		// Show current settings
		cout << endl;
		printSettings();
		cout << endl;

		unsigned triplecount = 0;
		vector<unsigned> order;

		for (unsigned int i = 0; i < args_info.iterations_arg || args_info.permute_flag; ++i) {

			if (Q != NULL) {
				delete Q;
				Q = NULL;
			}

			char *query = strdup(input.str().c_str());
			YY_BUFFER_STATE b = _yy_scan_string(query);

			Karla.start("Query Compilation Time:", fname);
			int flag = _yyparse();
			Karla.end("Query Compilation Time:", fname);

			_yy_delete_buffer(b);

			if (!i && args_info.virtuoso_flag) {
				//
				string sq(query);
				size_t split = sq.find("SELECT *");
				// string sq1 = sq.substr(0,split);
				string sq2 = sq.substr(split+8);
				// cout << sq1 << " : " << sq2 << endl;
				stringstream vquery;
				vquery << "SPARQL SELECT COUNT(*)" 
						<< " FROM " << "<http://" << fname << "# "
						<< sq2;
				// cout << vquery.str() << endl;
				
				ofstream qfile(fname+".virtuoso.profile.script");
				if (checkStream(fname+".virtuoso.profile.script",qfile)) {
					qfile << "profile('" << vquery.str() << "');" << endl;
					qfile.close();
				}
				qfile.open(fname+".virtuoso.script");
				if (checkStream(fname+".virtuoso.script",qfile)) {
					qfile << vquery.str() << ";" << endl;
					qfile.close();
				}
			}

			delete query;

			QGSimulation &sim = *Q;

			if (args_info.permute_flag) {
				if (!i) {
					order = sim.getOrder();
					sort(order.begin(), order.end());
				} else {
					if (!next_permutation(order.begin(),order.end())) {
						break;
					}
				}
				sim.setOrder(order);
			}
			
			Karla.start("computing dualsim", fname);

			unsigned int iter = 0;

			switch (algorithm) {
				case MAETAL: {
					iter = sim.MaEtAl();
					break;
				}
				case HHK: {
					iter = sim.HHK();
					break;
				}
				default: {
					if (args_info.profile_flag && !i) {
						iter = sim.fixpointWithProfile(3);
					} else {
						iter = sim.fixpoint(3);
					}
				}
			}
		
			Karla.end("computing dualsim", fname);

			// if (!i) {
			// 	triplecount = sim.countTriples();
			// }

			// if verbose output is given, print both times
			if (!i && args_info.verbose_flag) {
				// cout << Karla.getPretty("initialize simulation", fname);
				cout << Karla.getPretty("Query Compilation Time:", fname);
				cout << Karla.getPretty("computing dualsim", fname);
			}
			
			if (!i && args_info.pruning_flag) {
				ofstream pruning(fname+".reduced.nt");
				if (checkStream(fname+".reduced.nt", pruning)) {
					Karla.start("writing reduced database to disk", fname);
					pruning << "# |E|= " << DB->Esize() << endl;
					pruning << "#  Q = " << input.str(); 
					cout << "writing reduced database to file '" << fname << ".reduced.nt'" << endl << "  ..";
					pruning << sim;
					cout << " 100%" << endl;
					Karla.end("writing reduced database to disk", fname);
					
					pruning.close();
					
					if (args_info.verbose_flag) {
						cout << Karla.getPretty("writing reduced database to disk", fname);
					}
					
					if (args_info.virtuoso_flag) {
						ofstream gext(fname+".reduced.ext.graph");
						if (checkStream(fname+".reduced.ext.graph",gext)) {
							gext << "<http://" << fname << "#";
							gext.close();	
						}
					}
				}


			}

			if (args_info.csv_flag) {
				ofstream statistics;
				if (!i)
					statistics.open(fname+".statistics", ofstream::trunc);
				else
					statistics.open(fname+".statistics", ofstream::app);
				if (!checkStream(fname+".statistics", statistics))
					continue;
				statistics 	
					<< Karla.getDoubleValue("Query Compilation Time:", fname)
					<< ","
					<< Karla.getDoubleValue("computing dualsim", fname)
					<< ","
					<< iter
					<< ","
					<< triplecount
					<< ","
					<< input.str()
					<< ","
					<< sim.order()
					<< endl;
				statistics.close();
			}

			// cleaning up
			delete Q;
			Q = NULL;
		}
	}

	Karla.report();
	delete DB;

	return 0;
}