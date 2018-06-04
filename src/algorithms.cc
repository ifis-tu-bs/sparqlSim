// #include "algorithms.h"

// #include <cassert>
// #include <iostream>
// #include <unordered_set>

// using namespace std;

// #include "edge.h"
// #include "graph.h"
// #include "node.h"
// #include "label.h"

// #include "simulation.h"

// #include "reporter.h"

// extern Reporter Karla;

// // TWO ISSUES:
// // (1) Recognize duplicate balls; DONE, there are not so many
// // (1b) Recognize distinct output graphs; DONE
// // (2) Only add relevant edges; DONE
// // void match(Graph &q, Graph &g, vector<Graph *> &Theta) {
// // 	// vector<Graph *> result;
// // 	vector<unordered_set<unsigned int> > balls;

// // 	// cout << "computing diameter of pattern" << endl;
// // 	int dq = q.diameter();
// // 	// cout << "computed diameter = " << dq << endl;
// // 	assert(dq >= 0);

// // 	double pct, lpct;
// // 	lpct = 0.0;
// // 	for (int i = 0; i < g.size(); ++i) {
// // 		pct = (double) i / (double) g.size();
// // 		if (pct > lpct + .01)
// // 			cout << (unsigned int) (pct * 100.0) << " percent" << endl;
// // 		// Compute dual simulation for ball G[i,dq]
// // 		// cout << "computing ball of " << g(i) << endl;
// // 		Graph ball(g, i, dq);
// // 		// Karla << i << endl;

// // 		// check for duplicates
// // 		// unordered_set<unsigned int> bset;
// // 		// bool duplicate = false;
// // 		// for (int i = 0; i < ball.size(); ++i) {
// // 		// 	bset.insert(g.getIndex(ball(i)));
// // 		// }
// // 		// cout <<	"bset:";
// // 		// for (auto &j : bset)
// // 		// 	cout << " " << j;
// // 		// cout << endl;
// // 		// for (int i = 0; i < balls.size(); ++i) {
// // 		// 	// cout << "balls[" << i << "]:";
// // 		// 	// 	for (auto &j : balls[i])
// // 		// 	// 		cout << " " << j;
// // 		// 	// 	cout << endl;
// // 		// 	if (balls[i] == bset) {
// // 		// 		// cout << "balls[" << i << "]:";
// // 		// 		// for (auto &j : balls[i])
// // 		// 		// 	cout << " " << j;
// // 		// 		// cout << endl << "bset:";
// // 		// 		// for (auto &j : bset)
// // 		// 		// 	cout << " " << j;
// // 		// 		// cout << endl <<"duplicate found" << endl;
// // 		// 		duplicate = true;
// // 		// 		break;
// // 		// 	}
// // 		// }
// // 		// if (duplicate)
// // 		// 	continue;
// // 		// else
// // 		// 	balls.push_back(bset);
// // 		// cout << ball;

// // 		Simulation sim;
// // 		// sim : nodes of q -> set of nodes of ball
// // 		dualSim(q, ball, sim);

// // 		// Extract match graph if exists
// // 		bool icont = false; // node i contained in sim?
// // 		for (unsigned int i = 0; i < sim.size(); ++i) {
// // 			if (sim(i,0)) { // node i is represented by the first node in a ball
// // 				icont = true;
// // 				break;
// // 			}
// // 		}

// // 		// cout << "[ ";
// // 		// for (int i = 0; i < sim.size(); ++i) {
// // 		// 	cout << q(i) << " ->";
// // 		// 	for (unsigned int j : sim[i])
// // 		// 		cout << " " << ball(j);
// // 		// 	cout << endl << "  ";
// // 		// }
// // 		// cout << "]" << endl;

// // 		// Add match graph
// // 		if (!sim.empty() && icont) { // TODO check for emptiness of each component
// // 			Graph *gs = new Graph(ball, q, sim);
// // 			// Graph *newg = new Graph(*gs);
// // 			// bool duplicate = false;
// // 			// for (Graph *other : Theta)
// // 			// 	if (*other == *newg) {
// // 			// 		duplicate = true;
// // 			// 		break;
// // 			// 	}
// // 			// if (duplicate)
// // 			// 	continue;
// // 			Theta.push_back(gs);
// // 			// Theta.push_back(newg);
// // 		}
// // 	}

// // 	// return result;
// // }

// // assume initialized simulation
// void dualSimOld(Graph &q, Graph &ball, Simulation &sim) {
// 	// Initialize simulation with |q| rows and |ball| columns
// 	// sim.initialize(q.size(), ball.size());

// 	// // runtime variables
// 	// bool changes = true; // tracks the changes in one iteration
// 	// bool sims; // tracks the simulation status for one edge ulux

// 	// while (changes) {
// 	// 	// so far, nothing changes
// 	// 	changes = false;

// 	// 	// for each pattern node u, find those nodes v
// 	// 	// pretending to simulate u
// 	// 	for (unsigned int u = 0; u < q.size(); ++u) {
// 	// 		// candidate simulators v for u
// 	// 		for (unsigned int v = sim.init(u); v < sim.max(); v = sim.next(u)) {
// 	// 			// for each edge u -l-> u'
// 	// 			for (Edge *ulux : q._post[u]) {
// 	// 				sims = false;
// 	// 				// does there exist a v' s.t. v -l-> v' and sim(u',v')
// 	// 				for (Edge *vlvx : ball._post[v]) { // if one simulates edge, ok
// 	// 					if (ulux->label() == vlvx->label()
// 	// 						&& sim(q.getIndex(ulux->target()),ball.getIndex(vlvx->target()))) {
// 	// 						sims = true;
// 	// 						break;
// 	// 					}
// 	// 				}
// 	// 				// if u -l-> u' is not simulated, v does not simulate u
// 	// 				// and something has changed
// 	// 				if (!sims) {
// 	// 					sim.toggle(u);
// 	// 					changes = true;
// 	// 					break;
// 	// 				}
// 	// 			}
// 	// 			if (!sim(u,v)) // v could have been outsourced earlier
// 	// 				continue;
// 	// 			// Every backwards edge u'-l->u has a v'-l->v and sim(u',v')
// 	// 			for (Edge *ulux : q._pre[u]) {
// 	// 				sims = false;

// 	// 				for (Edge *vlvx : ball._pre[v]) { // if one simulates edge, ok
// 	// 					if (ulux->label() == vlvx->label()
// 	// 						&& sim(q.getIndex(ulux->source()),ball.getIndex(vlvx->source()))) {
// 	// 						sims = true;
// 	// 						break;
// 	// 					}
// 	// 				}
// 	// 				// if one edge is not simulated by j, j is removed
// 	// 				if (!sims) {
// 	// 					sim.toggle(u);
// 	// 					changes = true;
// 	// 					break;
// 	// 				}
// 	// 			}
// 	// 		} // END FOR-LOOP v


// 	// 		// check for emptiness
// 	// 		if (sim.empty(u)) {
// 	// 			Karla << "node " << q(u).getName() << " cannot be simulated" << "\n";
// 	// 		}
// 	// 	} // END FOR-LOOP v
// 	// 	// if (iter-1 % 10 == 0)
// 	// 	// 	cout << "#iteration " << iter << endl;
// 	// }
// 	// cout << (sim.empty() ? "empty" : "not empty") << endl;
// 	// Karla.end("computing dual sim");
// }

// // assume initialized simulation
// void dualSim(Graph &q, Graph &ball, Simulation &sim) {
// 	// Initialize simulation with |q| rows and |ball| columns
// 	// sim.initialize(q.size(), ball.size());
// 	// sim.initialize(q, ball);
// 	// sim.initialize(q.size(), q.groupSize(), ball.size());

// 	// cout << sim << endl;

// 	// runtime variables
// 	bool changes = true; // tracks the changes in one iteration
// 	bool sims; // tracks the simulation status for one edge ulux

// 	while (changes) {
// 		// so far, nothing changes
// 		changes = false;

// 		// for each pattern node u, find those nodes v
// 		// pretending to simulate u
// 		for (unsigned int u = 0; u < q.size(); ++u) {
// 			for (unsigned int g = 0; g < q.groupSize(); g++) {
// 				if (q.groupEmpty(g))
// 					continue;
// 				// cout << q.printGroup(g) << endl;
// 				// cout << "here " << u << " " << g << endl;

// 				string sub = q.getNode(u).getName();
// 				// cout << "checking " << sub << endl;

// 				// candidate simulators v for u
// 				for (unsigned int v = sim.init(u, g); v < sim.max(); v = sim.next(u, g)) {
// 					// for each edge u -l-> u'
// 					for (auto &ulux : q._postt[u]) {
// 						unsigned int lid = ulux.first;
// 						string lab = q._Sigma[lid]->str();

// 						sims = false; // initialize sims for ulux

// 						for (unsigned int ux = 0; ux < ulux.second.size(); ++ux) {
// 							unsigned int uxid = ulux.second[ux];
// 							string obj = q.getNode(uxid).getName();

// 							// if ulux is not a group edge, continue with next edge
// 							if (!q.inGroup(sub, lab, obj, g)) {
// 								sims = true;
// 								continue;
// 							}

// 							sims = false; // initialize sims for specific neighbor ux

// 							// does there exist a v' s.t. v -l-> v' and sim(u',v') in g
// 							if (ball._postt[v].count(ball._rSigma[lab])) {
// 								for (unsigned int vx = 0; vx < ball._postt[v][ball._rSigma[lab]].size(); ++vx) { // if one simulates edge, ok
// 									// check whether simulation holds
// 									if (sim(uxid, g, ball._postt[v][ball._rSigma[lab]][vx])) {
// 										sims = true;
// 										break;
// 									}
// 								}
// 							}
// 							// if u -l-> u' is not simulated, v does not simulate u
// 							// and something has changed
// 							if (!sims) {
// 								break;
// 							}
// 						}
// 						if (!sims) {
// 							sim.reset(u, g);
// 							changes = true;

// 							// SPARQL interna
// 							vector<unsigned int> compat = q.compatGroups(g);
// 							for (unsigned int ug = 0; ug < compat.size(); ++ug) {
// 								sim.reset(u, compat[ug]);
// 							}

// 							break;
// 						}
// 					}
// 					if (!sim(u,g,v)) // v could have been outsourced earlier
// 						continue;
// 					// Every backwards edge u'-l->u has a v'-l->v and sim(u',v')
// 					// for each edge u -l-> u'
// 					for (auto &ulux : q._pree[u]) {
// 						unsigned int lid = ulux.first;
// 						string lab = q._Sigma[lid]->str();
// 						sims = false; // initialize sims for ulux
// 						for (unsigned int ux = 0; ux < ulux.second.size(); ++ux) {
// 							unsigned int uxid = ulux.second[ux];
// 							string obj = q.getNode(uxid).getName();
// 							if (!q.inGroup(obj, lab, sub, g)) {
// 								sims = true;
// 								continue;
// 							}
// 							// cout << *ulux << endl;
// 							sims = false; // initialize sims for specific neighbor ux
// 							// does there exist a v' s.t. v -l-> v' and sim(u',v')
// 							if (ball._pree[v].count(ball._rSigma[lab])) {
// 								for (unsigned int vx = 0; vx < ball._pree[v][ball._rSigma[lab]].size(); ++vx) { // if one simulates edge, ok
// 									// cout << "checking " << *vlvx << endl;
// 									if (//ulux->label() == vlvx->label() &&
// 										sim(uxid, g, ball._pree[v][ball._rSigma[lab]][vx])) {
// 										sims = true;
// 										break;
// 									}
// 								}
// 							}
// 							// if u -l-> u' is not simulated, v does not simulate u
// 							// and something has changed
// 							if (!sims) {
// 								break;
// 							}
// 						}
// 						if (!sims) {
// 							sim.reset(u, g);
// 							changes = true;

// 							// SPARQL interna
// 							vector<unsigned int> compat = q.compatGroups(g);
// 							for (unsigned int ug = 0; ug < compat.size(); ++ug) {
// 								sim.reset(u, compat[ug]);
// 							}
// 							break;
// 						}
// 					}

// 				} // END FOR-LOOP v


// 				// cout << sim << endl;
// 				// check for emptiness
// 				// if (sim.empty(u)) {
// 				// 	Karla << "node " << q(u).getName() << " cannot be simulated" << "\n";
// 				// }
// 			} // END FOR-LOOP g
// 		} // END FOR-LOOP u
// 		// if (iter-1 % 10 == 0)
// 		// 	cout << "#iteration " << iter << endl;
// 	}

// 	// cout << sim << endl;
// }
