# Petropoulakis Panagiotis
CC = g++
FLAGS = -g -Wall

exmperiment: experiment.o experimentHelpers.o utils.o hashFunction.o item.o fileHandler.o lshEuclidean.o lshCosine.o hypercubeEuclidean.o hypercubeCosine.o cluster.o initAlgorithms.o assignAlgorithms.o updateAlgorithms.o clusterHelpers.o
	$(CC) -o experiment $(FLAGS) experiment.o experimentHelpers.o utils.o hashFunction.o item.o fileHandler.o lshEuclidean.o lshCosine.o hypercubeEuclidean.o hypercubeCosine.o cluster.o initAlgorithms.o assignAlgorithms.o updateAlgorithms.o clusterHelpers.o -std=c++11

experiment.o: experiment.cc
	$(CC) -c  $(FLAGS) experiment.cc -std=c++11

experimentHelpers.o: helpers/experimentHelpers.cc
	$(CC) -c  $(FLAGS) helpers/experimentHelpers.cc -std=c++11

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

hypercubeEuclidean.o: ../clustering/neighborsProblem/model/hypercube/hypercubeEuclidean.cc
	$(CC) -c $(FLAGS) ../clustering/neighborsProblem/model/hypercube/hypercubeEuclidean.cc -std=c++11

hypercubeCosine.o: ../clustering/neighborsProblem/model/hypercube/hypercubeCosine.cc
	$(CC) -c $(FLAGS) ../clustering/neighborsProblem/model/hypercube/hypercubeCosine.cc -std=c++11

cluster.o: ../clustering/clusteringProblem/cluster.cc
	$(CC) -c $(FLAGS) ../clustering/clusteringProblem/cluster.cc -std=c++11

initAlgorithms.o: ../clustering/clusteringProblem/algorithms/initAlgorithms.cc
	$(CC) -c $(FLAGS) ../clustering/clusteringProblem/algorithms/initAlgorithms.cc -std=c++11

assignAlgorithms.o: ../clustering/clusteringProblem/algorithms/assignAlgorithms.cc
	$(CC) -c $(FLAGS) ../clustering/clusteringProblem/algorithms/assignAlgorithms.cc -std=c++11

updateAlgorithms.o: ../clustering/clusteringProblem/algorithms/updateAlgorithms.cc
	$(CC) -c $(FLAGS) ../clustering/clusteringProblem/algorithms/updateAlgorithms.cc -std=c++11

clusterHelpers.o: ../clustering/clusteringProblem/helpers/clusterHelpers.cc
	$(CC) -c $(FLAGS) ../clustering/clusteringProblem/helpers/clusterHelpers.cc -std=c++11

.PHONY:
	clean
	check
	run

clean:
	rm -rf experiment experiment.o experimentHelpers.o utils.o hashFunction.o item.o fileHandler.o lshEuclidean.o lshCosine.o hypercubeEuclidean.o hypercubeCosine.o cluster.o initAlgorithms.o assignAlgorithms.o updateAlgorithms.o clusterHelpers.o

check:
	valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all --vgdb-error=1 ./experiment -i dataSets/twitter_dataset_small.csv -c cluster.conf -o outputFile.txt -d euclidean -complete

run:
	./experiment -i dataSets/twitter_dataset_small.csv -c cluster.conf -o outputFile.txt -d euclidean
