#include <iostream>
#include <list>
#include <chrono>
#include <fstream>
#include "../clustering/item/item.h"
#include "../clustering/fileHandler/fileHandler.h"
#include "../clustering/utils/utils.h"
#include "helpers/experimentHelpers.h"

using namespace std;

int main(int argc, char **argv){
    int numClucsters, k, l;
    char delim = ',';
    list<Item> items; // Items in given data set
    int argumentsProvided, returnVal;
    int complete; // Complete: print items for every cluster
    string inputFile, confFile, outputFile, metrice;
    ofstream outputStream;
    errorCode status; // Keep errors

    /* Read and check arguments */
    argumentsProvided = readArguments(argc, argv, inputFile, confFile, outputFile, complete, metrice);
    if(argumentsProvided != 0){
        cout << "Please give valid arguments. Try again later\n";
        return 0;
    }

    /* Measure time */
    chrono::steady_clock::time_point beginTimer, endTimer;

    cout << "Welcome to clustering problem\n";
    cout << "-----------------------------\n\n";

    cout << "Cluster:$ Reading data set\n\n";

    /* Read data set */
    beginTimer = chrono::steady_clock::now();
    readDataSet(inputFile, 1, delim, items, status);
    endTimer = chrono::steady_clock::now();
    if(status != SUCCESS){
        printError(status);
        return 0;
    }

    /* Print elapsed time for reading the data set */
    cout << "Cluster:$ Items have been determined[in: " << chrono::duration_cast<chrono::microseconds>(endTimer - beginTimer).count() / 1000000.0 << " sec]\n\n";

    /* Read config file */
    readConf(confFile, numClucsters, k, l, status);
    if(status != SUCCESS){
        printError(status);
        return 0;
    }

    /* Open output file */
    outputStream.open(outputFile, ios::trunc);
    if(!outputStream){
        cout << "Can't open given output file\n";
        return 0;
    }

    ////////////////////////
    /* Perform clustering */
    /* Run all models     */
    ////////////////////////

    numClucsters = 2;
    
    returnVal = runModel(items, complete, outputStream, "k-means++", "range-lsh", "k-means", metrice, numClucsters, k, l);
    if(returnVal == -1){
        outputStream.close();
        return 0;
    }


    
    /*
    returnVal = runModel(items, complete, outputStream, "random", "lloyd", "k-means", metrice, numClucsters);
    if(returnVal == -1){
        outputStream.close();
        return 0;
    }

    returnVal = runModel(items, complete, outputStream, "random", "lloyd", "pam-lloyd", metrice, numClucsters);
    if(returnVal == -1){
        outputStream.close();
        return 0;
    }

    returnVal = runModel(items, complete, outputStream, "k-means++", "lloyd", "k-means", metrice, numClucsters);
    if(returnVal == -1){
        outputStream.close();
        return 0;
    }

    returnVal = runModel(items, complete, outputStream, "k-means++", "lloyd", "pam-lloyd", metrice, numClucsters);
    if(returnVal == -1){
        outputStream.close();
        return 0;
    }
    returnVal = runModel(items, complete, outputStream, "random", "range-lsh", "k-means", metrice, numClucsters, k, l);
    if(returnVal == -1){
        outputStream.close();
        return 0;
    }

    returnVal = runModel(items, complete, outputStream, "random", "range-lsh", "pam-lloyd", metrice, numClucsters, k, l);
    if(returnVal == -1){
        outputStream.close();
        return 0;
    }

    returnVal = runModel(items, complete, outputStream, "k-means++", "range-lsh", "k-means", metrice, numClucsters, k, l);
    if(returnVal == -1){
        outputStream.close();
        return 0;
    }

    returnVal = runModel(items, complete, outputStream, "k-means++", "range-lsh", "pam-lloyd", metrice, numClucsters, k, l);
    if(returnVal == -1){
        outputStream.close();
        return 0;
    }

    returnVal = runModel(items, complete, outputStream, "random", "range-hypercube", "k-means", metrice, numClucsters, k);
    if(returnVal == -1){
        outputStream.close();
        return 0;
    }

    returnVal = runModel(items, complete, outputStream, "random", "range-hypercube", "pam-lloyd", metrice, numClucsters, k);
    if(returnVal == -1){
        outputStream.close();
        return 0;
    }

    returnVal = runModel(items, complete, outputStream, "k-means++", "range-hypercube", "k-means", metrice, numClucsters, k);
    if(returnVal == -1){
        outputStream.close();
        return 0;
    }

    returnVal = runModel(items, complete, outputStream, "k-means++", "range-hypercube", "pam-lloyd", metrice, numClucsters, k);
    if(returnVal == -1){
        outputStream.close();
        return 0;
    }
*/
    cout << "--Expirement is over. Have a good day!--\n";
    outputStream.close();

    return 0;
}
// Petropoulakis Panagiotis
