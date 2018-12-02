#include "reporter.h"

#include <iostream>
#include <iomanip>
#include <map>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

Reporter::Reporter() : _os(new ostream(NULL)) {}
Reporter::~Reporter() {}

void Reporter::start(const string &callee, const string &note) {
	if (_report.find(callee) == _report.end()) {
		_report[callee] = vector<std::pair<string, string> >();
		_rstart[callee] = map<string, high_resolution_clock::time_point>();
		_rend[callee] = map<string, high_resolution_clock::time_point>();
	}

	// _report[callee][note] = 0.0;
	_rstart[callee][note] = high_resolution_clock::now();
}

void Reporter::end(const string &callee, const string &note) {
	_rend[callee][note] = high_resolution_clock::now();
	if (_rstart.find(callee) != _rstart.end()
		&& _rstart[callee].find(note) != _rstart[callee].end()) {
		duration<double> time_span =
				duration_cast<duration<double> >(
					_rend[callee][note]-_rstart[callee][note]);
		tmp.str("");
		tmp << time_span.count();
		_report[callee].push_back(std::pair<string,string>(note, tmp.str()));
		tmp.str("");
	}
	_order.push_back(callee);
}

void Reporter::note(const string &callee, const string &note, const string &val) {
	_report[callee].push_back(std::pair<string,string>(note, val));
}

const unsigned int Reporter::size() const { return _report.size(); }

void Reporter::shout(const string &callee) {
	out() << "# Report: " << callee;
	// if (_report[callee].first >= 0.0)
	// 	out() << " [runtime: " << _report[callee][note] << " s]";
	// if (note.size() > 0)
	// 	out() << " (" << note << ")";
	// out() << endl;
}

string Reporter::get(const string &callee, const string &note) {
	stringstream result;
	if (_report[callee].empty())
		return "";
	result << callee << "," << _report[callee].back().second << "," << _report[callee].back().first << endl;
	return result.str();
}

string Reporter::getValue(const string &callee, const string &note) {
	stringstream result;
	if (_report[callee].empty())
		return "";
	result << _report[callee].back().second;
	return result.str();
}

string Reporter::getPretty(const string &callee, const string &note) {
	stringstream result;
	result << callee << "     " << _report[callee].back().second << " [" << note << "]" << endl;
	return result.str();
}

// double Reporter::getDoubleValue(const string &callee, const string &note) {
// 	return _report[callee].back().second;
// }

void Reporter::report() {
	out() << endl << "## Runtime Report ##" << endl;
	for (auto & i : _report) {
		for (auto & j : i.second) {
			out() << "# Report: " << i.first;
			// if (j.second >= 0.0)
				out() << " [runtime: " << j.second << " s]";
			// if (j.first.size())
				out() << " (" << j.first << ")";
			out() << endl;
		}
	}
	out() << "####################" << endl;
}


Reporter & Reporter::operator<<(const string & s) {
	out() << s;
	return *this;
}

Reporter & Reporter::operator<<(const int & s) {
	out() << s;
	return *this;
}

void Reporter::endline() {
	out() << endl;
}

// Replaces the output stream
void Reporter::operator()(std::ostream &os) {
	_os = &os;
}

ostream &Reporter::out() {
	return *_os;
}

void Reporter::initProgress(const unsigned &max, const unsigned &steps) {
	_max = max;
	_steps = steps;
	_progress = 0;
	_step = _max / _steps;
}

void Reporter::printProgress(const unsigned val) {
	if (val) {
		updateProgressVal(val);
	}

	endline();
	out() << "   [";
	for (unsigned short i = 0; i < _progress; ++i) {
		out() << "|";
	}
	for (unsigned short i = 0; i < _steps - _progress; ++i) {
		out() << " ";
	}
	out() << "] " << setw(2) << setfill(' ') << (100 * _progress) / _steps << "%";

	_barPrinted = true;
}

void Reporter::updateProgress(const unsigned val) {
	if (!_barPrinted) {
		printProgress(val);
		return;
	}
	
	out() << "\r\r\r\r\r";
	for (unsigned short i = 0; i < _steps; ++i) {
		out() << '\r';
	}
	updateProgressVal(val);
	for (unsigned short i = 0; i < _progress; ++i) {
		out() << "|";
	}
	for (unsigned short i = 0; i < _steps - _progress; ++i) {
		out() << " ";
	}
	out() << "] " << setw(2) << setfill(' ') << (100 * _progress) / _steps << "%";
	out().flush();
}

void Reporter::updateProgressVal(const unsigned val) {
	while (val > _step * _progress)
		++_progress;
}

void Reporter::finishProgress() {
	if (!_barPrinted) {
		printProgress(_max);
		_barPrinted = true;
	}

	updateProgress(_max);
	endline();
}

const unsigned Reporter::step() const {
	return _step + 1;
}
