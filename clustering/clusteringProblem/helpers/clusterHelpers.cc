#include <iostream>
#include <vector>
#include <list>
#include "../../utils/utils.h"
#include "../cluster.h"

using namespace std;

/////////////////////////
/* Objective functions */
/////////////////////////
double getNextObjective1(double prevVal, double dist){
    double tmp;

    tmp = dist * dist;

    return prevVal + tmp;
}

double getNextObjective2(double prevVal, double dist){
    return prevVal + dist;
}


/* Helper function for constructor */
/* Check for errors and initialize */ 
/* some members                    */
void cluster::fixCluster(errorCode& status, std::list<Item>& items, int numClusters, std::string initAlgo, std::string assignAlgo, std::string updateAlgo, std::string metrice, int maxIter, double tol){

    status = SUCCESS;

    /* Check parameters */
    if(numClusters < MIN_CLUSTERS || numClusters > MAX_CLUSTERS){
        status = INVALID_CLUSTERS;
        return;
    }

    if(tol < MIN_TOL || tol > MAX_TOL){
        status = INVALID_TOL;
        return;
    }

    if((items.size() / numClusters) > 2){
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

    list<Item>::iterator iterItems = items.begin();
    int i;

    this->currStateVal = -1;

    /* Set distance function */
    if(metrice == "euclidean")
        this->distFunc = &euclideanDistance;
    else
        this->distFunc = &cosineDistance;

    /* Set objective function */
    if(updateAlgo == "k-means")
        this->objFunc = &getNextObjective1;
    else
        this->objFunc = &getNextObjective2;

    /* Check dim */
    this->dim = iterItems->getDim();
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
    for(iterItems = items.begin(); iterItems != items.end(); iterItems++){

        /* Check consistency of dim */
        if(this->dim != iterItems->getDim()){
            status = INVALID_POINTS;
            return;
        }

        /* Add item */
        this->items.push_back(*iterItems);
        this->itemsClusters.push_back(-1);
    } // End for

    /* Initialize clustersItems vector */
    for(i = 0; i < this->numClusters; i++)
        this->clustersItems.push_back(list<int>());
}


/* Perform upper bound algorithm like stl's algorithms. Return index of given value */
/* Given vector must be sorted                                                      */
/* Form of vector: [[0, 0], [val2, pos2], ...., [valn, posn]                        */
/* Form of vector: x[i][1] == pos, x[i], x[j] can have the same pos                 */
/* !!! Do not call this function with different form of vector                      */
int cluster::myUpperBound(vector<vector<double> >& x, double val, errorCode& status){

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

    /*  Check parameters */
    if(x.size() < 2 || val < 0){
        status = INVALID_PARAMETERS;
        return -1;
    }

    if(x[0].size() < 1 || x[0][0] != 0){
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
    double result = 0; // Average distance
    double tmpDist = 0;
    int flag = 0, clusterSize = 0;

    status = SUCCESS;

    /* Check parameters */
    if(itemPos < 0 || itemPos >= this->n || itemClusterPos < 0 || itemClusterPos >= this->numClusters){
        status = INVALID_PARAMETERS;
        return -1;
    }

    clusterSize = this->clustersItems[itemClusterPos].size();

    /* Scan items of give cluster */
    for(iterClusterItems = this->clustersItems[itemClusterPos].begin(); iterClusterItems != this->clustersItems[itemClusterPos].end(); iterClusterItems++){

        /* Discard same item */
        if(itemPos == *iterClusterItems){
            flag = 1;
            continue;
        }

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

    /* One item in cluster */
    if((flag == 1 && clusterSize == 1) || clusterSize == 0)
        return 0;

    /* Fix average distance aka result               */
    /* FLag == 1: Given item belongs to this cluster */
    if(flag == 1)
        result /= clusterSize - 1;
    else
        result /= clusterSize;

    return result;
}

/* Initialize radius for range search */
/* Minimum distance between centers   */
void cluster::initRadius(double& radius, errorCode& status){
    int i, j, flag = 0, clustersSize;
    double minDist = 0, tmpDist = 0;

    status = SUCCESS;

    clustersSize = this->clusters.size();

    /* Check model */
    if(this->fitted == -1 || clustersSize != this->numClusters){
        status = INVALID_METHOD;
        return;
    }

    if(this->fitted == 1){
        status = METHOD_ALREADY_USED;
        return;
    }

    /* Scan centers */
    for(i = 0; i < clustersSize; i++){

        /* Scan other centers */
        for(j = 0; j < clustersSize; j++ ){
            /* Discard same cluster */
            if(i == j)
                continue;

            /* Find current distance */
            tmpDist = this->distFunc(this->clusters[i], this->clusters[j], status);
            if(status != SUCCESS)
               return;

            /* Fix min dist */
            if(flag == 0){
                minDist = tmpDist;
                flag = 1;
            }
            else if(minDist > tmpDist)
                minDist = tmpDist;
        } // End for
    } // End for

    /* Set radius */
    radius = minDist  + (minDist * 0.75);
}
// Petropoulakis Panagiotis
