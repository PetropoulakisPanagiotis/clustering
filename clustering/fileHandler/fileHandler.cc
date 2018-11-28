#include <iostream>
#include <string>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <list>
#include <set>
#include <algorithm>
#include "../item/item.h"
#include "../utils/utils.h"
#include "fileHandler.h"

using namespace std;

/* Read given file and extract points */
/* WithId == 0, points have not id's  */
/* WithId == 1, points have id's      */
void readDataSet(string fileName, int withId, char delim, list<Item>& points, errorCode& status){
    ifstream file; 
    string line, word; // Line is splitted in words
    int i, wordsSize, specialChar;
    double currComponent;
    
    /* Structures */
    vector<string> metrices {"euclidean" , "cosine"}; // Available metrices
    set<string> ids; // Keep all ids - Check if all ids are unique
    set<string>::iterator iterSet; // Iterate through ids
    string::iterator iterStr; // Iteratre through word

    /* Note: words in file == points/id/metrices */
    status = SUCCESS;

    /* Check parameters */
    if(fileName.length() == 0 || (withId != 1 && withId != 0)){
        status = INVALID_PARAMETERS;
        return;
    }

    /* Clear points, types */
    points.clear();

    file.open(fileName);
    
    /* Check if file opened properly */
    if(!file){
        status = INVALID_DATA_SET;
        return;
    }
    
    /* Read lines in file */
    while(getline(file, line)){
        /* Discard empty lines */
        if(line.length() == 0)
            continue;

        /* For current points in line */
        vector<double> components;
        string id; 

        /* Split line */
        std::istringstream wordStream(line);
        vector<string> words; 

        /* Get points */
        while(getline(wordStream, word, delim))
            words.push_back(word);
       
        /* Receive id's - First word in line */
        if(withId == 1){
            iterSet = ids.find(words[0]); 

            /* Id exists - Invalid file */
            if(iterSet != ids.end()){
                status = INVALID_DATA_SET;
                return;
            }
            /* Add id in set */
            ids.insert(words[0]);

            /* Keep currend id */
            id = words[0];
        } // End if - id       

        string currWord; // Change form of word
        wordsSize = words.size();

        /* Read components */
        for(i = withId; i < wordsSize; i++){

            /* Handle word */
            currWord = words[i];

            specialChar = 0;

            /* Discard special characters */
            iterStr = currWord.begin();
            while(iterStr != currWord.end()){
                if((*iterStr < '0' || *iterStr > 'z') && *iterStr != '.' && *iterStr != '-'){
                    specialChar = 1;
                    break;
                }
                iterStr++;
            }

            /* Special characters found */
            if(specialChar == 1)
                continue;

            /* Handle negative numbers */
            if(currWord[0] == '-')
                currWord.erase(0,1);

            /* Handle floating points */
            currWord.erase(remove(currWord.begin(), currWord.end(), '.'), currWord.end());

            /* Check if component is digit */
            iterStr = currWord.begin();
            while(iterStr != currWord.end() && isdigit(*iterStr))
                iterStr++;

            /* Not digit */
            if(currWord.size() == 0 || iterStr != currWord.end()){
                status = INVALID_DATA_SET;
                return;
            }

            /* Convert string to double */
            try{
                currComponent = stod(words[i]);
            }
            catch(...){
                status = INVALID_POINTS;
                return;
            }

            /* Valid component - Save it */
            components.push_back(currComponent);
        } // End for
       
        /* Number of points is too big */
        if(points.size() == MAX_POINTS){
            status = INVALID_POINTS;
            return;
        }

        /* Create new item */
        if(withId == 1){
            points.push_back(Item(id,components,status));
            if(status != SUCCESS){
                return;
            }
        }
        else{
            points.push_back(Item(components,status));
            if(status != SUCCESS){
                return;
            }
        }
        
    } // End while - Read line

    /* Small number of points */
    if(points.size() < MIN_POINTS){
        status = INVALID_POINTS;
        return;
    }

    file.close();
}

/* Read config file         */
/* It contains 1 or 3 lines */
void readConf(string fileName, int& numClusters, int& k, int& l, errorCode& status){
    ifstream file;
    string line, word; // Line is splitted in words
    int step = 0;

    /* Note: words in file == points/id/metrices */
    status = SUCCESS;

    /* Check parameters */
    if(fileName.length() == 0){
        status = INVALID_PARAMETERS;
        return;
    }

    file.open(fileName);

    /* Check if file opened properly */
    if(!file){
        status = INVALID_CONF;
        return;
    }

    /* Default values */
    k = 4;
    l = 5;

    /* Read lines in file */
    while(getline(file, line)){

        if(step == 3)
            break;

        /* Discard empty lines */
        if(line.length() == 0)
            continue;

        /* Split line */
        std::istringstream wordStream(line);
        vector<string> words;

        /* Get words */
        while(getline(wordStream, word, ' '))
            words.push_back(word);

        if(words.size() != 2){
            status = INVALID_CONF;
            file.close();
            return;
        }

        if(step == 0){
            if(words[0] != "number_of_clusters:"){
                status = INVALID_CONF;
                file.close();
                return;
            }
            try{
                numClusters = stoi(words[1]) ; 
            }
            catch(...){
                status = INVALID_CONF;
                file.close();
                return;
            }
        }
        if(step == 1){
            if(words[0] != "number_of_hash_functions:"){
                status = INVALID_CONF;
                file.close();
                return;
            }
            try{
                k = stoi(words[1]) ; 
            }
            catch(...){
                status = INVALID_CONF;
                file.close();
                return;
            }
        }
        if(step == 2){
            if(words[0] != "number_of_hash_tables:"){
                status = INVALID_CONF;
                file.close();
                return;
            }
            try{
                l = stoi(words[1]) ; 
            }
            catch(...){
                status = INVALID_CONF;
                file.close();
                return;
            }
        }
        step++;
    } // End while

    file.close();
}
// Petropoulakis Panagiotis
