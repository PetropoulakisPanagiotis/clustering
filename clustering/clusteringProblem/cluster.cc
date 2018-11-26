#include <iostream>
#include <vector>
#include <list>
#include <unordered_set>
#include <random>
#include <chrono>
#include "../utils/utils.h"
#include "cluster.h"

using namespace std;

/////////////////////////////////////
/* Implementation of class cluster */
/////////////////////////////////////

/* Init cluster: Check for errors, set items and other members */
cluster::cluster(errorCode& status, list<Item>& items, int numClusters, string initAlgo, string assignAlgo, string updateAlgo, string metrice, int maxIter):numClusters(numClusters), maxIter(maxIter), initAlgo(initAlgo), assignAlgo(assignAlgo), updateAlgo(updateAlgo), metrice(metrice){

    status = SUCCESS;

    this->fitted = -1;

    /* Check parameters */
    if(numClusters < MIN_CLUSTERS || numClusters > MAX_CLUSTERS){
        status = INVALID_CLUSTERS;
        return;
    }

    if(initAlgo != "random" && initAlgo != "k-means++"){
        status = INVALID_ALGO;
        return;
    }

    if(assignAlgo != "lloyd" && assignAlgo != "range-lsh" && assignAlgo != "range-hypercube"){
        status = INVALID_ALGO;
        return;
    }

    if(updateAlgo != "k-means" && updateAlgo != "pam-lloyd"){
        status = INVALID_ALGO;
        return;
    }

    if(metrice != "euclidean" && metrice != "cosine"){
        status = INVALID_METRICE;
        return;
    }

    if(maxIter < MIN_ITER || maxIter > MAX_ITER){
        status = INVALID_ITER;
        return;
    }

    /////////////////
    /* Set members */
    /////////////////

    list<Item>::iterator iter = items.begin();
    int i;

    this->currStateVal = 0;

    /* Set distance function */
    if(metrice == "euclidean")
        this->distFunc = &euclideanDistance;
    else
        this->distFunc = &cosineDistance;

    /* Check dim */
    this->dim = iter->getDim();
    if(this->dim <= 0 || this->dim > MAX_DIM){
        status = INVALID_DIM;
        return;
    }

    /* Set number of items */
    this->n = items.size();
    if(this->n < MIN_POINTS || this->n > MAX_POINTS){
        status = INVALID_POINTS;
        return;
    }

    /* Fix vectors */
    this->items.reserve(this->n);
    this->itemsClusters.reserve(this->n);
    this->clusters.reserve(this->numClusters);

    /* Copy items */
    for(iter = items.begin(); iter != items.end(); iter++){

        /* Check consistency of dim */
        if(this->dim != iter->getDim()){
            status = INVALID_POINTS;
            return;
        }

        /* Add item and initialize clusters for items */
        this->items.push_back(*iter);
        this->itemsClusters.push_back(-1);
    } // End for

    /* Initialize clustersItems vector */
    for(i = 0; i < this->numClusters; i++)
        this->clustersItems.push_back(list<int>());

    /* No error occured */
    this->fitted = 0;
}

/* Compute clustering */
void cluster::fit(vector<Item>& clusters, vector<int>& clustersSize, errorCode& status){
    int step;
    int terminate, itemPos, clusterPos;

    status = SUCCESS;

    /* Check model */
    if(this->fitted == -1){
        status = INVALID_METHOD;
        return;
    }

    if(this->fitted == 1){
        status = METHOD_ALREADY_USED;
        return;
    }


    /* Clear vectors */
    clusters.clear();
    clustersSize.clear();

    ///////////////////////////
    /* Select init algorithm */
    ///////////////////////////

    if(initAlgo == "random")
        this->randomInit();
    else if(initAlgo == "k-means++")
        this->kmeansPlusInit(status);

    /* Error occured */
    if(status != SUCCESS)
        return;

    /* Perform clustering                             */
    /* Max iter: Is the strongest terminate condition */
    for(step = 0; step < this->maxIter; step++){

        /////////////////////////////////
        /* Select assignment algorithm */
        /////////////////////////////////

        if(this->assignAlgo == "lloyd")
            terminate = lloydAssign(status);

        /* Error occured */
        if(status != SUCCESS)
            return;

        /* Clusters have been determined */
        if(terminate == 1)
           break;


        /////////////////////////////
        /* Select update algorithm */
        /////////////////////////////

        if(this->updateAlgo == "k-means")
            kmeans(status);
        else if(this->updateAlgo == "pam-lloyd")
            pamLloyd(status);

        /* Error occured */
        if(status != SUCCESS)
            return;
    } // End while

    /* Map clusters with items for fast calculations(silhouette etc.) */
    for(itemPos = 0; itemPos < this->n; itemPos++){
        this->clustersItems[this->itemsClusters[itemPos]].push_back(itemPos);
    } // End for items

    /* Set parameters */
    for(clusterPos = 0; clusterPos < this->numClusters; clusterPos++){
        clusters.push_back(this->clusters[clusterPos]);
        clustersSize.push_back(this->clustersItems[clusterPos].size());
    }

    /* Success */
    this->fitted = 1;
}

/* Calculate average silhouette of cluster */
void cluster::getSilhouette(vector<double>& silhouetteArray, errorCode& status){
    list<int>::iterator iterClusterItems; // Iterate through indexes ofitems in each cluster
    vector<vector<double> > calculatedDistances; // Save visited distances
    int itemDistancesPos, itemCurrentDistancePos, i; // Iterate through static vectors
    int itemPos = 0, itemClusterPos = 0, clusterPos = 0, flag = 0, itemSecondClusterPos = 0;
    double a = 0, b = 0;
    double tmpDist = 0, minDist = 0, tmpDouble = 0;

    status = SUCCESS;

    /* Check model */
    if(this->fitted == -1){
        status = INVALID_METHOD;
        return;
    }

    if(this->fitted == 0){
        status = METHOD_UNFITTED;
        return;
    }

    /* Init and clear list */
    silhouetteArray.clear();
    for(i = 0; i < this->numClusters + 1; i++)
        silhouetteArray.push_back(0);

    //////////////////////////
    /* Calculate silhouette */
    //////////////////////////

    /* Initialize calculated distances */
    for(itemDistancesPos = 0; itemDistancesPos < this->n; itemDistancesPos++){
        calculatedDistances.push_back(vector<double>());

        for(itemCurrentDistancePos = 0; itemCurrentDistancePos < this->n; itemCurrentDistancePos++){
            calculatedDistances[itemDistancesPos].push_back(-1);
        }
    } // End for

    /* Scan all items */
    for(itemPos = 0; itemPos < this->n; itemPos++){

        /* Resets */
        a = 0;
        b = 0;
        flag = 0;

        /* Find position of the item's cluster */
        itemClusterPos = this->itemsClusters[itemPos];

        /* Find second nearest cluster */
        for(clusterPos = 0; clusterPos < this->numClusters; clusterPos++){

            /* Skip cluster of current item */
            if(clusterPos == itemClusterPos)
                continue;

            /* Find current distance */
            tmpDist = this->distFunc(this->items[itemPos], this->clusters[clusterPos], status);
            if(status != SUCCESS)
                return;

            if(flag == 0){
                minDist = tmpDist;
                itemSecondClusterPos = clusterPos;
                flag = 1;
            }
            else if(minDist > tmpDist){
                minDist = tmpDist;
                itemSecondClusterPos = clusterPos;
            }
        } // End for clusters

        /* Find a - Average distance between current item and other items in the main cluster */
        a = this->findItemAvgDist(itemPos, itemClusterPos, calculatedDistances, status);
        if(status != SUCCESS)
            return;

        /* Find b - Average distance between current item and other items in the second best cluster */
        b = this->findItemAvgDist(itemPos, itemSecondClusterPos, calculatedDistances, status);
        if(status != SUCCESS)
            return;

        ////////////////////
        /* Fix silhouette */
        ////////////////////
        tmpDouble = b - a;

        if(a > b)
            tmpDouble /=  a;
        else
            tmpDouble /=  b;

        /* Add current s(itemPos) */
        silhouetteArray[this->itemsClusters[itemPos]] += tmpDouble;

        /* Fix overall silhouette */
        silhouetteArray[this->numClusters] += tmpDouble;
    } // End for items

    int tmpSize;

    /* Fix silhouette for every cluster */
    for(i = 0; i < this->numClusters; i++){
        tmpSize = this->clustersItems[i].size();
        if(tmpSize != 0)
            silhouetteArray[i] /= this->clustersItems[i].size();
    } // End for

    /* Fix overall silhouette */
    silhouetteArray[this->numClusters] /= this->n;
}

/* Get index of cluster for items */
void cluster::getItemsCluster(list<int>& itemsPos, errorCode& status){
    int itemPos;

    status = SUCCESS;

    /* Check model */
    if(this->fitted == -1){
        status = INVALID_METHOD;
        return;
    }

    if(this->fitted == 0){
        status = METHOD_UNFITTED;
        return;
    }


    items.clear();

    for(itemPos = 0; itemPos < this->n; itemPos++)
        itemsPos.push_back(this->itemsClusters[itemPos]);
}
// Petropoulakis Panagiotis
