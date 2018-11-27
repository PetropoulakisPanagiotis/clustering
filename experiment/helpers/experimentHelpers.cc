#include <iostream>
#include <string.h>
#include <list>
#include <vector>
#include <chrono>
#include <fstream>
#include "experimentHelpers.h"
#include "../../clustering/utils/utils.h"
#include "../../clustering/item/item.h"
#include "../../clustering/clusteringProblem/cluster.h"

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
int runModel(list<Item>& items, int complete, ofstream& outputStream, string initAlgo, string assignAlgo, string updateAlgo, string& metrice, int numClucsters){
    vector<double> silhouetteArray;
    vector<Item> clusters; // Keep clusters
    vector<int> clustersSize;
    cluster* myCluster; // Cluster method
    int i, j;
    int silhouetteArraySize;
    chrono::steady_clock::time_point beginTimer, endTimer; // Measure time
    errorCode status;

    /* Check parameters */
    if(complete != 0 && complete != 1)
        return -1;

    cout << "Cluster:$ Creating model[" << initAlgo << "/" << assignAlgo << "/" << updateAlgo << "/" << metrice << "]\n";

    //////////////////
    /* Create model */
    //////////////////

    beginTimer = chrono::steady_clock::now();
    myCluster = new cluster(status, items, numClucsters, initAlgo, assignAlgo, updateAlgo, metrice);
    endTimer = chrono::steady_clock::now();

    if(myCluster == NULL){
        status = ALLOCATION_FAILED;
        printError(status);
        return -1;
    }

    if(status != SUCCESS){
        printError(status);
        delete myCluster;
        return -1;
    }

    /* Print elapsed time - Fix output file*/
    outputStream << "Algorithm: " << myCluster->getId(status) << "\n";
    if(status != SUCCESS){
        printError(status);
        delete myCluster;
        return -1;
    }

    outputStream << "Metric: " << metrice << "\n";
    cout << "Cluster:$ Model created[in: " << chrono::duration_cast<chrono::microseconds>(endTimer - beginTimer).count() / 1000000.0 << " sec]\n";

    cout << "Cluster:$ Fitting clusters\n";

    ///////////////////
    /* Find clusters */
    ///////////////////
    beginTimer = chrono::steady_clock::now();
    myCluster->fit(clusters, clustersSize, status);
    endTimer = chrono::steady_clock::now();

    if(status != SUCCESS){
        printError(status);
        delete myCluster;
        return -1;
    }

    /* Print elapsed time */
    cout << "Cluster:$ Clusters have been determined[in: " << chrono::duration_cast<chrono::microseconds>(endTimer - beginTimer).count() / 1000000.0 << " sec]\n";

    /* Print statistics of clusters in file */
    for(i = 0; i < numClucsters; i++){
        if(updateAlgo == "pam-lloyd")
            outputStream << "CLUSTER-" << i + 1 << " {size: " << clustersSize[i] << "," << " centroid: " << clusters[i].getId() << "\n";

        else{
            outputStream << "CLUSTER-" << i + 1 << " {size: " << clustersSize[i] << "," << " centroid: ";
            outputStream << "[";

            /* Print components of cluster */
            for(j = 0; j < clusters[i].getDim(); j++){
                outputStream << clusters[i].getComponent(j, status);
                if(status != SUCCESS){
                    printError(status);
                    delete myCluster;
                    return -1;
                }

                if(j != clusters[i].getDim() - 1)
                    outputStream << ",";
                else
                    outputStream << "]}\n";
            } // End for components
        }
    } // End for clusters

    cout << "Cluster:$ Calculating silhouette\n";

    ///////////////////////////////
    /* Find silhouette of cluster*/
    ///////////////////////////////
    beginTimer = chrono::steady_clock::now();
    myCluster->getSilhouette(silhouetteArray, status);
    endTimer = chrono::steady_clock::now();

    if(status != SUCCESS){
        printError(status);
        return -1;
    }

    cout << "Cluster:$ Silhouette: [";
    outputStream << "Silhouette: [";

    silhouetteArraySize = silhouetteArray.size();

    /* Print silhouette results */
    for(i = 0; i < silhouetteArraySize; i++){
        cout << silhouetteArray[i];
        outputStream << silhouetteArray[i];

        if(i != silhouetteArraySize - 1){
            cout << ",";
            outputStream << ", ";

        }
    } // End for

    cout << "] [in: " << chrono::duration_cast<chrono::microseconds>(endTimer - beginTimer).count() / 1000000.0 << " sec]\n";
    outputStream << "]\n";

    //////////////////////////
    /* Print whole clusters */
    //////////////////////////
    if(complete == 1){
        vector<vector<string> > clustersItemsStr; // Save each id per item in the coresponding culster
        list<int> itemsClusters; // Id of cluster for every item
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
            delete myCluster;
            return -1;
        }

        iterInt = itemsClusters.begin();

        /* Scan items and fix str vector */
        for(iterItems = items.begin(); iterItems != items.end(); iterItems++){
            clustersItemsStr[*iterInt].push_back(iterItems->getId());

            iterInt++;
        } // End for - items

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
    } // End if complete

    outputStream << "\n";

    /* Delete model */
    cout << "Cluster:$ Deleting cluster\n\n";
    delete myCluster;

    return 0;
};
// Petropoulakis Panagiotis
