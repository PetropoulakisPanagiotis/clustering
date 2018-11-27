#include <iostream>
#include <iomanip>
#include "../../utils/utils.h"
#include "../cluster.h"

using namespace std;

//////////////////////////
/* Assignment functions */
//////////////////////////

/* Assign items with Lloyd's method                                                */
/* Obj func: dist(item(0), minCLuster(0))^ 2 +  dist(item(n-1), minCluster(n-1))^2 */
/* Success: 0                                                                      */
/* Terminate: 1                                                                    */
/* Failure: -1                                                                     */
int cluster::lloydAssign(errorCode& status){
    double newObjVal = 0, tmpObjVal = 0;
    int itemPos = 0, clusterPos = 0; // Indexes
    double minDist = 0, currDist = 0 ; // Distances
    int minClusterPos = 0; // Index of minimum cluster

    status = SUCCESS;

    /* Assign every item to it's nearest cluster */
    for(itemPos = 0; itemPos < this->n; itemPos++){

        /* Initialize minimum cluster */
        minDist = this->distFunc(this->items[itemPos], this->clusters[0], status);
        if(status != SUCCESS)
            return -1;
        minClusterPos = 0;

        /* Scan clusters and find nearest */
        for(clusterPos = 1; clusterPos < this->numClusters; clusterPos++){

            /* Find current distance */
            currDist = this->distFunc(this->items[itemPos], this->clusters[clusterPos], status);
            if(status != SUCCESS)
                return -1;

            if(currDist < minDist){
                minDist = currDist;
                minClusterPos = clusterPos;
            }
        } // End for - clusters

        /* Calculate objective function */
        tmpObjVal = minDist * minDist;
        newObjVal += tmpObjVal;

        /* Assign item to it's nearest cluster */
        this->itemsClusters[itemPos] = minClusterPos;
    } // End for - items

    cout << setprecision(15) << this->currStateVal << "\n";
    cout << setprecision(15) << newObjVal << "\n";

    /* Check terminate condition */
    if(this->currStateVal == newObjVal)
        return 1;

    /* Set prevObjVal */
    this->currStateVal = newObjVal;

    return 0;
}

// Petropoulakis Panagiotis
