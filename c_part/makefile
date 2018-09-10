CC = g++ -std=c++11

OBJS = proto.o pool.o mdspi.o trader.o agent.o main.o

LLFLAGS = -L/usr/local/lib
CCFLAGS = -I/usr/local/include -I/usr/local/include/eigen3 -I/usr/local/include/tf -I/usr/local/include/ta-lib
LDFLAGS = -lpthread -lthostmduserapi -lthosttraderapi -lzmq -lprotobuf

ALL:$(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) $(CCFLAGS) $(LLFLAGS) -o CTP_Agent

main.o:main_ctp.cpp
	$(CC) -c main_ctp.cpp $(CCFLAGS) $(LLFLAGS) $(LDFLAGS) -o main.o

pool.o:CThreadPool.cpp CThreadPool.h
	$(CC) -c CThreadPool.cpp $(CCFLAGS) $(LLFLAGS) $(LDFLAGS) -o pool.o

trader.o:CTraderSpi.cpp CTraderSpi.h
	$(CC) -c CTraderSpi.cpp $(CCFLAGS) $(LLFLAGS) $(LDFLAGS) -o trader.o

mdspi.o:CMdSpi.cpp CMdSpi.h
	$(CC) -c CMdSpi.cpp $(CCFLAGS) $(LLFLAGS) $(LDFLAGS) -o mdspi.o

agent.o:CAgent_DQN.cpp CAgent_DQN.h
	$(CC) -c CAgent_DQN.cpp $(CCFLAGS) $(LLFLAGS) $(LDFLAGS) -o agent.o

proto.o:DQN_Agent.pb.cc DQN_Agent.pb.h
	$(CC) -c DQN_Agent.pb.cc $(CCFLAGS) $(LLFLAGS) $(LDFLAGS) -o proto.o


clean:
	rm -rf *.o test
