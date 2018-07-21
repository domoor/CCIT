all: simple_agent

simple_agent: simple_agent.o
	g++ -o simple_agent simple_agent.o

simple_agent.o:
	g++ -o simple_agent.o -c simple_agent.cpp

clean:
	rm -f simple_agent
	rm -f *.o

