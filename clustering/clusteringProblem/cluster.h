#pragma once
#include <vector>
#include <list>
#include "../item/item.h"
#include "../utils/utils.h"
#include "../neighborsProblem/model/model.h"

/* Class cluster                */
/* Provided algorithms:         */
/* Init: random/k-means++       */
/* Assign: Lloyd's/Range rearch */
/* Update: k-means/pam improved */
class cluster{
    private:
        std::vector<Item> items;
        std::vector<int> itemsClusters; // Map item with cluster
        std::vector<Item> clusters; // Items that represents every cluster
        std::vector<std::list<int> > clustersItems; // Map every cluster with items
        model* rangeModel; // For range search
        double (*distFunc)(Item&, Item&, errorCode&); // Distance function
        double (*objFunc)(double, double); // Objective function
        int n;
        int dim;
        int numClusters; // Clusters to be created
        int maxIter;
        double tol; // Tolerance
        float coefficientRadius;
        double currStateVal; // Evaluate current state of cluster. Curr state represents objective value or something else.
        int fitted; // 1: items fitted, 2: clusters have been determined
        std::string initAlgo; // Init algorithm to be performed
        std::string assignAlgo;
        std::string updateAlgo;
        std::string metrice; // Euclidean/cosine

        /* Init functions */
        void randomInit(void);
        void kmeansPlusInit(errorCode& status);

        /* Assigment functions */
        int lloydAssign(errorCode& status);
        int rangeAssign(double& radius, errorCode& status);

        /* Update functions */
        void kmeans(errorCode& status);
        void pamLloyd(errorCode& status);

        /* Helpers functions */
        void fixCluster(errorCode& status, std::list<Item>& items, int numClusters, std::string initAlgo, std::string assignAlgo, std::string updateAlgo, std::string metrice, int maxIter, double tol); 
        int myUpperBound(std::vector<std::vector<double> >& x, double val, errorCode& status);
        double findItemAvgDist(int itemPos, int itemClusterPos, std::vector<std::vector<double> >& calculatedDistances, errorCode& status);
        void initRadius(double& radius, errorCode& status);

    public:
        cluster(errorCode& status, std::list<Item>& items, int numClusters=5, std::string initAlgo="random", std::string assignAlgo="lloyd", std::string updateAlgo="k-means", std::string metrice="euclidean", int maxIter=500, double tol=0.001);
        cluster(errorCode& status, std::list<Item>& items, int k, int l, int numClusters=5, std::string initAlgo="random", std::string assignAlgo="lloyd", std::string updateAlgo="k-means", std::string metrice="euclidean", int w=6, float coefficient=0.5, float coefficientRadius=0.001, int maxIter=500, double tol=0.001);
        cluster(errorCode& status, std::list<Item>& items, int k, int numClusters=5, std::string initAlgo="random", std::string assignAlgo="lloyd", std::string updateAlgo="k-means", std::string metrice="euclidean", int m=3000, int probes=15, int w=6, float coefficientRadius=5, int maxIter=500, double tol=0.001);

        ~cluster();

        /* Compute clustering */
        void fit(std::vector<Item>& clusters, std::vector<int>& clustersSize, errorCode& status);

        /* Get silhouette */
        void getSilhouette(std::vector<double>& silhouetteList, errorCode& status);

        /* Map items with it's cluster */
        void predict(std::list<int>& itemsPos, errorCode& status);

        /* Get id */
        std::string getId(errorCode& status);
};

/////////////////////////
/* Objective functions */
/////////////////////////
double getNextObjective1(double prevVal, double dist);
double getNextObjective2(double prevVal, double dist);
// Petropoulakis Panagiotis
