#include <iostream>
#include <list>
#include <chrono>
#include <fstream>
#include "../clustering/item/item.h"
#include "../clustering/fileHandler/fileHandler.h"
#include "../clustering/utils/utils.h"
#include "../clustering/clusteringProblem/cluster.h"
#include "helper.h"

using namespace std;

int main(int argc, char **argv){
    int numClucsters;
    char delim = ',';
    list<Item> items; // Items in given data set
    int argumentsProvided;
    string inputFile, confFile, outputFile, metrice;
    string initAlgo, assignAlgo, updateAlgo;
    fstream outputStream;
    errorCode status;

    /* Read and check arguments */
    argumentsProvided = readArguments(argc, argv, inputFile, confFile, outputFile, metrice);
    if(argumentsProvided != 0){
        cout << "Please give valid arguments. Try again later\n";
        return 0;
    }

    chrono::steady_clock::time_point beginTimer, endTimer; // Measure time

    cout << "Welcome to clustering problem\n";
    cout << "-----------------------------\n\n";

    cout << "Cluster:$ Reading data set\n";

    /* Read data set */
    beginTimer = chrono::steady_clock::now();
    readDataSet(inputFile, 1, delim, items, status);
    if(status != SUCCESS){
        printError(status);
        return 0;
    }
    endTimer = chrono::steady_clock::now();

    /* Print time */
    cout << "Cluster:$ Items have been determined[in: " << chrono::duration_cast<chrono::microseconds>(endTimer - beginTimer).count() / 1000000.0 << " sec]\n\n";

    /* Open output file */
    outputStream.open(outputFile, ios::trunc);
    if(!outputStream){
        cout << "Can't open given output file\n";
        return 0;
    }

    cluster* myCluster;

    /*  Create cluster method */
    initAlgo = "random";
    assignAlgo = "lloyd";
    updateAlgo = "k-means";
    metrice = "euclidean";
    numClucsters = 4;


    cout << "Cluster:$ Creating model[" << initAlgo << "/" << assignAlgo << "/" << updateAlgo << "/" << metrice << "]\n\n";

    beginTimer = chrono::steady_clock::now();
    myCluster = new cluster(status, items, numClucsters, initAlgo, assignAlgo, updateAlgo, metrice);
    endTimer = chrono::steady_clock::now();

    /* Print time */
    cout << "Cluster:$ Model created[in: " << chrono::duration_cast<chrono::microseconds>(endTimer - beginTimer).count() / 1000000.0 << " sec]\n\n";

    /* Perform clustering */
    runModel(myCluster, outputStream, initAlgo, assignAlgo, updateAlgo, metrice, numClucsters);

    cout << "Cluster:$ Deleting cluster\n\n";
    delete myCluster;




    cout << "--Expirement is over. Have a good day!--\n";

    return 0;
}
// Petropoulakis Panagiotis
