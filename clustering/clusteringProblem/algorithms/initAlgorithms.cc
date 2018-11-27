#include <iostream>
#include <vector>
#include <unordered_set>
#include <random>
#include <chrono>
#include "../../utils/utils.h"
#include "../cluster.h"

using namespace std;

////////////////////
/* Init functions */
////////////////////

/* Select uniformly random initial clusters */
void cluster::randomInit(void){
    /* Set random engine */
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    default_random_engine generator(seed);

    /* Set distribution */
    uniform_int_distribution<int> uniformDist(0, this->n); // [0, n)

    /* Do not pick same cluster twice */
    unordered_set<int> visited;
    unordered_set<int>::iterator iterVisited;
    int currPos, i;

    /* Select clusters */
    for(i = 0; i < this->numClusters; i++){

        /* Find new cluster */
        currPos = uniformDist(generator);

        /* Check visited */
        iterVisited = visited.find(currPos);

        if(iterVisited == visited.end()){
            this->clusters.push_back(this->items[currPos]);
            visited.insert(currPos);
        }
        /* CLuster exists: try again */
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
    uniform_int_distribution<int> uniformDist(0, this->n);

    int clusterPos = 0, itemPos = 0, newClusterPos = 0;
    double currDist = 0, minDist = 0, tmpDouble = 0, randomDouble = 0;

    /* Keep distances between items and minimum cluster */
    vector<double> minCalculatedDistances;

    /* Pick initial cluster */
    itemPos = uniformDist(generator);
    this->clusters.push_back(this->items[itemPos]);

	/* Fix visited */
    visited.insert(itemPos);

    /* Find initial distances: dist(item, initial cluster) */
    for(itemPos = 0; itemPos < this->n; itemPos++){

        /* Find dist */
        currDist = this->distFunc(this->items[itemPos], this->clusters[0], status);
        if(status != SUCCESS)
            return;

        /* Add distances as min */
        minCalculatedDistances.push_back(currDist);
    } // End for items

    /* Pick remaining clusters */
    for(clusterPos = 1; clusterPos < this->numClusters; clusterPos++){

        /* Vector of partial sums - Reset                      */
        /* Each cell contains current partial sum and item pos */
        vector<vector<double> > partialSums;

        for(int i = 0; i < this->n + 1; i++)
            partialSums.push_back(vector<double>());

        /* Add zero */
        partialSums[0].push_back(0); // Dist
        partialSums[0].push_back(0); // Pos

        /* For every item find min cluster, calculate dist(item,minCluster)^2 and fix current partial sum*/
        for(itemPos = 0; itemPos < this->n; itemPos++){

			/* Discard clusters */
    	    iterVisited = visited.find(itemPos);

            /* Item is a cluster */
    	    if(iterVisited != visited.end()){
				partialSums[itemPos + 1].push_back(partialSums[itemPos][0]); // Add previous partial sum
				partialSums[itemPos + 1].push_back(partialSums[itemPos][1]); // Add previous pos
			}
            else{
                /* Set as initial min distance as the previous one */
                minDist = minCalculatedDistances[itemPos];

                /* Check if previous cluster is closer to the current item */
                if(clusterPos != 1){
                    /* Find distance form previous cluster */
                    currDist = this->distFunc(this->items[itemPos], this->clusters[clusterPos - 1], status);
                    if(status != SUCCESS)
                        return;

                    /* Set min distance */
                    if(currDist < minDist){
                        minDist = currDist;

                        /* Set new min distance */
                        minCalculatedDistances[itemPos] = minDist;
                    }
                } // End if - Find new min cluster

                //////////////////////
                /* Set partial sums */
                //////////////////////

                /* Find: D(i)^2 */
                tmpDouble = minDist * minDist;

                /* Find current partial sum - add previous partial sum with current */
                tmpDouble += partialSums[itemPos][0];

                /* Fix partial sums */
                partialSums[itemPos + 1].push_back(tmpDouble); // Add partial sum
                partialSums[itemPos + 1].push_back(partialSums[itemPos][1] + 1); // Add new pos
            } // End if - item is not a cluster
        } // End for items

		/* Pick a random value between [0-p(n-t)] */
    	uniform_real_distribution<double> uniformDist1(0, partialSums[this->n][0]);

	    /* Find random double */
        randomDouble = uniformDist1(generator);

        /* Find position on new cluster */
        newClusterPos = myUpperBound(partialSums, randomDouble, status);
        if(status != SUCCESS)
            return;

        /* Add new cluster and fix visited */
        this->clusters.push_back(this->items[newClusterPos]);
        iterVisited = visited.find(newClusterPos);
    } // End for remaining clusters 
}

// Petropoulakis Panagiotis