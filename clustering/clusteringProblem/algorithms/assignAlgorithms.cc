#include <iostream>
#include <unordered_set> 
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

    /* Check model */
    if(this->fitted == -1){
        status = INVALID_METHOD;
        return -1;
    }

    if(this->fitted == 1){
        status = METHOD_ALREADY_USED;
        return -1;
    }

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
        newObjVal = this->objFunc(newObjVal, minDist);

        /* Assign item to it's nearest cluster */
        this->itemsClusters[itemPos] = minClusterPos;
    } // End for - items

    cout << this->currStateVal << "\n";
    cout << newObjVal << "\n\n";

    /* Check terminate condition */
    if(abs(this->currStateVal - newObjVal) < this->tol)
        return 1;

    /* Set prevObjVal */
    this->currStateVal = newObjVal;

    return 0;
}

/* Assign items with range method - lsh orhypercube                                */
/* Obj func: dist(item(0), minCLuster(0))^ 2 +  dist(item(n-1), minCluster(n-1))^2 */
/* Success: 0                                                                      */
/* Terminate: 1                                                                    */
/* Failure: -1                                                                     */
int cluster::rangeAssign(double& radius, errorCode& status){
    double newObjVal = 0, tmpObjVal = 0;
    int itemPos = 0, clusterPos = 0; // Indexes
    double minDist = 0, currDist = 0 ; // Distances
    int minClusterPos = 0; // Index of minimum cluster

    /* Keep distances between items and (current)minimum cluster */
    vector<double> minCalculatedDistances;

    /* Keep neighbors and distances */
    list<int> neighbors;
    list<int>::iterator iterNeighbors;
    list<double> neighborsDistances;
    list<double>::iterator iterNeighborsDistances;

    /* Keep truck the number of items that belog to one or more clusters and */
    /* the number of the unassigned items                                    */
    int itemsInManyClusters = 0;
    int unassignedItems = 0;

    status = SUCCESS;

    /* Check model */
    if(this->fitted == -1 || this->rangeModel == NULL){
        status = INVALID_METHOD;
        return -1;
    }

    if(this->fitted == 1){
        status = METHOD_ALREADY_USED;
        return -1;
    }

    /* Cleare clusters of items */
    for(itemPos = 0; itemPos < this->n; itemPos++){
        this->itemsClusters[itemPos] = -1;
        minCalculatedDistances.push_back(0);
    }


    ////////////////////////////////////////////
    /* Perform range search for every cluster */
    ////////////////////////////////////////////
    for(clusterPos = 0; clusterPos < this->numClusters; clusterPos++){

        /* Find neighbors */
        this->rangeModel->radiusNeighbors(this->clusters[clusterPos], radius, neighbors, &neighborsDistances, status);
        if(status != SUCCESS)
            return -1;

        /* Scan neighbors and distances */
        iterNeighborsDistances = neighborsDistances.begin();
        for(iterNeighbors = neighbors.begin(); iterNeighbors != neighbors.end(); iterNeighbors++){

            //////////////////////////////////////////////
            /* Check if item is assign to other cluster */
            //////////////////////////////////////////////

            /*  Check if current cluster is the nearest */
            if(this->itemsClusters[*iterNeighbors] != -1 && clusterPos != 0){

                /* Increase counter */
                itemsInManyClusters += 1;

                if(minCalculatedDistances[*iterNeighbors] > *iterNeighborsDistances){
                    minCalculatedDistances[*iterNeighbors] = *iterNeighborsDistances;

                    /* Change cluster */
                    this->itemsClusters[*iterNeighbors] = clusterPos;
                }
            }
            else{
                minCalculatedDistances[*iterNeighbors] = *iterNeighborsDistances;
                this->itemsClusters[*iterNeighbors] = clusterPos;
            }

            iterNeighborsDistances++;
        } // End for - neighbors
    } // End for - clusters

    /* Assign items with no clusters and calculate objective function */
    for(itemPos = 0; itemPos < this->n; itemPos++){

        if(this->itemsClusters[itemPos] == -1){

            unassignedItems += 1;

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
            newObjVal = this->objFunc(newObjVal, minDist);

            /* Assign item to it's nearest cluster */
            this->itemsClusters[itemPos] = minClusterPos;

        } // End if - Find min cluster
        else{ // Cluster exists

            /* Calculate objective function */
            tmpObjVal = minCalculatedDistances[itemPos] * minCalculatedDistances[itemPos];
            newObjVal += tmpObjVal;
        }
    } // End for - items
    cout << "Not ass: " << unassignedItems << "\n";
    cout << "sigrousis: " << itemsInManyClusters  << "\n\n";

    /* Check terminate condition */
    if(abs(this->currStateVal - newObjVal) < this->tol)
        return 1;

    /* Set prevObjVal */
    this->currStateVal = newObjVal;

    /* Set radius */
    double nextRadius = radius + (radius * this->coefficientRadius);

    /* Increase radius up to a point       */
    /* Stop increasing radius if there are */
    /* points in many clusters             */
    if(nextRadius <= MAX_RADIUS && unassignedItems > 30 &&  ((this->numClusters * this->n) - itemsInManyClusters >= this->n)){
        radius = nextRadius;
        cout << "yes\n";
    }

    return 0;
}

// Petropoulakis Panagiotis
