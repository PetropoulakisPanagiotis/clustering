#include <iostream>
#include <vector>
#include <list>
#include "../utils/utils.h"
#include "cluster.h"

using namespace std;

/* Find average distance between given item and items in the given cluster */
/* Check given vector(distances) for better performance                    */
double cluster::findItemAvgDist(int itemPos, int itemClusterPos, vector<vector<double> >& calculatedDistances, errorCode& status){
    list<int>::iterator iterClusterItems;
    double result = 0;
    double tmpDist = 0;

    status = SUCCESS;

    /* Check parameters */
    if(itemPos < 0 || itemPos >= this->n || itemClusterPos < 0 || itemClusterPos >= this->numClusters){
        status = INVALID_PARAMETERS;
        return -1;
    }

    /* Scan items of give cluster */
    for(iterClusterItems = this->clustersItems[itemClusterPos].begin(); iterClusterItems != this->clustersItems[itemClusterPos].end(); iterClusterItems++){

        /* Discard same item */
        if(itemPos == *iterClusterItems)
           continue;

        /* Check calculated distances - Array is symetric */
        /* Distance have been calculated                  */
        if(calculatedDistances[*iterClusterItems][itemPos] != -1){
            tmpDist = calculatedDistances[*iterClusterItems][itemPos];
        }
        else{
            /* Find current distance */
            tmpDist = this->distFunc(this->items[itemPos], this->items[*iterClusterItems], status);
            if(status != SUCCESS)
               return -1;

            /* Save distance */
            calculatedDistances[itemPos][*iterClusterItems] = tmpDist;
        }

        /* Increase result */
        result = mySumDouble(tmpDist, result, status);
        if(status != SUCCESS)
            return -1;
    } // End for items in cluster

    /* Fix result */
    result = myDivDouble(result, this->clustersItems[itemClusterPos].size(), status);
    if(status != SUCCESS)
            return -1;

    return result;
}

// Petropoulakis Panagiotis
