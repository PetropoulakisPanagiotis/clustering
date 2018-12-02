#include <iostream>
#include <vector>
#include <list>
#include <unordered_set>
#include <random>
#include <chrono>
#include "../utils/utils.h"
#include "cluster.h"
#include "../neighborsProblem/model/lsh/lsh.h"
#include "../neighborsProblem/model/hypercube/hypercube.h"

using namespace std;

/////////////////////////////////////
/* Implementation of class cluster */
/////////////////////////////////////

/* Init cluster: Check for errors, set items and other members */
/* Range model: default constructor                            */
cluster::cluster(errorCode& status, list<Item>& items, int numClusters, string initAlgo, string assignAlgo, string updateAlgo, string metrice, int maxIter, double tol):numClusters(numClusters), maxIter(maxIter), tol(tol), initAlgo(initAlgo), assignAlgo(assignAlgo), updateAlgo(updateAlgo), metrice(metrice){

    status = SUCCESS;

    this->fitted = -1;

    /* Check parameters and initialize members items + clusters */
    fixCluster(status, items, numClusters, initAlgo, assignAlgo, updateAlgo, metrice, maxIter, tol);
    if(status != SUCCESS)
       return;

    /* Check for range search */
    if(assignAlgo == "range-lsh" && metrice == "euclidean")
        this->rangeModel = new lshEuclidean();
    else if(assignAlgo == "range-lsh" && metrice == "cosine")
        this->rangeModel = new lshCosine();
    else if(assignAlgo == "range-hypercube" && metrice == "euclidean")
        this->rangeModel = new hypercubeEuclidean();
    else if(assignAlgo == "range-hypercube" && metrice == "cosine")
        this->rangeModel = new hypercubeCosine();
    else
        this->rangeModel = NULL;

    if(assignAlgo != "lloyd" && this->rangeModel == NULL){
        status = ALLOCATION_FAILED;
        return;
    }

    /* Fit model */
    if(this->rangeModel != NULL){
        this->rangeModel->fit(items, status);
        if(status != SUCCESS)
            return;
    }

    /* Success */
    this->fitted = 0;
}

/* Init cluster: Check for errors, set items and other members */
/* Range model: custom constructor - only lsh                  */
cluster::cluster(errorCode& status, list<Item>& items, int k, int l, int numClusters, string initAlgo, string assignAlgo, string updateAlgo, string metrice, int w, float coefficientint, float coefficientRadius, int maxIter, double tol):numClusters(numClusters), maxIter(maxIter), tol(tol), coefficientRadius(coefficientRadius), initAlgo(initAlgo), assignAlgo(assignAlgo), updateAlgo(updateAlgo), metrice(metrice){

    status = SUCCESS;

    this->fitted = -1;

    /* Check parameters and initialize members items + clusters */
    fixCluster(status, items, numClusters, initAlgo, assignAlgo, updateAlgo, metrice, maxIter, tol);
    if(status != SUCCESS)
       return;

    /* Check coefficient radius */
    if(coefficientRadius < MIN_COEF_RADIUS || coefficientRadius > MAX_COEF_RADIUS){
        status = INVALID_COEF_RADIUS;
        return;
    }

    /* Check for range search */
    if(assignAlgo == "range-lsh" && metrice == "euclidean")
        this->rangeModel = new lshEuclidean(k, l, w, coefficientint, status);
    else if(assignAlgo == "range-lsh" && metrice == "cosine")
        this->rangeModel = new lshCosine(k, l, status);
    else if(assignAlgo == "range-hypercube" && metrice == "euclidean")
        status = INVALID_PARAMETERS;
    else if(assignAlgo == "range-hypercube" && metrice == "cosine")
        status = INVALID_PARAMETERS;
    else
        this->rangeModel = NULL;

    if(status != SUCCESS)
        return;

    if(assignAlgo != "lloyd" && this->rangeModel == NULL){
        status = ALLOCATION_FAILED;
        return;
    }

    /* Fit model */
    if(this->rangeModel != NULL){
        this->rangeModel->fit(items, status); 
        if(status != SUCCESS)
            return;
    }

    /* Success */
    this->fitted = 0;
}

/* Init cluster: Check for errors, set items and other members */
/* Range model: custom constructor - only hypercube            */
cluster::cluster(errorCode& status, list<Item>& items, int k, int numClusters, string initAlgo, string assignAlgo, string updateAlgo, string metrice, int m, int probes, int w, float coefficientRadius, int maxIter, double tol):numClusters(numClusters), maxIter(maxIter), tol(tol), coefficientRadius(coefficientRadius), initAlgo(initAlgo), assignAlgo(assignAlgo), updateAlgo(updateAlgo), metrice(metrice){

    status = SUCCESS;

    this->fitted = -1;

    /* Check parameters and initialize members items + clusters */
    fixCluster(status, items, numClusters, initAlgo, assignAlgo, updateAlgo, metrice, maxIter, tol);
    if(status != SUCCESS)
       return;

    /* Check coefficient radius */
    if(coefficientRadius < MIN_COEF_RADIUS || coefficientRadius > MAX_COEF_RADIUS){
        status = INVALID_COEF_RADIUS;
        return;
    }

    /* Check for range search */
    if(assignAlgo == "range-lsh" && metrice == "euclidean")
        status = INVALID_PARAMETERS;
    else if(assignAlgo == "range-lsh" && metrice == "cosine")
        status = INVALID_PARAMETERS;
    else if(assignAlgo == "range-hypercube" && metrice == "euclidean")
        this->rangeModel = new hypercubeEuclidean(k, m, probes, w, status);
    else if(assignAlgo == "range-hypercube" && metrice == "cosine")
        this->rangeModel = new hypercubeCosine(k, m, probes, status);
    else
        this->rangeModel = NULL;

    if(status != SUCCESS)
        return;

    if(assignAlgo != "lloyd" && this->rangeModel == NULL){
        status = ALLOCATION_FAILED;
        return;
    }

    /* Fit model */
    if(this->rangeModel != NULL){
        this->rangeModel->fit(items, status);
        if(status != SUCCESS)
            return;  
    }

    /* Success */
    this->fitted = 0;
}

/* Destructor: delete range model */
cluster::~cluster(){
    if(this->rangeModel)
        delete this->rangeModel;
}

/* Perform clustering             */
/* Return clusters and their size */
void cluster::fit(vector<Item>& clusters, vector<int>& clustersSize, errorCode& status){
    int step, terminate;
    int itemPos, clusterPos; // Indexes
    double radius; // For range search

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


    ////////////////////////////////////////
    /* Initialize radius for range search */
    ////////////////////////////////////////
    if(assignAlgo != "lloyd"){
        initRadius(radius, status);
        if(status != SUCCESS)
            return;
    }

    /* Perform clustering                             */
    /* Max iter: Is the strongest terminate condition */
    for(step = 0; step < this->maxIter; step++){

        /////////////////////////////////
        /* Select assignment algorithm */
        /////////////////////////////////

        if(this->assignAlgo == "lloyd")
            terminate = this->lloydAssign(status);
        else
            terminate = this->rangeAssign(radius, status);

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
            this->kmeans(status);
        else if(this->updateAlgo == "pam-lloyd")
            this->pamLloyd(status);

        /* Error occured */
        if(status != SUCCESS)
            return;
    } // End for - perform clustering

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

/* Return silhouette for every cluster and overall silhouette */
void cluster::getSilhouette(vector<double>& silhouetteArray, errorCode& status){
    list<int>::iterator iterClusterItems; // Iterate through clusters of items
    vector<vector<double> > calculatedDistances; // Save visited distances
    int itemDistancesPos, itemCurrentDistancePos, i; // Iterate through calculated distances
    int itemPos = 0, itemClusterPos = 0, clusterPos = 0; // Indexes
    int flag = 0, itemSecondClusterPos = 0, currentClusterSize = 0;
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

    /* Init and clear given vector */
    silhouetteArray.clear();
    for(i = 0; i < this->numClusters + 1; i++)
        silhouetteArray.push_back(0);

    /* Initialize calculated distances */
    for(itemDistancesPos = 0; itemDistancesPos < this->n; itemDistancesPos++){
        calculatedDistances.push_back(vector<double>());

        for(itemCurrentDistancePos = 0; itemCurrentDistancePos < this->n; itemCurrentDistancePos++){
            calculatedDistances[itemDistancesPos].push_back(-1);
        } // End for - Dist(currentItem, xItem)
    } // End for - Distances for current item

    //////////////////////////
    /* Calculate silhouette */
    //////////////////////////

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
        if(a > b){
            if(a != 0)
                tmpDouble /= a;
            else
                tmpDouble = 1;
        }
        else{
            if(b != 0)
                tmpDouble /= b;
            else
                tmpDouble = -1;
        }

        /* Fix s(itemPos) */
        silhouetteArray[this->itemsClusters[itemPos]] += tmpDouble;

        /* Fix overall silhouette */
        silhouetteArray[this->numClusters] += tmpDouble;
    } // End for items

    /* Fix silhouette for every cluster */
    for(i = 0; i < this->numClusters; i++){
        currentClusterSize = this->clustersItems[i].size();

        if(currentClusterSize != 0)
            silhouetteArray[i] /= currentClusterSize;
        else
            silhouetteArray[i] = -2; // Empty cluster. BAD K!
    } // End for - silhouette per cluster

    /* Fix overall silhouette */
    cout << silhouetteArray[this->numClusters] << "-" << this->n << "\n";
    silhouetteArray[this->numClusters] /= this->n;
}

/* Return for every item the index of it's cluster */
void cluster::predict(list<int>& itemsPos, errorCode& status){
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

/* Get id of method */
string cluster::getId(errorCode& status){
    string id = "";

    status = SUCCESS;

    /* Check model */
    if(this->fitted == -1){
        status = INVALID_METHOD;
        return "";
    }

    if(this->initAlgo == "random")
        id += to_string(1) + "x";
    else
        id += to_string(2) + "x";

    if(this->assignAlgo == "lloyd")
        id += to_string(1) + "x";
    else if(this->assignAlgo == "range-lsh")
        id += to_string(2) + "x";
    else
        id += to_string(3) + "x";

    if(this->updateAlgo == "k-means")
        id += to_string(1);
    else
        id += to_string(2);

    return id;
}
// Petropoulakis Panagiotis
