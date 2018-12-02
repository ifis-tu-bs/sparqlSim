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

extern vector<string> files; // file names to be processed
vector<string> queries; // queries to be process

// bounds helper
unsigned NODEMAX = 1;
unsigned LABELMAX = 1;
unsigned MOD;

// gengetopt arguments record
gengetopt_args_info args_info;

// global reporter class
Reporter Karla;

// Global Database Instance
Graph *DB;
// Global Query
QGSimulation *Q;

// command return value produced by cmdline console
extern CmdReturn cret;

// bool FIXPOINT_STRATEGY = false;

int main(int argc, char **argv) {

	/****************************************\
	| 0. Parse parameters                    |
	|  - use gengetopt-parser                |
	|  - pass verbose flag to reporter class |
	\****************************************/

	cmdline_parser(argc,argv,&args_info);
	if (args_info.threads_arg > 1 && 
		args_info.output_given && 
		args_info.output_arg != output_arg_FILE) {
		cerr << "output to streams is not supported" << endl;
		cerr << "exiting the program..." << endl;
		return 1;
	}

	if (args_info.icde2019_flag) {
		args_info.output_given = 1;
		args_info.csv_flag = 1;
		args_info.verbose_flag = 1;
	}
	if (args_info.vldb2019_flag) {
		args_info.csv_flag = 1;
		args_info.eval_arg[0] = eval_arg_CSTRONG;
	}	

	if (args_info.bounds_given) {
		string bounds(args_info.bounds_arg);
		unsigned split = bounds.find(':');
		NODEMAX = stoi(bounds.substr(0,split));
		LABELMAX = stoi(bounds.substr(split+1));
	}

	ostream *verb = new ostream(NULL);
	if (args_info.verbose_flag) {
		delete verb;
		verb = &clog;
		if (args_info.verbose_file_given) {
			ofstream *vfile = new ofstream(args_info.verbose_file_arg);
			if (!checkStream(args_info.verbose_file_arg, *vfile)) {
				cerr << "could not open '" << args_info.verbose_file_arg << "': falling back to STDLOG" << endl;
			} else {
				verb = vfile;
			}
		}
	}
	ostream &verbose = *verb;

	Karla(verbose);
	// FIXPOINT_STRATEGY = args_info.vertical_flag;

	/****************************************\
	| 1. Initialize varibales + Construct db |
	|  - Graph db .. database file as graph  |
	\****************************************/
	DB = new Graph(NODEMAX, LABELMAX);
	Graph &db = *DB; // method-local database reference
	string fname; // the filename currently read / written

	if (!args_info.load_flag) {
		Karla.start("import initial db");
		for (unsigned int i = 0; i < args_info.inputs_num; ++i) {
			string f = args_info.inputs[i];
			cout << "importing file '" << f << "' (" << i+1 << " / " << args_info.inputs_num << ")" << endl;
			loadDBFile(f);
		}
		Karla.end("import initial db");
	} else {
		if (args_info.inputs_num > 1) {
			cerr << "loading of more than 1 database instance untested" << endl;
		}
		Karla.start("load initial db");
		for (unsigned int i = 0; i < args_info.inputs_num; ++i) {
			string f = args_info.inputs[i];
			cout << "loading file " << i+1 << " / " << args_info.inputs_num << endl;
			db.load(f);
		}
		Karla.end("load initial db");
	}

	// after reading is finished, print db stats
	db.report(clog);

	// storing before optimization, because it's not tested
	if (args_info.store_given) {
		Karla.start("store db to disk");
		db.store(args_info.store_arg);
		Karla.end("store db to disk");
	}

	// Compress and optimize storage
	if (!args_info.no_compression_flag) {
		Karla.start("begin compression");
		db.compress();
		Karla.end("begin compression");
	}

	// Minimization of the database
	Karla.start("minimize db");
	// TO BE FILLED BY DENIS!!
	// NOT IMPLEMENTED YET
	Karla.end("minimize db");

	Karla(clog);
	Karla.report();
	Karla(verbose);

	for (unsigned int i = 0; i < args_info.input_given; ++i) {
		files.push_back(args_info.input_arg[i]);
	}

	for (unsigned int i = 0; i < args_info.exec_given; ++i) {
		queries.push_back(args_info.exec_arg[i]);
	}

	// adapt the modulo modifier
	// cout << "eval_arg[0] == PRUNE? " << (args_info.eval_arg[0] == eval_arg_PRUNE) << endl;
	// cout << args_info.eval_min << ":" << args_info.eval_max << endl;
	MOD = (args_info.eval_given ? args_info.eval_given - 1 : 0);

	/****************************************\
	| 3. Main Loop                           |
	|  - read file(s) given as input         |
	|  - interactive flag gives prompt       |
	\****************************************/
	bool quit = false; // quit flag
	unsigned qinline = 0;
	unsigned qinfile = 0;
	string Fname;

	// stores the number of results evaluated for a query
	unsigned resultsize = 0;

	while (!quit) {
		// input read from cmdline
		string in;
		stringstream input;

		// Variable initialization
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
				in="";
				input.str("");
				continue;
			}
			input << in.substr(0,in.size()-1);

			// parse the given input line
			char *query = strdup(input.str().c_str());
			YY_BUFFER_STATE b = _yy_scan_string(query);
			_yyparse();
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
				unsigned n = 0;
				while (getline(f,tmp)) {
					if (tmp[0] != '#') {
						queries.push_back(tmp);
						// cout << n++ << ":" << tmp << endl;
					}

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
				db.report(clog);
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
		printTime();
		// Show current settings
		// printSettings();

		for (unsigned int i = 0; 
			i < args_info.iterations_arg || 
			args_info.permute_flag ||
			i < args_info.eval_given; ++i) {

			if (Q != NULL) {
				delete Q;
				Q = NULL;
			}

			if (args_info.eval_given) {
				if (MOD)
					--MOD;
				else
					MOD = args_info.eval_given - 1;
			}

			/// 1. parse the query
			char *query = strdup(input.str().c_str());
			YY_BUFFER_STATE b = _yy_scan_string(query);

			Karla.start("Query Compilation Time:", fname);
			int parse_error = _yyparse();
			Karla.end("Query Compilation Time:", fname);

			_yy_delete_buffer(b);
			delete query;

			if (parse_error) {
				cerr << "skip this query" << endl;
				break;
			}

			QGSimulation &sim = *Q;
			sim.setQuery(input.str());

			if (args_info.output_given) {
				// Karla.start("Producing Output:", fname);
				switch (args_info.output_arg) {
					case output_arg_FILE:
						sim.setOutput(fname);
						break;
					case output_arg_STDERR:
						sim.setOutputStream(cerr);
						break;
					case output_arg_STDLOG:
						sim.setOutputStream(clog);
						break;
					case output_arg_STDOUT:
						sim.setOutputStream(cout);
					default:
						break;
				}
				// Karla.end("Producing Output:", fname);
			}

			Karla.start("Query Evaluation Time:", fname);
			resultsize = sim.evaluate(verbose);
			Karla.end("Query Evaluation Time:", fname);

			cout << endl << " .. no. of results = " << resultsize << endl
			             << " .. time (in sec.) = " << Karla.getValue("Query Evaluation Time:", fname) << endl << endl;

			if (args_info.eval_arg[MOD] == eval_arg_PRUNE) {
				Karla.start("Producing Output (post)", fname);
				sim.output();
				Karla.end("Producing Output (post)", fname);
			}

			if (args_info.stats_flag) {
				Karla.start("Writing Statistics:", Fname);
				sim.statistics(Fname);
				Karla.end("Writing Statistics:", Fname);
			}

			if (args_info.csv_flag) {
				Karla.start("Writing CSV:", Fname);
				sim.csv(Fname);
				Karla.end("Writing CSV:", Fname);
			}

			// cleaning up
			delete Q;
			Q = NULL;
		}
	}

	// final report
	Karla.report();

	delete DB;

	return 0;
}
