all: simple_console

simple_console: simple_console.o
	g++ -o simple_console simple_console.o

simple_console.o:
	g++ -o simple_console.o -c simple_console.cpp

clean:
	rm -f simple_console
	rm -f *.o

