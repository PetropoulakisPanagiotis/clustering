#include <iostream>
#include <vector>
#include <list>
#include <cmath>
#include <unordered_set>
#include <new>
#include "lsh.h"
#include "../../hashFunction/hashFunction.h"
#include "../../../item/item.h"
#include "../../../utils/utils.h"

using namespace std;

///////////////////////////////////////////
/* Implementation of lsh euclidean class */
///////////////////////////////////////////

/* Default constructor */
lshEuclidean::lshEuclidean():tableSize(0),coefficient(0.25),n(0),l(5),k(4),dim(0),w(800),fitted(0){
    int i;

    this->tables.reserve(this->l);

    /* Set size of hash tables */
    for(i = 0; i < this->l; i++)
        this->tables.push_back(vector<list<entry> >(this->l));
        
}

lshEuclidean::lshEuclidean(int k, int l, errorCode& status):tableSize(0),coefficient(0.25),n(0),l(l),k(k),dim(0),w(500),fitted(0){
    int i;

    /* Check parameters */
    if(l < MIN_L || l > MAX_L || k < MIN_K || k > MAX_K){
        status = INVALID_PARAMETERS;
        this->k = -1;
    }
    else{
        /* Set size of hash functions */
        this->hashFunctions.reserve(this->l);

        /* Set size of hash tables */
        for(i = 0; i < this->l; i++)
            this->tables.push_back(vector<list<entry> >(this->l));
    }
}

lshEuclidean::lshEuclidean(int l, int k, int w, float coefficient, errorCode& status):tableSize(0),coefficient(coefficient),n(0),l(l),k(k),w(w),fitted(0){

    /* Check parameters */
    if(l < MIN_L || l > MAX_L || k < MIN_K || k > MAX_K || w < MIN_W || w > MAX_W || coefficient < MIN_C || coefficient > MAX_C){
        status = INVALID_PARAMETERS;
        this->k = -1;
    }
    else{
      
        int i;
    
        /* Set size of hash functions */
        this->hashFunctions.reserve(this->l);

        /* Set size of hash tables */
        for(i = 0; i < this->l; i++)
            this->tables.push_back(vector<list<entry> >(this->l)); 
    }
}

lshEuclidean::~lshEuclidean(){
 
    /* Check method */
    if(this->k == -1){
        return;
    }   
    
    int i;

    /* Delete hash functions */
    if(this->fitted == 1)
        for(i = 0; i < this->l; i++)
            delete this->hashFunctions[i];
}

/* Fix hash table, members of lsh euclidean and add given points in the hash tables */
void lshEuclidean::fit(list<Item>& points, errorCode& status){
    int i, j, p;
    int pos; // Pos(line) in current hash table
    entry newEntry;

    /* Iteratiors */
    list<Item>::iterator iterPoints = points.begin(); // Iterate through points

    status = SUCCESS;

    /* Check method */
    if(this->k == -1){
        status = INVALID_METHOD;
        return;
    }

    /* Already fitted */
    if(this->fitted == 1){
        status = METHOD_ALREADY_USED;
        return;
    }

    /* Set members */
    this->n = points.size();
    if(this->n < MIN_POINTS || this->n > MAX_POINTS){
        status = INVALID_POINTS;
        return;
    }
    
    /* Set table size */
    this->tableSize = (int)(this->n * this->coefficient);
    if(this->tableSize == 0)
        this->tableSize = 1;

    /* Fix each table - Each table contains lists of entries */
    for(i = 0; i < this->l; i++){
        this->tables[i].reserve(this->tableSize);
        for(j = 0; j < this->tableSize; j++)
            this->tables[i].push_back(list<entry>());

    }

    /* Set dimension */
    this->dim = iterPoints->getDim();
    if(this->dim <= 0){
        status = INVALID_DIM;
        return;
    }
    
    ////////////////////////
    /* Set hash functions */
    ////////////////////////

    /* Function that will be inserted in hash functions table */
    hashFunctionEuclidean* newFunc = NULL;  

    for(i = 0; i < this->l; i++){
        newFunc = new hashFunctionEuclidean(this->dim, this->k, this->w, this->tableSize); 
        if(newFunc == NULL){
            status = ALLOCATION_FAILED;
            this->k = -1;
            break;
        }

        /* Truncate same hash functions */
        for(j = 0; j < i; j++){
            if(this->hashFunctions[j]->compare(*newFunc,status) == 0){
                delete newFunc;
                break;
            }

            if(status != SUCCESS){
                delete newFunc;
                this->k = -1;
                break;
            }
        } // End for
        
        if(status != SUCCESS)
            break;

        if(i == j){
            this->hashFunctions.push_back(newFunc); // Add hash function
        }
        else
            i -= 1;
    } // End for - Hash functions

    /* Delete remaining hash functions */
    if(status != SUCCESS){
        for(j = 0; j < i; j++)
            delete this->hashFunctions[j];
        return;
    }

    /* Set points */
    this->points.reserve(this->n);

    for(iterPoints = points.begin(); iterPoints != points.end(); iterPoints++)
        this->points.push_back(*iterPoints);

    /////////////////////
    /* Set hash tables */
    /////////////////////
    
    /* Scan each table */
    for(i = 0; i < this->l; i++){
    
        /* Scan points */
        for(p = 0; p < this->n; p++){
            /* Check consistency of dim */
            if(i == 0){
                if(this->dim != this->points[p].getDim()){
                    status = INVALID_DIM;
                    break;
               }
            }
            /* Find position in hash table */
            pos = this->hashFunctions[i]->hash(this->points[p], status);
            if(pos < 0 || pos >= tableSize){
                status = INVALID_HASH_FUNCTION;
                break;
            }

            if(status != SUCCESS){
                this->k = -1;
                break;
            }
            
            /* Set value g */
            vector<int> newValueG;

            for(j = 0; j < this->k; j++){
                newValueG.push_back(this->hashFunctions[i]->hashSubFunction(this->points[p], j, status));
                if(status != SUCCESS){
                    this->k = -1;
                    break;
                }
            } // End for

            /* Set new entry */
            newEntry.point = &(this->points[p]);
            newEntry.valueG.assign(newValueG.begin(), newValueG.end());
            newEntry.pos = p;

            /* Add point */
            this->tables[i][pos].push_back(newEntry);
        } // End for - Points

        if(status != SUCCESS)
            break;
    } // End for - Hash tables
  
    /* Error occured - Clear structures */
    if(status != SUCCESS){
       
        /* Clear points */
        for(i = 0; i < this->l; i++)
            for(j = 0; j < this->tableSize; j++)
                this->tables[i].clear();

        this->points.clear();

        /* Clear hash functions */
        for(i = 0; i < this->l; i++)
            delete this->hashFunctions[i];      
    }
    else
        /* Method fitted */
        this->fitted = 1;
}

/* Find the radius neighbors of a given point */
void lshEuclidean::radiusNeighbors(Item& query, int radius, list<Item>& neighbors, list<double>* neighborsDistances, errorCode& status){
    int i, pos, j;
    double currDist; // Distance of a point in list
    list<entry>::iterator iter;
    unordered_set<string> visited; // Visited points
    string currId;

    status = SUCCESS;

    /* Check parameters */
    if(radius < MIN_RADIUS || radius > MAX_RADIUS){
        status = INVALID_RADIUS;
        return; 
    }

    /* Check model */
    if(this->fitted == 0){
        status = METHOD_UNFITTED;
        return;
    }

    if(this->k == -1){
        status = INVALID_METHOD;
        return;
    }

    /* Clear given lists */
    neighbors.clear();
    if(neighborsDistances != NULL)
        neighborsDistances->clear();

    /* Scan all tables */
    for(i = 0; i < this->l; i++){
    
        /* Find position in table */
        pos = this->hashFunctions[i]->hash(query, status);
        if(status != SUCCESS)
            return;

        /* Empty list */
        if(this->tables[i][pos].size() == 0)
            continue;

        /* Find value g for query */
        vector<int> valueG;
        for(j = 0; j < this->k; j++){
            valueG.push_back(this->hashFunctions[i]->hashSubFunction(query, j, status));
            if(status != SUCCESS){        
                this->k = -1;
                break;
            }
        } // End for

        /* Scan list of specific bucket */
        for(iter = this->tables[i][pos].begin(); iter != this->tables[i][pos].end(); iter++){  
            
            currId = iter->point->getId();

            /* Compare values g of query and current point */
            if(!equal(valueG.begin(), valueG.end(), iter->valueG.begin()))
                continue;

            /* Find current distance */
            currDist = iter->point->euclideanDist(query, status);
            if(status != SUCCESS)
                return;
           
            /* Keep neighbor */
            if(currDist < radius){
                /* Not found - add it */
                if(visited.find(currId) == visited.end()){
                    visited.insert(currId);
                }
                /* Vidited - Discard it */
                else
                    continue;
                
                neighbors.push_back(*(iter->point));
                if(neighborsDistances != NULL)
                    neighborsDistances->push_back(currDist);
            }
        } // End for - Scan list
    } // End for - Tables
}

/* Find the radius neighbors of a given point */
void lshEuclidean::radiusNeighbors(Item& query, int radius, list<int>& neighborsIndexes, list<double>* neighborsDistances, errorCode& status){
    int i, pos, j;
    double currDist; // Distance of a point in list
    list<entry>::iterator iter;
    unordered_set<string> visited; // Visited points
    string currId;

    status = SUCCESS;

    /* Check parameters */
    if(radius < MIN_RADIUS || radius > MAX_RADIUS){
        status = INVALID_RADIUS;
        return; 
    }

    /* Check model */
    if(this->fitted == 0){
        status = METHOD_UNFITTED;
        return;
    }

    if(this->k == -1){
        status = INVALID_METHOD;
        return;
    }

    /* Clear given lists */
    neighborsIndexes.clear();
    if(neighborsDistances != NULL)
        neighborsDistances->clear();

    /* Scan all tables */
    for(i = 0; i < this->l; i++){
    
        /* Find position in table */
        pos = this->hashFunctions[i]->hash(query, status);
        if(status != SUCCESS)
            return;

        /* Empty list */
        if(this->tables[i][pos].size() == 0)
            continue;

        /* Find value g for query */
        vector<int> valueG;
        for(j = 0; j < this->k; j++){
            valueG.push_back(this->hashFunctions[i]->hashSubFunction(query, j, status));
            if(status != SUCCESS){        
                this->k = -1;
                break;
            }
        } // End for

        /* Scan list of specific bucket */
        for(iter = this->tables[i][pos].begin(); iter != this->tables[i][pos].end(); iter++){  
            
            currId = iter->point->getId();

            /* Compare values g of query and current point */
            if(!equal(valueG.begin(), valueG.end(), iter->valueG.begin()))
                continue;

            /* Find current distance */
            currDist = iter->point->euclideanDist(query, status);
            if(status != SUCCESS)
                return;
           
            /* Keep neighbor */
            if(currDist < radius){
                /* Not found - add it */
                if(visited.find(currId) == visited.end()){
                    visited.insert(currId);
                }
                /* Vidited - Discard it */
                else
                    continue;
                
                neighborsIndexes.push_back(iter->pos);
                if(neighborsDistances != NULL)
                    neighborsDistances->push_back(currDist);
            }
        } // End for - Scan list
    } // End for - Tables
}


/* Find the nearest neighbor of a given point */
void lshEuclidean::nNeighbor(Item& query, Item& nNeighbor, double* neighborDistance, errorCode& status){
    int i, pos, j, found = 0, flag = 0;
    double minDist = -1; // Current minimum distance 
    double currDist; // Distance of a point in list
    list<entry>::iterator iter;
    list<entry>::iterator iterNearestNeighbor;
    
    status = SUCCESS;

    /* Check model */
    if(this->fitted == 0){
        status = METHOD_UNFITTED;
        return;
    }

    if(this->k == -1){
        status = INVALID_METHOD;
        return;
    }

    /* Scan all tables */
    for(i = 0; i < this->l; i++){
    
        /* Find position in table */
        pos = this->hashFunctions[i]->hash(query, status);
        if(status != SUCCESS)
            return;

        /* Empty list */
        if(this->tables[i][pos].size() == 0)
            continue;

        /* Find value g for query */
        vector<int> valueG;
        for(j = 0; j < k; j++){
            valueG.push_back(this->hashFunctions[i]->hashSubFunction(query, j, status));
            if(status != SUCCESS){        
                this->k = -1;
                break;
            }
        } // End for
        
        /* Scan list of specific bucket */
        for(iter = this->tables[i][pos].begin(); iter != this->tables[i][pos].end(); iter++){  

                        
            /* Compare values g of query and current point */
            if(!equal(valueG.begin(), valueG.end(), iter->valueG.begin()))
                continue;            

            /* Find current distance */
            currDist = iter->point->euclideanDist(query, status);
            if(status != SUCCESS)
                return;
            
            /* First neighbor */
            if(flag == 0){
                minDist = currDist;
                iterNearestNeighbor = iter;
                
                found = 1;
                flag = 1;
            }

            /* Change min distance */
            else if(minDist > currDist){
                minDist = currDist;
                iterNearestNeighbor = iter;
            }
        } // End for - Scan list
    } // End for - Tables

    /* Nearest neighbor found */
    if(found == 1){
        nNeighbor = *(iterNearestNeighbor->point);
        if(neighborDistance != NULL)
            *neighborDistance = minDist;
    }
    else{
        nNeighbor.setId("Nearest neighbor not found");
        if(neighborDistance != NULL)
            *neighborDistance = minDist;
    }
}

///////////////
/* Accessors */
///////////////

int lshEuclidean::getNumberOfPoints(errorCode& status){
    status = SUCCESS;

    if(this->fitted == 0){
        status = METHOD_UNFITTED;
        return -1;
    }
    else if(this->k == -1){
        status = INVALID_METHOD;
        return -1;
    }
    else
        return this->n;
}

int lshEuclidean::getDim(errorCode& status){
    status = SUCCESS;

    if(this->fitted == 0){
        status = METHOD_UNFITTED;
        return -1;
    }
    else if(this->k == -1){
        status = INVALID_METHOD;
        return -1;
    }
    else
        return this->dim;
}

unsigned lshEuclidean::size(void){
    unsigned result = 0;

    if(fitted == 0){
        return -1;
    }
    
    if(this->k == -1){
        return -1;
    }

    
    result += sizeof(this->tableSize);
    result += sizeof(this->coefficient);
    result += sizeof(this->n);
    result += sizeof(this->l);
    result += sizeof(this->k);
    result += sizeof(this->dim);
    result += sizeof(this->w);
    result += sizeof(this->fitted);
    
    int i, j;

    for(i = 0; i < this->l; i++)
        result += this->hashFunctions[i]->size();

    result += this->hashFunctions.capacity() * sizeof(hashFunction*);

    result += sizeof(hashFunctions);

    list<entry>::iterator iter;

    for(i = 0; i < this->l; i++){
        for(j = 0; j < this->tableSize; j++){
            for(iter = this->tables[i][j].begin(); iter!= this->tables[i][j].end(); iter++){
                result += sizeof(Item*);

                result += iter->valueG.capacity() * sizeof(int);
                result += sizeof(iter->valueG);
                result += sizeof(int);
            } // End for - iter
        } // End for - table size
    } // End for - l

    for(i = 0; i < this->l; i++){
        for(j = 0; j < this->tableSize; j++){
            result += sizeof(list<entry>);
        } // End for - table size
    } // End for - l

    result += this->tables.capacity() * sizeof(vector<list<entry> >);

    result += sizeof(this->tables);

    for(i = 0; i < this->n; i++)
        result += this->points[i].size();

    result += sizeof(Item) * this->points.capacity();

    result += sizeof(this->points);

    return result;
}

/* Print statistics */
void lshEuclidean::print(void){

    if(this->k == -1)
        cout << "Invalid method\n";
    else{

        cout << "Lsh euclidean statistics\n";
        cout << "Number of hash tables(l): " << this->l << "\n";
        cout << "Size per table: " << this->tableSize << "\n";
        cout << "Coefficient factor: " << this->coefficient << "\n";
        cout << "Window size(w): " << this->w << "\n";
        cout << "Number of sub hash functions(k): " << this->k << "\n\n";
    }
}

void lshEuclidean::printHashFunctions(void){

    if(this->k == -1)
        cout << "Invalid method\n";
    else if(this->fitted == 0)
        cout << "Method not fitted\n";
    else{

        int i;
        
        cout << "Hash functions of lsh euclidean\n";
        for(i = 0; i < this->l; i++)
            this->hashFunctions[i]->print();
    }
}

// Petropoulakis Panagiotis

