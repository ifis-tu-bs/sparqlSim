#ifndef GRAPH_IO_H
#define GRAPH_IO_H

#include <iostream>

using namespace std;

#include "bm.h"

class Graph;
class Node;
class Label;
class SMatrix;
class QGSimulation;


/* Overleaded Stream Operators */

ostream & operator<<(ostream &os, Graph &g);

ostream & operator<<(ostream &os, const Node &n);

ostream & operator<<(ostream &os, Label &l);

std::ostream & operator<<(std::ostream &os, SMatrix &a);

std::ostream & operator<<(std::ostream &os, const bm::bvector<> &a);

ostream & operator<<(ostream &os, QGSimulation &s);

#endif /* GRAPH_IO_H */
