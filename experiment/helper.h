#pragma once
#include <list>
#include <fstream>
#include "../clustering/item/item.h"
#include "../clustering/clusteringProblem/cluster.h"

/*  Read arguments */
int readArguments(int argc, char **argv, std::string& inputFile, std::string& confFile, std::string& outputFile, std::string& metrice);

/* Run model */
void runModel(cluster* myCluster, fstream& outputFile, std::string& initAlgo, std::string& assignAlgo, std::string& updateAlgo, std::string& metrice, int numClucsters);
// Petropoulakis Panagiotis
