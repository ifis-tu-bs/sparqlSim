# sparqlSim


## Getting Started
First make sure that you have the following packages installed:
```
bison flex gengetopt
```

Make sure the following programs are installed:
```
autoconf
```

## Building the project
Switch to the project directory and execute:
```
1. autoreconf -i
2. ./configure
3. make
```

We build our software using the latest gcc compiler (tested on gcc version 7.3.0).

## Using the software
```
./sparqlSim --shell DATABASEFILE1 DATABASEFILE2 ... DATABASEFILEk
... loads one graph from all database files and starts interactive console
    (may use regular expressions in filenames)
```

### Store and Load
Usually, the program reads NT-files and internally optimizes the its graph structure for querying purposes. Since version 0.9 of sparqlSim, you get the opportunity to also store your favorite databases in a format more similar to the program's internal representation of it. Therefore, simply call
```
./sparqlSim <ANY PARAMETERS> -s DBDIR DBFILE1 .. DBFILEn
```
The DBDIR must be an existing directory. Upon loading the NT-files, the program writes a couple of files into the DBDIR. If you rerun the program and you want to load the stored data from DBDIR, you call
```
./sparqlSim <ANY PARAMETERS> -l DBDIR
```
At the moment, only one database directory is supported for loading.

### Evaluation Modes
Since version 0.9, we support several different modes beyond the original pruning algorithm. Therefore, parameter '-e MODE' specifies the evaluation mode MODE. By default, MODE equals PRUNE which lets the program perform the original pruning algorithm. Further modes are
- DUAL: computes the dual simulation matches
- HHK: computes the pruning of BGPs by the HHK algorithm
- MAETAL: computes the pruning of BGPs by the algorithm of Ma et al.
- STRONG: computes the strong simulation results
- CSTRONG: computes the strong simulation results with better heuristics
By default, the only output that is produced is the number of results computed by the program. Specify your favorite output stream by parameter '-o STREAM'.

## Important parameters for evaluation purposes

Whenever you want evaluation results, such as compile and other running times, make sure to include the parameter ``--csv`` in your parameter list.

### BGPs and Other Simulation Algorithms
Usually, other algorithms compute simulations between two graphs, not a query and a graph. Therefore, the parameter ``--bgp`` forces the parser to ignore any SPARQL operator structure and just read the triples.

Our HHK implementation comes in two modes, one employing the 'count' improvement and one that does not use it. The parameter ``--no-count`` is usable with the evaluation mode ``HHK`` and does not make use of the count data structures.

### Repeat ICDE 2019 Experiments
Call the program as follows:
```
./sparqlSim --icde2019 DATABASEFILE1 .. DATABASEFILEk -f QUERYFILE1 .. -f QUERYFILEn
```
Here, for every query file 'filename.sparql' and every query in that file, the pruning is written to 'filename_#.reduced.nt' where # is the line number the query occurs in the file. Furthermore, 'filename_#.csv' contains statistics for that query file (1 line per query). Also, virtuoso scripts are generated; (1) 'filename.virtuoso.profile.script' and (2) 'filename_#.virtuoso.script'.

### Other Dual Simulation Algorithms
Alongside our own solution we have implemented the HHK algorithm by Henzinger et al. (FOCS 1995) as well as the algorithm presented by Ma et al. (ACM TODS 2014). For evaluation of SPARQL queries by Ma et al.'s algorithm call
```
./sparqlSim --maetal DATABASEFILE1 ..DATABASEFILEk -f QUERFILE1 .. -f QUERYFILEn
```
For evaluation under HHK call
```
./sparqlSim --hhk DATABASEFILE1 .. DATABASEFILEk -f QUERYFILE1 .. -f QUERYFILEn
```
Please note that the for both algorithms the queries are assumed to be basic graph patterns. Hence, if optional patterns and further operators are used, then our own algorithm yields different results, because we process the SPARQL operators.

## Interactive console

upon 'input> ':
```
  1. import more files (command 'import')
  2. SPARQL query evaluation (simply start writing queries by prefix or select)
  3. prepare file with one query per line and query them line-by-line (command 'query')
```

In cases 2. and 3., the program derives filenames from given ones (2. it is 'inline_query') and each query produces two distinct files:
```
  .statistics .. contains query evaluation and db statistics
  .reduced.nt .. contains the pruning result
```


Please note that upon restart of the program, existing output files will be rewritten. So please backup them when necessary before running the next queries.

A technial report is available at http://www.ifis.cs.tu-bs.de/sites/default/files/DualSimulationTR.pdf


### Thanks

Thanks to http://bitmagic.io for providing compressed bit-vector containers.


