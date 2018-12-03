#include <iostream>
#include <unistd.h>
#include <vector>
#include "../../utils/utils.h"
#include "../cluster.h"

using namespace std;

//////////////////////
/* Update functions */
//////////////////////

/* Update: kmeans                        */
/* New cluster: average item in cluster  */
void cluster::kmeans(errorCode& status){
    vector<int> flags; // For clearing components of clusters
    vector<int> sizeOfClusters;
    int itemPos, componentPos, clusterPos; // Indexes
    double newComponent;

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

    /* Initialize vectors */
    for(clusterPos = 0; clusterPos < this->numClusters; clusterPos++){
        flags.push_back(0);
        sizeOfClusters.push_back(0);
    } // End for

    /* Scan all items */
    for(itemPos = 0; itemPos < this->n; itemPos++){

        /* Find position of cluster of current item */
        clusterPos = this->itemsClusters[itemPos];

        /* Reset cluster only once */
        if(flags[clusterPos] == 0){
            this->clusters[clusterPos].clearComponents();
            flags[clusterPos] = 1;
        }

        /* Scan components of current item and add it in current cluster */
        for(componentPos = 0; componentPos < this->dim; componentPos++){

            /* Get current componet */
            newComponent = this->items[itemPos].getComponent(componentPos, status);
            if(status != SUCCESS)
                return;

            /* Add new component in current cluster */
            this->clusters[clusterPos].sumComponent(newComponent, componentPos, status);
            if(status != SUCCESS)
                return;
        } // End for - components

        /* Increase size of cluster */
        sizeOfClusters[clusterPos] += 1;
    } // End for - items

    /* Divide components of clusters with size */
    for(clusterPos = 0; clusterPos < this->numClusters; clusterPos++){

        /* Empty cluster */
        if(flags[clusterPos] == 0)
            continue;

        /* Divide each component */
        for(componentPos = 0; componentPos < this->dim; componentPos++){

            this->clusters[clusterPos].divComponent(sizeOfClusters[clusterPos], componentPos, status);
            if(status != SUCCESS)
                return;
        } // End for - components
    } // End for - clusters
}

/* Update: pam like lloyd's algorithm                          */
/* New cluster: minimizes the overall distance from every item */
void cluster::pamLloyd(errorCode& status){
    int itemPos, clusterPos = 0, itemSameClusterPos; // Indexes
    int newMinClusterPos = 0; // New cluster position
    int flag, flagSize;
    int clusterSize; // Size of current cluster(-1)
    double minDist, tmpDist, currDist;

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

    /*  Update each cluster with it's medoid */
    for(clusterPos = 0; clusterPos < this->numClusters; clusterPos++){

        /* Resets */
        flag = 0;
        flagSize = 0;
        clusterSize = 0;
        newMinClusterPos = -1;

        /* Scan all items and keep items in current cluster */
        for(itemPos = 0; itemPos < this->n; itemPos++){

            /* Item does not exists in current cluster - Discard it */
            if(this->itemsClusters[itemPos] != clusterPos)
                continue;

            tmpDist = 0;

            /* Scan other items in the same cluster and find dissimilarty distance for current item */
            for(itemSameClusterPos = 0; itemSameClusterPos < this->n; itemSameClusterPos++){

                /* Do not check current item with it's self */
                if(itemPos == itemSameClusterPos || (this->itemsClusters[itemSameClusterPos] != clusterPos))
                    continue;

                /* Fix cluster size */
                if(flagSize == 0)
                    clusterSize += 1;

                /* Find current distance */
                currDist = this->distFunc(this->items[itemPos], this->items[itemSameClusterPos], status);
                if(status != SUCCESS)
                    return;

                tmpDist += currDist;
            } // End for - iitems in same cluster except from x item

            if(flagSize == 0){
                flagSize = 1;
                /* Only one or zero items in cluster */
                if(clusterSize == 0)
                    break;
            }

            /* Fix average dist */
            tmpDist = tmpDist / (double)clusterSize;

            /* Fix minimun distance */
            if(flag == 0){
                minDist = tmpDist;
                newMinClusterPos = itemPos;
                flag = 1;
            }
            else if(minDist > tmpDist){
                minDist = tmpDist;
                newMinClusterPos = itemPos;
            }
        } // End for - items(x)

        /* Set new medoid */
        if(newMinClusterPos != -1)
            this->clusters[clusterPos] = this->items[newMinClusterPos];
    } // End for clusters
}
// Petropoulakis Panagiotis
