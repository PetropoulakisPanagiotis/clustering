#include <iostream>
#include <string.h>
#include <list>
#include <vector>
#include <chrono>
#include <fstream>
#include "helper.h"
#include "../clustering/utils/utils.h"
#include "../clustering/item/item.h"
#include "../clustering/clusteringProblem/cluster.h"

using namespace std;

/* Read arguments from the user */
/* Arguments provided: 0        */
/* Invalid arguments: -1        */
int readArguments(int argc, char **argv,string& inputFile, string& confFile, string& outputFile, int& complete, string& metrice){

    /* Invalid arguments */
    if(argc != 9 && argc != 10)
        return -1;

    if(strcmp(argv[1], "-i") || strcmp(argv[3], "-c") || strcmp(argv[5], "-o") || strcmp(argv[7], "-d"))
        return -1;

    if(argc == 10 && strcmp(argv[9], "-complete"))
        return -1;

    /* Copy arguments */
    inputFile = argv[2];
    confFile = argv[4];
    outputFile = argv[6];
    metrice = argv[8];

    if(argc == 10)
        complete = 1;
    else
        complete = 0;

    return 0;
}

/* Create and run clustering model based on parameters */
/* Print results in output file                        */
/* Success: 0                                          */
/* Failure: -1                                         */
int runModel(cluster* myCluster, list<Item>& items, int complete, ofstream& outputStream, string& initAlgo, string& assignAlgo, string& updateAlgo, string& metrice, int numClucsters){
    vector<double> silhouetteArray;
    vector<Item> clusters;
    vector<int> clustersSize;
    int i, j, itemsSize = (int)items.size();
    chrono::steady_clock::time_point beginTimer, endTimer; // Measure time
    errorCode status;


    /* Check parameters */
    if(myCluster == NULL || (complete != 0 && complete != 1))
        return -1;

    cout << "Cluster:$ Fitting clusters\n\n";

    /* Find clusters */
    beginTimer = chrono::steady_clock::now();
    myCluster->fit(clusters, clustersSize, status);
    if(status != SUCCESS){
        printError(status);
        return -1;
    }

    endTimer = chrono::steady_clock::now();

    /* Print time */
    cout << "Cluster:$ Clusters have been determined[in: " << chrono::duration_cast<chrono::microseconds>(endTimer - beginTimer).count() / 1000000.0 << " sec]\n\n";


    /* Print in file */
    for(i = 0; i < numClucsters; i++){
        if(updateAlgo == "pam-lloyd")
            outputStream << "CLUSTER-" << i + 1 << " {size: " << clustersSize[i] << "," << " centroid: " << clusters[i].getId() << "\n";
        else if(updateAlgo == "k-means"){
            outputStream << "CLUSTER-" << i + 1 << " {size: " << clustersSize[i] << "," << " centroid: ";

            outputStream << "[";
            /* Print components of cluster */
            for(j = 0; j < clusters[i].getDim(); j++){
                outputStream << clusters[i].getComponent(j, status);
                if(status != SUCCESS){
                    printError(status);
                    return -1;
                }

                if(j != clusters[i].getDim() - 1)
                    outputStream << ",";
                else
                    outputStream << "]}\n";
            } // End for components
        }
    } // End for clusters

    cout << "Cluster:$ Calculating silhouette\n\n";

    /* Find silhouette of cluster*/
    beginTimer = chrono::steady_clock::now();
    myCluster->getSilhouette(silhouetteArray, status);
    if(status != SUCCESS){
        printError(status);
        return -1;
    }

    endTimer = chrono::steady_clock::now();

    cout << "Cluster:$ Silhouette: [";
    outputStream << "Silhouette: [";

    /* Print silhouette results */
    for(i = 0; i < (int)silhouetteArray.size(); i++){

        cout << silhouetteArray[i];
        outputStream << silhouetteArray[i];

        if(i != (int)silhouetteArray.size() - 1){
            cout << ",";    
            outputStream << ", ";    

        }
    } // End for

    cout << "] [in: " << chrono::duration_cast<chrono::microseconds>(endTimer - beginTimer).count() / 1000000.0 << " sec]\n\n";
    outputStream << "]\n";

    /* Print whole clusters */
    if(complete == 1){
        /* Check for complete - Print for every cluster it's items */
        vector<vector<string> > clustersItemsStr; // Keep for every cluster every item's id
        list<int> itemsClusters;
        list<int>::iterator iterInt;
        list<Item>::iterator iterItems;
        int clusterSize;

        /* Initialize strs */
        for(i = 0; i < numClucsters; i++)
            clustersItemsStr.push_back(vector<string>());

        /* Get index of cluster for every item */
        myCluster->predict(itemsClusters, status);
        if(status != SUCCESS){
            printError(status);
            return -1;
        }

        iterInt = itemsClusters.begin();

        /* Scan items and fix complete Strs vector */
        for(iterItems = items.begin(); iterItems != items.end(); iterItems++){
            clustersItemsStr[*iterInt].push_back(iterItems->getId());

            iterInt++;
        } // End for scann items

        outputStream << "\n";

        /* Print whole clusters in file */
        for(i = 0; i < numClucsters; i++){
            clusterSize = (int)clustersItemsStr[i].size();

            if(clusterSize == 0)
                outputStream <<  "CLUSTER-" << i << " {}\n";
            else
                outputStream <<  "CLUSTER-" << i << " {";

            /* Write items in current cluster */
            for(j = 0; j < clusterSize; j++){

                outputStream << clustersItemsStr[i][j];

                if(j != clusterSize - 1)
                    outputStream << ", ";
                else
                    outputStream << "}\n";
            } // End for items in cluster
        } // End for clusters

        outputStream << "\n";
    } // End else complete


    return 0;
};
// Petropoulakis Panagiotis
