#include <iostream>
#include <vector>
#include <list>
#include <unordered_set>
#include <random>
#include <chrono>
#include <algorithm>
#include "../utils/utils.h"
#include "cluster.h"

using namespace std;

///////////////
/* Distances */
///////////////

double distCosine(Item& x, Item& y, errorCode& status){
    status = SUCCESS;

    return x.cosineDist(y, status);
}

double distEuclidean(Item& x, Item& y, errorCode& status){
    status = SUCCESS;

    return x.euclideanDist(y, status);
}

////////////////////
/* Init functions */
////////////////////

/* Select uniformly random clusters */
void cluster::randomInit(void){
    /* Set random engine */
    unsigned seed = chrono::system_clock::now().time_since_epoch().count(); 
    default_random_engine generator(seed);

    /* Set distribution */
    uniform_int_distribution<int> uniformDist(0, this->n);

    /* Do not pick same cluster twice */
    unordered_set<int> visited;
    unordered_set<int>::iterator iterVisited;
    int currPos;
    int i;

    for(i = 0; i < this->numClusters; i++){

        /* Find new cluster */
        currPos = uniformDist(generator);

        /* Check visited */
        iterVisited = visited.find(currPos);

        if(iterVisited == visited.end()){
            this->clusters.push_back(this->items[currPos]);
            visited.insert(currPos);
        }
        /* Try again */
        else{
            i -= 1;
            continue;
        }
    } // End for
}

/* Select clusters based on kmeans++ algorithm */
void cluster::kmeansPlusInit(errorCode& status){
    /* Set random engine */
    unsigned seed = chrono::system_clock::now().time_since_epoch().count(); 
    default_random_engine generator(seed);

	/* Do not pick the same cluster twice */
    unordered_set<int> visited;
    unordered_set<int>::iterator iterVisited;

    /* Set distribution */
    static uniform_int_distribution<int> uniformDist(0, this->n);
   
    int clusterPos, itemPos, i, newClusterPos;
    double currDist, minDist, tmpDouble, randomDouble;
    
    /* Keep distances between items and minimum cluster */
    vector<double> minCalculatedDistances; 
	vector<double>::iterator iterDouble;   
 
    /* Pick initial cluster */
    itemPos = uniformDist(generator);
    this->clusters.push_back(this->items[itemPos]); 

	/* Fix visited */
    visited.insert(itemPos);

    /* Find initial distances: first cluster and items */
    for(itemPos = 0; itemPos < this->n; itemPos++){
        /* Find dist */
        currDist = this->distFunc(this->items[itemPos], this->clusters[0], status);
        if(status != SUCCESS)
            return;
        minCalculatedDistances.push_back(currDist);
    } // End for items
                                                                     
    /* Pick remaining clusters */
    for(clusterPos = 1; clusterPos < this->numClusters; clusterPos++){

        /* List of partial sums - Reset */
        vector<double> partialSums;
        
        /* Add zero */
        partialSums.push_back(0);
        
        /* For every item find min cluster and calculate dist(item,minCluster)^2 */
        for(itemPos = 0; itemPos < this->n; itemPos++){
	
			/* Discard clusters */
    	    iterVisited = visited.find(itemPos);
	
    	    if(iterVisited != visited.end()){
				partialSums.push_back(*partialSums.end()); // Add previous partial sum
				continue;	
			}
			
			/* Set as initial min distance the previous one */
			minDist = minCalculatedDistances[itemPos];
	
			/* Find new minimum distance between clusters and current item */
			for(i = 1; i < clusterPos; i++){
		        /* Find distance for current cluster */
        		currDist = this->distFunc(this->items[itemPos], this->clusters[i], status);
        		if(status != SUCCESS)
            		return;

				/* Set min distance */
				if(currDist < minDist)
					minDist = currDist;
			} // End for find min cluster 
			
			/* Set new min distances */
			minCalculatedDistances[itemPos] = minDist;
	
			/* Set partialSums */

			/* Find: D(i)^2 */			
			tmpDouble = myMultDouble(minDist, minDist, status);
        	if(status != SUCCESS)
            	return;

			/* Find current partial sum - add previous partial sum with current */
			tmpDouble = mySumDouble(tmpDouble, partialSums[itemPos], status);
   	       	if(status != SUCCESS)
            	return; 
			
			partialSums.push_back(tmpDouble);		
        } // End for items

		/* Pick a random value between [0-p(n-t)] */
    	uniform_real_distribution<double> uniformDist1(0, *partialSums.end());

		/* Perform binary search and pick new cluster */
		while(1){
			
			/* Find random double */
			randomDouble = uniformDist1(generator);

			/* Find position on new cluster */
			iterDouble = upper_bound(partialSums.begin(), partialSums.end(), randomDouble);
			newClusterPos = *iterDouble;

			/* Discard same clusters */
    	    iterVisited = visited.find(newClusterPos);

			/* Cluster has not been selected - pick this cluster */	
    	    if(iterVisited == visited.end()){
				break;	
			}
		} // End while
    } // End for remaining clusters 
}

/////////////////////////
/* Assigment functions */
/////////////////////////

/* Assign items with Lloyd's method */
/* Success: 0                       */
/* Terminate: 1                     */
/* Failure: -1                      */
int cluster::lloydAssign(errorCode& status){
    double newObjVal = 0, tmpObjVal;
    int itemPos = 0, clusterPos = 0; // Indexes
    double minDist = 0, currDist = 0 ; // Distances
    int minClusterPos = 0; // Index of cluster

    status = SUCCESS;

    /* Assign every item to the nearest cluster */
    for(itemPos = 0; itemPos < this->n; itemPos++){

        /* Scan clusters */
        for(clusterPos = 0; clusterPos < this->numClusters; clusterPos++){

            /* Find current distance */
            currDist = this->distFunc(this->items[itemPos], this->clusters[clusterPos], status);
            if(status != SUCCESS)
                    return -1;

            /* Set min dist */
            if(clusterPos == 0){
                minDist = currDist;
                minClusterPos = 0;
            }
            else if(currDist < minDist){
                minDist = currDist;
                minClusterPos = clusterPos;
            }
        } // End for - clusters

        /* Calculate objective function */
        tmpObjVal = myMultDouble(minDist, minDist, status);
        if(status != SUCCESS)
            return -1;

        newObjVal = mySumDouble(tmpObjVal, newObjVal, status);
        if(status != SUCCESS)
            return -1;

        /* Assign item to cluster */
        this->itemsClusters[itemPos] = minClusterPos;
    } // End for - items

    /* Check terminate conditions */
    if(this->objVal == newObjVal)
        return 1;

    /* Set prevObjVal */
    this->objVal = newObjVal;

    return 0;
}

//////////////////////
/* Update functions */
//////////////////////

/* Update: kmeans */
void cluster::kmeans(errorCode& status){
    vector<int> flags; // Clear clusters
    vector<int> sizeOfClusters;
    int itemPos, componentPos; // Indexes
    int clusterPos;
    double newComponent;

    status = SUCCESS;

    /* Clear flags and set size per cluster*/
    for(clusterPos = 0; clusterPos < this->numClusters; clusterPos++){
        flags.push_back(0);
        sizeOfClusters.push_back(0);
    }

    /* Scan all items */
    for(itemPos = 0; itemPos < this->n; itemPos++){

        /* Find position of the item's cluster */
        clusterPos = this->itemsClusters[itemPos];

        /* Reset cluster only once */
        if(flags[clusterPos] == 0){
            this->clusters[clusterPos].clearComponents();
            flags[clusterPos] = 1;
        }

        /* Scan components of item */
        for(componentPos = 0; componentPos < this->dim; componentPos++){

            /* Get current component */
            newComponent = this->items[itemPos].getComponent(componentPos, status);
            if(status != SUCCESS)
                return;

            /* Add new component in current cluster */
            this->clusters[clusterPos].sumComponent(newComponent, componentPos, status);
            if(status != SUCCESS)
                return;
        } // End for components

        sizeOfClusters[clusterPos] += 1;
    } // End for - Items

    /* Divide components of clusters with size */
    for(clusterPos = 0; clusterPos < this->numClusters; clusterPos++){
        if(flags[clusterPos] == 0)
            continue;

        /* Divide each component */
        for(componentPos = 0; componentPos < this->dim; componentPos++){

            this->clusters[clusterPos].divComponent(sizeOfClusters[clusterPos], componentPos, status);
            if(status != SUCCESS)
                return;
        } // End for components
    } // End for clusters
}


// Petropoulakis Panagiotis
