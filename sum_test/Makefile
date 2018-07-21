all : sum_test

sum_test : main.o sum.o
	g++ -o sum_test main.o sum.o

sum.o : sum.cpp sum.h
	g++ -o sum.o -c sum.cpp

main.o : main.cpp sum.h
	g++ -o main.o -c main.cpp

clean :
	rm -f *.o sum_test

