# sparqlSim


## Getting Started
First make sure that you have the following packages installed:
```
bison flex gengetopt libboost-dev libboost-all-dev
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

We build our software using the latest gcc compiler. 

## Using the software
```
./sparqlSim --shell DATABASEFILE1 DATABASEFILE2 ... DATABASEFILEk
... loads one graph from all database files and starts interactive console
    (may use regular expressions in filenames)
```
## Important parameters for evaluation purposes 
Call the program as follows:
```	
./sparqlSim --icde2019 DATABASEFILE1 .. DATABASEFILEk -f QUERYFILE1 .. -f QUERYFILEn
```
Here, for every query file 'filename.sparql' and every query in that file, the pruning is written to 'filename_#.reduced.nt' where # is the line number the query occurs in the file. Furthermore, 'filename_#.csv' contains statistics for that query file (1 line per query). Also, virtuoso scripts are generated; (1) 'filename.virtuoso.profile.script' and (2) 'filename_#.virtuoso.script'.

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


### Thanks

Thanks to http://bitmagic.io for providing compressed bit-vector containers.


