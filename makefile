CC = g++ -std=c++11

OBJS = CEnv.o main.o

LLFLAGS = -L/usr/local/lib
CCFLAGS = -I/usr/local/include -I/usr/local/include/eigen3 -I/usr/local/include/tf -I/usr/local/include/ta-lib
LDFLAGS = -lpthread -ltensorflow_cc -lta_lib

ALL:$(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) $(CCFLAGS) $(LLFLAGS) -o test

CEnv.o:CEnv.h CEnv.cpp
	$(CC) $(CCFLAGS) $(LLFLAGS) $(LDFLAGS) -c CEnv.cpp -o CEnv.o

main.o:test.cpp
	$(CC) -c test.cpp $(CCFLAGS) $(LLFLAGS) $(LDFLAGS) -o main.o


clean:
	rm -rf *.o test
