#pragma once
#include <vector>
#include <list>
#include "../utils/utils.h"
#include "../item/item.h"

/* Functions for reading data set and query set */

/* Read given file, extract items and read possible metrices(euclidean, cosine, etc) */
void readDataSet(std::string fileName, int withId, char delim, std::list<Item>& points, errorCode& status); 

/* Read config file */
void readConf(std::string fileName, int& numClusters, int& k, int& l, errorCode& status); 
// Petropoulakis Panagiotis
