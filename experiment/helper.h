#pragma once
#include <list>
#include "../clustering/item/item.h"

/*  Read arguments */
int readArguments(int argc, char **argv, std::string& inputFile, std::string& confFile, std::string& outputFile, std::string& metrice);

/* Run model */
void runModel(std::list<Item>& items, std::string& outputFile, std::string& initAlgo, std::string& assignAlgo, std::string& updateAlgo, std::string& metrice, int numClucsters);
// Petropoulakis Panagiotis
