#include <iostream>
#include <string.h>
#include "helper.h"

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
// Petropoulakis Panagiotis
