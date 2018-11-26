# Petropoulakis Panagiotis
CC = g++
FLAGS = -g -Wall

exmperiment: experiment.o helper.o utils.o hashFunction.o item.o fileHandler.o lshEuclidean.o lshCosine.o cluster.o clusterAlgorithms.o clusterHelpers.o
	$(CC) -o experiment $(FLAGS) experiment.o helper.o utils.o hashFunction.o item.o fileHandler.o lshEuclidean.o lshCosine.o cluster.o clusterAlgorithms.o clusterHelpers.o -std=c++11

experiment.o: experiment.cc
	$(CC) -c  $(FLAGS) experiment.cc -std=c++11

helper.o: helper.cc
	$(CC) -c  $(FLAGS) helper.cc -std=c++11

utils.o: ../clustering/utils/utils.cc
	$(CC) -c $(FLAGS) ../clustering/utils/utils.cc -std=c++11

hashFunction.o: ../clustering/neighborsProblem/hashFunction/hashFunction.cc
	$(CC) -c $(FLAGS) ../clustering/neighborsProblem/hashFunction/hashFunction.cc -std=c++11

item.o: ../clustering/item/item.cc
	$(CC) -c $(FLAGS) ../clustering/item/item.cc -std=c++11

fileHandler.o: ../clustering/fileHandler/fileHandler.cc
	$(CC) -c $(FLAGS) ../clustering/fileHandler/fileHandler.cc -std=c++11

lshEuclidean.o: ../clustering/neighborsProblem/model/lsh/lshEuclidean.cc
	$(CC) -c $(FLAGS) ../clustering/neighborsProblem/model/lsh/lshEuclidean.cc -std=c++11

lshCosine.o: ../clustering/neighborsProblem/model/lsh/lshCosine.cc
	$(CC) -c $(FLAGS) ../clustering/neighborsProblem/model/lsh/lshCosine.cc -std=c++11

cluster.o: ../clustering/clusteringProblem/cluster.cc
	$(CC) -c $(FLAGS) ../clustering/clusteringProblem/cluster.cc -std=c++11

clusterAlgorithms.o: ../clustering/clusteringProblem/clusterAlgorithms.cc
	$(CC) -c $(FLAGS) ../clustering/clusteringProblem/clusterAlgorithms.cc -std=c++11

clusterHelpers.o: ../clustering/clusteringProblem/clusterHelpers.cc
	$(CC) -c $(FLAGS) ../clustering/clusteringProblem/clusterHelpers.cc -std=c++11

.PHONY:
	clean
	check

clean:
	rm -rf experiment experiment.o helper.o utils.o hashFunction.o item.o fileHandler.o lshEuclidean.o lshCosine.o cluster.o clusterAlgorithms.o clusterHelpers.o

check:
	valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all --vgdb-error=1 ./experiment -i dataSets/twitter_dataset_small_small.csv -c cluster.conf -o outputFile.txt -d euclidean -complete
