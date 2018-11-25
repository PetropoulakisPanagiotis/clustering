#include <iostream>
#include <string.h>
#include <list>
#include <chrono>
#include "helper.h"
#include "../clustering/utils/utils.h"
#include "../clustering/item/item.h"
#include "../clustering/clusteringProblem/cluster.h"

using namespace std;

/* Read arguments from the user */
/* Arguments provided: 0        */
/* Invalid arguments: -1        */
int readArguments(int argc, char **argv,string& inputFile, string& confFile, string& outputFile, string& metrice){

    /* Invalid arguments */
    if(argc != 9)
        return -1;

    if(strcmp(argv[1], "-i") || strcmp(argv[3], "-c") || strcmp(argv[5], "-o") || strcmp(argv[7], "-d"))
        return -1;

    /* Copy arguments */
    inputFile = argv[2];
    confFile = argv[4];
    outputFile = argv[6];
    metrice = argv[8];

    return 0;
}

/* Create and run clustering model based on parameters */
/* Print results in output file                        */
void runModel(list<Item>& items, string& outputFile, string& initAlgo, string& assignAlgo, string& updateAlgo, string& metrice, int numClucsters){
    cluster* myCluster; // Cluster method
    double silhouette = 0;
    chrono::steady_clock::time_point beginTimer, endTimer; // Measure time
    errorCode status;

    cout << "Cluster:$ Creating model[" << initAlgo << "/" << assignAlgo << "/" << updateAlgo << "/" << metrice << "]\n\n";

    beginTimer = chrono::steady_clock::now();
    myCluster = new cluster(status, items, numClucsters, initAlgo, assignAlgo, updateAlgo, metrice);
    endTimer = chrono::steady_clock::now();

    /* Print time */
    cout << "Cluster:$ Model created[in: " << chrono::duration_cast<chrono::microseconds>(endTimer - beginTimer).count() / 1000000.0 << " sec]\n\n";


    cout << "Cluster:$ Fitting clusters\n\n";

    /* Find clusters */
    beginTimer = chrono::steady_clock::now();
    myCluster->fit(status);
    if(status != SUCCESS){
        printError(status);
        delete myCluster;
        return;
    }

    endTimer = chrono::steady_clock::now();

    /* Print time */
    cout << "Cluster:$ Clusters have been determined[in: " << chrono::duration_cast<chrono::microseconds>(endTimer - beginTimer).count() / 1000000.0 << " sec]\n\n";

    cout << "Cluster:$ Calculating overall silhouette\n\n";

    /* Find silhouette of cluster*/
    beginTimer = chrono::steady_clock::now();
    silhouette = myCluster->getSilhouette(status);
    if(status != SUCCESS){
        printError(status);
        delete myCluster;
        return;
    }

    endTimer = chrono::steady_clock::now();

    cout << "Cluster:$ Overall silhouette is: " << silhouette << " [in: " << chrono::duration_cast<chrono::microseconds>(endTimer - beginTimer).count() / 1000000.0 << " sec]\n\n";


    cout << "Cluster:$ Deleting cluster\n\n";

    delete myCluster;
};
// Petropoulakis Panagiotis
