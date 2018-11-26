#include <iostream>
#include <vector>
#include <list>
#include "../utils/utils.h"
#include "cluster.h"

using namespace std;

/* Perform upper bound algorithm like stl's algorithms. Return index of given value */
/* Given vector must be sorted                                                      */
/* Form of vector: [[0, pos], [val2, pos2], ...., [valn, posn]                      */
/* !!! Do not call this function with different form of vector                      */
int cluster::myUpperBound(vector<vector<double> >& x, double val, errorCode& status){

    /*  Check parameters */
    if(x.size() < 2 || val < 0 || x[0][0] != 0){
        status = INVALID_PARAMETERS;
        return -1;
    }

    int middle, left = 0, right = x.size() - 1;

    if(val == 0)
        return x[1][1];

    while(1){

        /* Find middle */
        middle = (left + right) / 2;

        /* Check if val found */
        if(x[middle - 1][0] < val && x[middle][0] >= val)
            return x[middle][1];

        if(x[middle][0] < val && x[middle + 1][0] >= val)
            return x[middle + 1][1];

        /* Reach end of vector */
        if(left == right - 1)
            return x[right][1];

        /* Fix new left, right */
        if(x[middle][0] < val)
            left = middle;
        else
           right = middle;

    } // End while
}

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
        result += tmpDist;
    } // End for items in cluster

    /* Fix result */
    result /= this->clustersItems[itemClusterPos].size() - 1;

    return result;
}

// Petropoulakis Panagiotis
