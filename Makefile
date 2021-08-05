all:
	g++ -pthread *.cpp -o exec

clean:
	rm *.o exec