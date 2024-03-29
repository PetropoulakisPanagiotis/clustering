#pragma once
#include <vector>
#include <list>
#include "../model.h"
#include "../../../item/item.h"
#include "../../../utils/utils.h"
#include "../../hashFunction/hashFunction.h"

/* Neighbors problem using lsh euclidean */
class lshEuclidean: public model{
    private:
        /* Entries in hash tables */
        typedef struct entry{
            Item* point; // Use pointer - Save memory
            std::vector<int> valueG; // Value of g hash function(2 levels of hashing - compare query and point with same g)
            int pos; // Position of point
        }entry;

        std::vector<Item> points; // Keep points
        std::vector<std::vector<std::list<entry> > > tables; // Each table is a hash table(vector of lists)
        std::vector<hashFunction*> hashFunctions; // Each table has one hash function
        int tableSize;
        float coefficient; // Table size == n * coefficient, (coefficient <= 1)
        int n; // Number of items 
        int l; // Total tables, hash functions
        int k; // Number of sub hash functions
        int dim; // Dimension
        int w; // Window size
        int fitted; // Method is fitted with data
    
    public:

        lshEuclidean();
        lshEuclidean(int k, int l, errorCode& status);
        lshEuclidean(int l, int k, int w, float coefficient, errorCode& status);
        
        ~lshEuclidean();

        void fit(std::list<Item>& points, errorCode& status);

        void radiusNeighbors(Item& query, int radius, std::list<Item>& neighbors, std::list<double>* neighborsDistances, errorCode& status);
        void radiusNeighbors(Item& query, int radius, std::list<int>& neighborsIndexes, std::list<double>* neighborsDistances, errorCode& status);

        void nNeighbor(Item& query, Item& nNeighbor, double* neighborDistance, errorCode& status);
        
        int getNumberOfPoints(errorCode& status);
        int getDim(errorCode& status);
        unsigned size(void);
        
        void print(void);
        void printHashFunctions(void);
};

/* Neighbors problem using lsh cosine */
class lshCosine: public model{
    private:
        /* Entries in hash tables */
        typedef struct entry{
            Item* point; // Use pointer - Save memory
            int pos; // Position of point
        }entry;

        std::vector<Item> points; // Keep points
        std::vector<std::vector<std::list<entry> > > tables; // Each table is a hash table(vector of lists)
        std::vector<hashFunction*> hashFunctions; // Each table has one hash function       
        int tableSize;
        int n; // Number of items 
        int l; // Total tables 
        int k; // Number of sub hash functions
        int dim; // Dimension
        int fitted;
    
    public:

        lshCosine();
        lshCosine(int k, int l, errorCode& status);

        ~lshCosine();

        void fit(std::list<Item>& points, errorCode& status);

        void radiusNeighbors(Item& query, int radius, std::list<Item>& neighbors, std::list<double>* neighborsDistances, errorCode& status);
        void radiusNeighbors(Item& query, int radius, std::list<int>& neighborsIndexes, std::list<double>* neighborsDistances, errorCode& status);
        void nNeighbor(Item& query, Item& nNeighbor, double* neighborDistance, errorCode& status);
        
        int getNumberOfPoints(errorCode& status);
        int getDim(errorCode& status);
        unsigned size(void);

        void print(void);
        void printHashFunctions(void);
};

// Petropoulakis Panagiotis
