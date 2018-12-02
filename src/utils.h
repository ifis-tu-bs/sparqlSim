#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <fstream>
#include <sstream>

#include <map>
#include <string>
#include "bm.h"

using namespace std;

class Graph;

void printInteractiveHelp();
char getDelimiter();

void storeGraph(const string &filepref);
void loadGraph(const string &filepref);

void loadDBFile(const string &filename, bool triples = true);
void readGraphFromFile(const string &filename, Graph &g, bool triples = true);
void readGraphFromCompressed(const string &filename, Graph &g, bool triples = true);

bool checkStream(const string &fname, ifstream &f);
bool checkStream(const string &fname, ofstream &f);

void verbose(const string &msg);

void printTime();
void printTime(string &filename);
void printSettings();

#endif /* UTILS_H */
