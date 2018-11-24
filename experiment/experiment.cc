#include <iostream>
#include <list>
#include "../clustering/clusteringProblem/cluster.h"
#include "../clustering/item/item.h"
#include "../clustering/fileHandler/fileHandler.h"
#include "../clustering/utils/utils.h"
#include "helper.h"

using namespace std;

int main(int argc, char **argv){
    cluster* myCluster; // Cluster method
    int numClucsters = 4;
    char delim = ',';
    list<Item> items; // Items in given data set
    int argumentsProvided;
    string inputFile, confFile, outputFile, metrice;
    double silhouette = 0;
    errorCode status;

    /* Read and check arguments */
    argumentsProvided = readArguments(argc, argv, inputFile, confFile, outputFile, metrice);
    if(argumentsProvided != 0){
        cout << "Please give valid arguments. Try again later\n";
        return 0;
    }

    /* Read data set */
    readDataSet(inputFile, 1, delim, items, status);
    if(status != SUCCESS){
        printError(status);
        return 0;
    }

    /*  Create cluster method */
    myCluster = new cluster(status, items, numClucsters, "k-means++", "lloyd", "k-means", "euclidean");

    /* Find clusters */
    myCluster->fit(status);
    if(status != SUCCESS){
        printError(status);
        return 0;
    }

    /* Find silhouette of cluster*/
    //silhouette = myCluster->getSilhouette(status);
    if(status != SUCCESS){
        printError(status);
        return 0;
    }

    cout << silhouette << "\n";

    delete myCluster;

    return 0;
}
// Petropoulakis Panagiotis
